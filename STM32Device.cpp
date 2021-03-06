/***********************************************************************************************************************
*                                                                                                                      *
* ANTIKERNEL v0.1                                                                                                      *
*                                                                                                                      *
* Copyright (c) 2012-2019 Andrew D. Zonenberg                                                                          *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Andrew D. Zonenberg
	@brief Implementation of STM32Device
 */

#include "jtaghal.h"
#include "STM32Device.h"
#include "STMicroDeviceID_enum.h"
#include "memory.h"

using namespace std;

STM32Device::STM32Device(
	unsigned int devid, unsigned int stepping,
	unsigned int idcode, JtagInterface* iface, size_t pos)
 : STMicroMicrocontroller(devid, stepping, idcode, iface, pos)
 , JtagDevice(idcode, iface, pos, 5)
 , m_deviceID(devid)
{
	if(pos == 0)
	{
		throw JtagExceptionWrapper(
			"STM32Device boundary scan TAP must not be the first device in the scan chain. Where's the ARM DAP?",
			"");
	}

	//Look up RAM size (TODO can we get this from descriptors somehow? common within a family?)
	switch(m_deviceID)
	{
		case STM32F103:
			m_ramKB				= 96;
			m_flashSfrBase		= 0x40022000;
			m_uniqueIDBase		= 0x1ffff7e8;
			m_flashSizeBase		= 0x1ffff7e0;
			break;

		case STM32F411E:
			m_ramKB 			= 128;
			m_flashSfrBase		= 0x40023C00;
			m_uniqueIDBase		= 0x1fff7a10;
			m_flashSizeBase		= 0x1fff7a20;
			break;

		case STM32F777:
			m_ramKB				= 512;
			m_flashSfrBase		= 0x40023C00;
			m_uniqueIDBase		= 0x1ff0F420;
			m_flashSizeBase		= 0x1ff0F440;
			break;

		default:
			m_ramKB = 0;
	}

	//TODO: How portable are these addresses?
	m_flashMemoryBase	= 0x08000000;
	m_sramMemoryBase	= 0x20000000;

	m_locksProbed = false;

}

void STM32Device::PostInitProbes(bool quiet)
{
	//Get a pointer to our ARM DAP. This should always be one scan chain position before us.
	m_dap = dynamic_cast<ARMDebugPort*>(m_iface->GetDevice(m_pos-1));
	if(m_dap == NULL)
	{
		throw JtagExceptionWrapper(
			"STM32Device expects an ARM DAP one chain position prior",
			"");
	}

	//Leave a lot of stuff blank to avoid probing and tripping alarms
	if(quiet)
	{
		//unknown protection level
		m_protectionLevel = 3;
		m_locksProbed = true;
		m_flashKB = 0;
		m_waferX = 0;
		m_waferY = 0;
		m_waferNum = 0;
		strncpy(m_waferLot, "???????", sizeof(m_waferLot));
		for(int i=0; i<12; i++)
			m_serialRaw[i] = 0;
		return;
	}

	//Check read lock status
	STM32Device::ProbeLocksNondestructive();

	//Look up size of flash memory.
	//If locked, don't try reading as this will trigger security lockdown
	if(IsDeviceReadLocked().GetValue())
		LogVerbose("STM32Device: Cannot determine flash size because read protection is set\n");
	else
	{
		try
		{
			m_flashKB = m_dap->ReadMemory(m_flashSizeBase) >> 16;	//F_ID, flash size in kbytes
		}
		catch(const JtagException& e)
		{
			//If we fail, set flash size to zero so we don't try doing anything with it
			m_flashKB = 0;
			LogWarning("STM32Device: Unable to read flash memory size even though read protection doesn't seem to be set\n");
		}
	}

	//Extract serial number fields
	if(IsDeviceReadLocked().GetValue())
	{
		LogVerbose("STM32Device: Cannot determine serial number because read protection is set\n");
	}
	else
	{
		try
		{
			uint32_t serial[3];
			serial[0] = m_dap->ReadMemory(m_uniqueIDBase);
			serial[1] = m_dap->ReadMemory(m_uniqueIDBase+4);
			serial[2] = m_dap->ReadMemory(m_uniqueIDBase+8);
			m_waferX = serial[0] >> 16;
			m_waferY = serial[0] & 0xffff;
			m_waferNum = serial[1] & 0xff;
			m_waferLot[0] = (serial[1] >> 24) & 0xff;
			m_waferLot[1] = (serial[1] >> 16) & 0xff;
			m_waferLot[2] = (serial[1] >> 8) & 0xff;
			m_waferLot[3] = (serial[2] >> 24) & 0xff;
			m_waferLot[4] = (serial[2] >> 16) & 0xff;
			m_waferLot[5] = (serial[2] >> 8) & 0xff;
			m_waferLot[6] = (serial[2] >> 0) & 0xff;
			m_waferLot[7] = 0;
			m_serialRaw[0] = m_waferX >> 8;
			m_serialRaw[1] = m_waferX & 0xff;
			m_serialRaw[2] = m_waferY >> 8;
			m_serialRaw[3] = m_waferY & 0xff;
			m_serialRaw[4] = m_waferNum;
			for(int i=0; i<7; i++)
				m_serialRaw[5+i] = m_waferLot[i];
		}
		catch(const JtagException& e)
		{
			//If we can't read the serial number, that probably means the chip is locked.
			//Write zeroes rather than leaving it uninitialized.
			m_waferX = 0;
			m_waferY = 0;
			m_waferNum = 0;
			strncpy(m_waferLot, "???????", sizeof(m_waferLot));
			for(int i=0; i<12; i++)
				m_serialRaw[i] = 0;

			//Display a warning if the chip is NOT locked,  but we couldn't read the serial number anyway
			LogWarning("STM32Device: Unable to read serial number even though read protection doesn't seem to be set\n");
		}
	}
}

/**
	@brief Destructor
 */
STM32Device::~STM32Device()
{
}

JtagDevice* STM32Device::CreateDevice(
	unsigned int devid, unsigned int stepping, unsigned int idcode, JtagInterface* iface, size_t pos)
{
	//TODO: Sanity checks
	return new STM32Device(devid, stepping, idcode, iface, pos);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Device property queries

bool STM32Device::ReadingSerialRequiresReset()
{
	return false;
}

int STM32Device::GetSerialNumberLength()
{
	return 12;
}

int STM32Device::GetSerialNumberLengthBits()
{
	return 96;
}

void STM32Device::GetSerialNumber(unsigned char* data)
{
	for(int i=0; i<12; i++)
		data[i] = m_serialRaw[i];
}

string STM32Device::GetPrettyPrintedSerialNumber()
{
	char tmp[256];
	snprintf(tmp, sizeof(tmp),
		"Die (%d, %d), wafer %d, lot %s",
		m_waferX, m_waferY,
		m_waferNum,
		m_waferLot);

	return string(tmp);
}

string STM32Device::GetDescription()
{
	string name = "(unknown STM32";
	switch(m_devicetype)
	{
		case STM32F103:
			name = "STM32F103";
			break;

		case STM32F411E:
			name = "STM32F411E";
			break;

		case STM32F777:
			name = "STM32F777";
			break;
	}

	char srev[256];
	snprintf(srev, sizeof(srev), "ST %s (%u KB SRAM, %u KB flash, stepping %u)",
		name.c_str(),
		m_ramKB,
		m_flashKB,
		m_stepping);

	return string(srev);
}

bool STM32Device::IsProgrammed()
{
	//If we're read locked, we're obviously programmed in some way
	ProbeLocksNondestructive();
	if(m_protectionLevel != 0)
		return true;

	//If the first word of the interrupt vector table is blank, the device is not programmed
	//because no code can execute.
	//This is a lot faster than a full chip-wide blank check.
	return (m_dap->ReadMemory(m_flashMemoryBase) != 0xffffffff);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lock bit access

//TODO: cache this somewhere
ARMv7MProcessor* STM32Device::GetCPU()
{
	for(size_t i=0; i<m_dap->GetNumTargets(); i++)
	{
		auto t = dynamic_cast<ARMv7MProcessor*>(m_dap->GetTarget(i));
		if(t)
			return t;
	}
	return NULL;
}

void STM32Device::PrintLockProbeDetails()
{
	auto cpu = GetCPU();

	const char* table[]=
	{
		"completely unlocked",
		"boundary scan enabled, memory access and debug disabled",
		"JTAG completely disabled",
		"quiet probe, lock status not checked"
	};
	LogNotice("STM32 read protection level appears to be: %d (%s)\nDetails:\n", m_protectionLevel, table[m_protectionLevel]);

	{
		LogIndenter li;

		if(!cpu)
			return;

		try
		{
			cpu->ReadCPURegister(ARMv7MProcessor::R0);
			LogNotice("CPU registers:  unlocked\n");
			cpu->PrintRegisters();
		}
		catch(const JtagException& e)
		{
			LogNotice("CPU registers:  locked\n");
		}

		try
		{
			cpu->ReadMemory(m_flashMemoryBase);
			LogNotice("Flash:          unlocked, dumping first few bytes\n");
			LogIndenter li;
			for(int i=0; i<4; i++)
			{
				uint32_t addr = m_flashMemoryBase + 16*i;
				LogVerbose("0x%08x: %08x %08x %08x %08x\n",
					addr,
					cpu->ReadMemory(addr),
					cpu->ReadMemory(addr + 0x4),
					cpu->ReadMemory(addr + 0x8),
					cpu->ReadMemory(addr + 0xc)
					);
			}

		}
		catch(const JtagException& e)
		{
			LogNotice("Flash:          locked\n");
		}

		try
		{
			cpu->ReadMemory(m_sramMemoryBase);
			LogNotice("SRAM:           unlocked, dumping first few bytes\n");
			LogIndenter li;
			for(int i=0; i<4; i++)
			{
				uint32_t addr = m_sramMemoryBase + 16*i;
				LogVerbose("0x%08x: %08x %08x %08x %08x\n",
					addr,
					cpu->ReadMemory(addr),
					cpu->ReadMemory(addr + 0x4),
					cpu->ReadMemory(addr + 0x8),
					cpu->ReadMemory(addr + 0xc)
					);
			}

		}
		catch(const JtagException& e)
		{
			LogNotice("SRAM:           locked\n");
		}
	}
}

void STM32Device::ProbeLocksNondestructive()
{
	//Don't poke more than once
	if(m_locksProbed)
		return;

	LogTrace("Running non-destructive lock status tests\n");
	LogIndenter li;

	try
	{
		uint32_t optcr = m_dap->ReadMemory(m_flashSfrBase + FLASH_OPTCR);
		uint32_t rdp = (optcr >> 8) & 0xff;

		LogTrace("OPTCR = %08x\n", optcr);
		LogTrace("OPTCR.RDP = 0x%02x\n", rdp);

		//If OPTCR is all 1s we probably just bulk-erased everything.
		//Treat this as unlocked.
		if(optcr == 0xffffffff)
			m_protectionLevel = 0;

		else
		{
			//TODO: query write protection

			//Not locked?
			if(rdp == 0xaa)
				m_protectionLevel = 0;

			//Full locked? we should never see this because it disables JTAG
			else if(rdp == 0xcc)
				m_protectionLevel = 2;

			//Level 1 lock
			//Unlikely to see this except right after programming the lock bit, since RDP disables access to OPTCR
			else
				m_protectionLevel = 1;
		}
	}
	catch(const JtagException& e)
	{
		//If wqe get here, reading one or more of the SFRs failed.
		//This is a probable indicator of level 1 read protection since we have limited JTAG access
		//but can still get ID codes etc (which rules out level 2).
		m_protectionLevel = 1;
	}

	m_locksProbed = true;
}

void STM32Device::ProbeLocksDestructive()
{
	//no destructive tests implemented yet
	return ProbeLocksNondestructive();
}

UncertainBoolean STM32Device::CheckMemoryAccess(uint32_t ptr, unsigned int access)
{
	if(access != ACCESS_READ)
	{
		throw JtagExceptionWrapper(
			"STM32Device write/execute testing not supported",
			"");
	}

	try
	{
		//If we get a value other than 0 or FF, it's definitely good
		uint32_t retval = m_dap->ReadMemory(ptr);
		if( (retval != 0x00000000) && (retval != 0xffffffff) )
			return UncertainBoolean(true, UncertainBoolean::CERTAIN);

		//These values might be returned by some protection schemes if the chip is locked
		//so give a slightly lower confidence
		else
			return UncertainBoolean(true, UncertainBoolean::VERY_LIKELY);
	}
	catch(const JtagException& e)
	{
		return UncertainBoolean(false, UncertainBoolean::VERY_LIKELY);
	}
}

UncertainBoolean STM32Device::IsDeviceReadLocked()
{
	ProbeLocksNondestructive();

	switch(m_protectionLevel)
	{
		case 2:
			return UncertainBoolean( true, UncertainBoolean::CERTAIN );

		case 0:
			return UncertainBoolean( false, UncertainBoolean::CERTAIN );

		case 3:
			return UncertainBoolean( true, UncertainBoolean::USELESS );

		case 1:
		default:
			return UncertainBoolean( true, UncertainBoolean::VERY_LIKELY );
	}
}

void STM32Device::SetReadLock()
{
	UnlockFlashOptions();

	LogTrace("Setting read lock...\n");
	LogIndenter li;

	//Read OPTCR
	uint32_t cr = m_dap->ReadMemory(m_flashSfrBase + FLASH_OPTCR);
	LogTrace("Old OPTCR = %08x\n", cr);
	cr &= 0xffff00ff;
	cr |= 0x5500;		//CC = level 2 lock
						//AA = no lock
						//anything else = level 1 lock

	//Actually commit the write to the option register
	//If we don't do this, the SRAM register is changed but it won't persist across reboots. Pretty useless!
	cr |= 0x2;

	LogTrace("Setting OPTCR = %08x\n", cr);

	//Write it back
	m_dap->WriteMemory(m_flashSfrBase + FLASH_OPTCR, cr);
}

void STM32Device::ClearReadLock()
{
	LogTrace("Clearing read lock...\n");
	LogIndenter li;

	UnlockFlash();
	UnlockFlashOptions();

	//Read OPTCR
	uint32_t cr = m_dap->ReadMemory(m_flashSfrBase + FLASH_OPTCR);
	LogTrace("Old OPTCR = %08x\n", cr);

	//Patch the read lock value to "not locked"
	cr &= 0xffff00ff;
	cr |= 0x0000aa00;

	//Actually commit the write to the option register
	//If we don't do this, the SRAM register is changed but it won't persist across reboots. Pretty useless!
	cr |= 0x2;

	LogTrace("Setting OPTCR = %08x\n", cr);

	//Write it back
	m_dap->WriteMemory(m_flashSfrBase + FLASH_OPTCR, cr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Programming

void STM32Device::UnlockFlashOptions()
{
	LogTrace("Unlocking Flash option register...\n");
	LogIndenter li;

	//Check CR
	uint32_t cr = 0x00000001;
	try
	{
		cr = m_dap->ReadMemory(m_flashSfrBase + FLASH_OPTCR);
	}
	catch(JtagException& e)
	{
		LogWarning("Couldn't read intial OPTCR, guessing value of %08x\n", cr);
	}
	LogTrace("Initial OPTCR = %08x\n", cr);
	if(cr & 0x00000001)
	{
		LogTrace("Option register is curently locked, unlocking...\n");

		//Unlock flash
		m_dap->WriteMemory(m_flashSfrBase + FLASH_OPTKEYR, 0x08192A3B);
		m_dap->WriteMemory(m_flashSfrBase + FLASH_OPTKEYR, 0x4C5D6E7F);

		//Check CR
		try
		{
			cr = m_dap->ReadMemory(m_flashSfrBase + FLASH_OPTCR);
		}
		catch(JtagException& e)
		{
			cr = 0x00000000;
			LogWarning("Couldn't read unlocked OPTCR, guessing value of %08x\n", cr);
		}
		LogTrace("Unlocked OPTCR = %08x\n", cr);
		if(cr & 0x00000001)
		{
			throw JtagExceptionWrapper(
				"STM32Device::UnlockFlashOptions() got OPTCR still locked after unlock sequence!!!",
				"");
		}
	}
	else
		LogTrace("Option register is already unlocked, no action required\n");
}

void STM32Device::UnlockFlash()
{
	LogTrace("Unlocking Flash memory...\n");
	LogIndenter li;

	//Check CR
	uint32_t cr = m_dap->ReadMemory(m_flashSfrBase + FLASH_CR);
	LogTrace("Initial CR = %08x\n", cr);
	if(cr & 0x80000000)
	{
		LogTrace("Flash is curently locked\n");

		//Unlock flash
		m_dap->WriteMemory(m_flashSfrBase + FLASH_KEYR, 0x45670123);
		m_dap->WriteMemory(m_flashSfrBase + FLASH_KEYR, 0xCDEF89AB);

		//Check CR
		cr = m_dap->ReadMemory(m_flashSfrBase + FLASH_CR);
		LogTrace("Unlocked CR = %08x\n", cr);
		if(cr & 0x80000000)
		{
			throw JtagExceptionWrapper(
				"STM32Device::UnlockFlash() got CR still locked after unlock sequence!!!",
				"");
		}
	}
	else
		LogTrace("Flash is already unlocked, no action required\n");
}

void STM32Device::PollUntilFlashNotBusy()
{
	//LogTrace("Waiting for Flash to be ready...\n");
	//LogIndenter li;

	//Poll FLASH_SR.BSY until it's clear
	uint32_t sr = m_dap->ReadMemory(m_flashSfrBase + FLASH_SR);
	//LogTrace("SR = %08x\n", sr);
	int interval = 1;
	while(sr & 0x00010000)
	{
		//LogTrace("SR = %08x\n", sr);
		usleep(100 * interval);
		sr = m_dap->ReadMemory(m_flashSfrBase + FLASH_SR);

		//Exponential back-off on polling interval
		interval *= 10;
	}
}

void STM32Device::Erase()
{
	LogTrace("Erasing...\n");

	//Look up FLASH_OPTCR
	uint32_t optcr = m_dap->ReadMemory(m_flashSfrBase + FLASH_OPTCR);
	LogTrace("FLASH_OPTCR = %08x\n", optcr);

	//Unlock flash and make sure it's ready
	UnlockFlash();
	PollUntilFlashNotBusy();

	//Do a full-chip erase with x32 parallelism
	//bit9:8 = 10 = x64
	//bit2 = mass erase
	//bit16 = go do it
	m_dap->WriteMemory(m_flashSfrBase + FLASH_CR, 0x10204);

	//Wait for it to finish the erase operation
	PollUntilFlashNotBusy();

	//Don't blank check by default
}

bool STM32Device::BlankCheck()
{
	LogDebug("Blank checking...\n");
	LogIndenter li;

	bool quitImmediately = true;

	uint32_t flashBytes = m_flashKB * 1024;
	uint32_t addrMax = m_flashMemoryBase + flashBytes;
	uint32_t addr = m_flashMemoryBase;
	LogTrace("Checking address range from 0x%08x to 0x%08x...\n", addr, addrMax);
	bool blank = true;
	for(; addr<addrMax; addr += 4)
	{
		if( (addr & 0x3fff) == 0)
		{
			float fracDone = (addr - m_flashMemoryBase) * 1.0f / flashBytes;
			LogDebug("%08x (%.1f %%)\n", addr, fracDone * 100.0f);
		}

		uint32_t rdata = m_dap->ReadMemory(addr);
		if(rdata != 0xffffffff)
		{
			LogNotice("Device is NOT blank. Found data 0x%08x at flash address 0x%08x\n",
				rdata, addr);
			if(quitImmediately)
				return false;
			blank = false;
		}
	}

	return blank;
}

/**
	@brief Performs a soft reset of the CPU
 */
void STM32Device::Reset()
{
	GetCPU()->Reset();
}

/**
	@brief Loads the firmware

	For now assume it's a raw ROM image, no ELF etc supported
 */
FirmwareImage* STM32Device::LoadFirmwareImage(const unsigned char* data, size_t len)
{
	ByteArrayFirmwareImage* img = new ByteArrayFirmwareImage;
	img->raw_bitstream_len = len;

	//Round length up to nearest full word
	size_t roundlen = len;
	if(len & 0x3)
		roundlen = len - (len & 3) + 4;
	img->raw_bitstream = new uint8_t[roundlen];
	memset(img->raw_bitstream, 0, roundlen);

	memcpy(img->raw_bitstream, data, len);
	return img;
}

/**
	@brief Programs a firmware image to the device

	For now. assume the image is a flat binary to be burned to flash.
 */
void STM32Device::Program(FirmwareImage* image)
{
	//TODO: take ELF images directly?
	auto bimage = dynamic_cast<ByteArrayFirmwareImage*>(image);
	if(!bimage)
	{
		throw JtagExceptionWrapper(
			"STM32Device::Program() needs a byte array firmware image",
			"");
	}

	//Halt the CPU
	auto cpu = GetCPU();
	cpu->DebugHalt();

	//Check if the beginning of the vector table is blank.
	//If not blank, trigger a bulk erase
	//This is a quick check for normally-programmed chips.
	//If the vector table is blank but there's data later in memory, a user-initiated bulk erase is required.
	if(m_dap->ReadMemory(m_flashMemoryBase) != 0xffffffff)
	{
		LogDebug("Flash is not blank, erasing...\n");
		Erase();
	}

	//Unlock flash and make sure it's ready
	UnlockFlash();
	PollUntilFlashNotBusy();

	LogDebug("Programming address range from 0x%08x to 0x%08x...\n",
		m_flashMemoryBase, m_flashMemoryBase + (uint32_t)bimage->raw_bitstream_len);

	uint32_t oldcr = m_dap->ReadMemory(m_flashSfrBase + FLASH_CR) & 0xfffffcfe;	//mask off PG bit and op size
	oldcr |= 0x200;		//set op size to x32
	for(size_t offset=0; offset < bimage->raw_bitstream_len; offset=offset+4)
	{
		//Look up the data word
		uint32_t addr = m_flashMemoryBase + offset;
		uint32_t* ptr = reinterpret_cast<uint32_t*>(bimage->raw_bitstream + offset);

		//Status print
		if( (offset & 0x3fff) == 0)
		{
			float fracDone = offset * 1.0f / bimage->raw_bitstream_len;
			LogDebug("%08x (%.1f %%)\n", addr, fracDone * 100.0f);
		}

		//OPTIMIZATION: if the data word is 0xffffffff, then no need to program it since the flash is already blank
		if(*ptr == 0xffffffff)
			continue;

		//Set PG bit in CR to configure flash for programming
		m_dap->WriteMemory(m_flashSfrBase + FLASH_CR, oldcr | 0x1);

		//Write the actual data word and wait until it finishes
		m_dap->WriteMemory(addr, *ptr);
		PollUntilFlashNotBusy();

		//Clear PG bit to exit programming mode
		m_dap->WriteMemory(m_flashSfrBase + FLASH_CR, oldcr);
	}

	//Automatically reset/resume at the end
	cpu->Reset();
	cpu->DebugResume();
}
