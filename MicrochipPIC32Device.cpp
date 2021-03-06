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
	@brief Implementation of MicrochipPIC32Device
 */

#include "jtaghal.h"
#include "MicrochipPIC32Device.h"
#include "memory.h"

using namespace std;

//IDCODE, name, family, cpu, sram, flash, bootflash
static const MicrochipPIC32DeviceInfo g_devinfo[] =
{
	//MX1xx series
	{ MicrochipPIC32Device::PIC32MX110F016B, "PIC32MX110F016B",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   4,  16,  3 },
	{ MicrochipPIC32Device::PIC32MX110F016C, "PIC32MX110F016C",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   4,  16,  3 },
	{ MicrochipPIC32Device::PIC32MX110F016D, "PIC32MX110F016D",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   4,  16,  3 },

	{ MicrochipPIC32Device::PIC32MX120F032B, "PIC32MX120F032B",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   8,  32,  3 },
	{ MicrochipPIC32Device::PIC32MX120F032C, "PIC32MX120F032C",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   8,  32,  3 },
	{ MicrochipPIC32Device::PIC32MX120F032D, "PIC32MX120F032D",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   8,  32,  3 },

	{ MicrochipPIC32Device::PIC32MX130F064B, "PIC32MX130F064B",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  16,  64,  3 },
	{ MicrochipPIC32Device::PIC32MX130F064C, "PIC32MX130F064C",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  16,  64,  3 },
	{ MicrochipPIC32Device::PIC32MX130F064D, "PIC32MX130F064D",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  16,  64,  3 },

	{ MicrochipPIC32Device::PIC32MX150F128B, "PIC32MX150F128B",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  32, 128,  3 },
	{ MicrochipPIC32Device::PIC32MX150F128C, "PIC32MX150F128C",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  32, 128,  3 },
	{ MicrochipPIC32Device::PIC32MX150F128D, "PIC32MX150F128D",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  32, 128,  3 },

	//MX2xx series
	{ MicrochipPIC32Device::PIC32MX210F016B, "PIC32MX210F016B",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   4,  16,  3 },
	{ MicrochipPIC32Device::PIC32MX210F016C, "PIC32MX210F016C",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   4,  16,  3 },
	{ MicrochipPIC32Device::PIC32MX210F016D, "PIC32MX210F016D",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   4,  16,  3 },

	{ MicrochipPIC32Device::PIC32MX220F032B, "PIC32MX220F032B",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   8,  32,  3 },
	{ MicrochipPIC32Device::PIC32MX220F032C, "PIC32MX220F032C",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   8,  32,  3 },
	{ MicrochipPIC32Device::PIC32MX220F032D, "PIC32MX220F032D",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,   8,  32,  3 },

	{ MicrochipPIC32Device::PIC32MX230F064B, "PIC32MX230F064B",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  16,  64,  3 },
	{ MicrochipPIC32Device::PIC32MX230F064C, "PIC32MX230F064C",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  16,  64,  3 },
	{ MicrochipPIC32Device::PIC32MX230F064D, "PIC32MX230F064D",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  16,  64,  3 },

	{ MicrochipPIC32Device::PIC32MX250F128B, "PIC32MX250F128B",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  32, 128,  3 },
	{ MicrochipPIC32Device::PIC32MX250F128C, "PIC32MX250F128C",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  32, 128,  3 },
	{ MicrochipPIC32Device::PIC32MX250F128D, "PIC32MX250F128D",
		MicrochipPIC32Device::FAMILY_MX12,  MicrochipPIC32Device::CPU_M4K,  32, 128,  3 },

	//MX3xx series
	{ MicrochipPIC32Device::PIC32MX330F064H, "PIC32MX330F064H",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  16,  64, 12 },
	{ MicrochipPIC32Device::PIC32MX330F064L, "PIC32MX330F064L",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  16,  64, 12 },

	{ MicrochipPIC32Device::PIC32MX340F512H, "PIC32MX340F512H",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  32, 512, 12 },

	{ MicrochipPIC32Device::PIC32MX350F128H, "PIC32MX350F128H",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },
	//{ MicrochipPIC32Device::PIC32MX350F128L, "PIC32MX350F128L",
	//	MicrochipPIC32Device::FAMILY_MX34, MicrochipPIC32Device::CPU_M4K, 32, 128, 12 },

	{ MicrochipPIC32Device::PIC32MX350F256H, "PIC32MX350F256H",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  64, 256, 12 },
	{ MicrochipPIC32Device::PIC32MX350F256L, "PIC32MX350F256L",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  64, 256, 12 },

	//MX4xx series
	{ MicrochipPIC32Device::PIC32MX430F064H, "PIC32MX430F064H",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  16,  64, 12 },
	{ MicrochipPIC32Device::PIC32MX430F064L, "PIC32MX430F064L",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  16,  64, 12 },

	{ MicrochipPIC32Device::PIC32MX450F128H, "PIC32MX450F128H",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },
	{ MicrochipPIC32Device::PIC32MX450F128L, "PIC32MX450F128L",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },

	{ MicrochipPIC32Device::PIC32MX450F256H, "PIC32MX450F256H",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  64, 256, 12 },
	{ MicrochipPIC32Device::PIC32MX450F256L, "PIC32MX450F256L",
		MicrochipPIC32Device::FAMILY_MX34,  MicrochipPIC32Device::CPU_M4K,  64, 256, 12 },

	//MX5xx series
	{ MicrochipPIC32Device::PIC32MX534F064H, "PIC32MX534F064H",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  16,  64, 12 },
	//{ MicrochipPIC32Device::PIC32MX534F064L, "PIC32MX534F064L",
	//	MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K, 16,  64, 12 },

	{ MicrochipPIC32Device::PIC32MX564F064H, "PIC32MX564F064H",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  16,  64, 12 },
	{ MicrochipPIC32Device::PIC32MX564F064L, "PIC32MX564F064L",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  16,  64, 12 },

	{ MicrochipPIC32Device::PIC32MX564F128H, "PIC32MX564F128H",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },
	{ MicrochipPIC32Device::PIC32MX564F128L, "PIC32MX564F128L",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },

	//MX6xx series
	{ MicrochipPIC32Device::PIC32MX664F064H, "PIC32MX664F064H",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  32,  64, 12 },
	{ MicrochipPIC32Device::PIC32MX664F064L, "PIC32MX664F064L",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  32,  64, 12 },

	{ MicrochipPIC32Device::PIC32MX664F128H, "PIC32MX664F128H",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },
	{ MicrochipPIC32Device::PIC32MX664F128L, "PIC32MX664F128L",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },

	{ MicrochipPIC32Device::PIC32MX695F512L, "PIC32MX695F512L",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K, 128, 512, 12 },

	//MX7xx series
	{ MicrochipPIC32Device::PIC32MX764F128H, "PIC32MX764F128H",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },
	{ MicrochipPIC32Device::PIC32MX764F128L, "PIC32MX764F128L",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K,  32, 128, 12 },

	{ MicrochipPIC32Device::PIC32MX795F512L, "PIC32MX795F512L",
		MicrochipPIC32Device::FAMILY_MX567, MicrochipPIC32Device::CPU_M4K, 128, 512, 12 },

	//MM series
	{ MicrochipPIC32Device::PIC32MM0016GPL020, "PIC32MM0016GPL020",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 4, 16, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0032GPL020, "PIC32MM0032GPL020",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 8, 32, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0064GPL020, "PIC32MM0064GPL020",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 8, 64, 5.75 },

	{ MicrochipPIC32Device::PIC32MM0016GPL028, "PIC32MM0016GPL028",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 4, 16, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0032GPL028, "PIC32MM0032GPL028",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 8, 32, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0064GPL028, "PIC32MM0064GPL028",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 8, 64, 5.75 },

	{ MicrochipPIC32Device::PIC32MM0016GPL036, "PIC32MM0016GPL036",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 4, 16, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0032GPL036, "PIC32MM0032GPL036",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 8, 32, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0064GPL036, "PIC32MM0064GPL036",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 8, 64, 5.75 },

	{ MicrochipPIC32Device::PIC32MM0064GPM028, "PIC32MM0064GPM028",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 16, 64, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0128GPM028, "PIC32MM0128GPM028",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 16, 128, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0256GPM028, "PIC32MM0256GPM028",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 32, 256, 5.75 },

	{ MicrochipPIC32Device::PIC32MM0064GPM036, "PIC32MM0064GPM036",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 16, 64, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0128GPM036, "PIC32MM0128GPM036",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 16, 128, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0256GPM036, "PIC32MM0256GPM036",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 32, 256, 5.75 },

	{ MicrochipPIC32Device::PIC32MM0064GPM048, "PIC32MM0064GPM048",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 16, 64, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0128GPM048, "PIC32MM0128GPM048",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 16, 128, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0256GPM048, "PIC32MM0256GPM048",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 32, 256, 5.75 },

	{ MicrochipPIC32Device::PIC32MM0064GPM064, "PIC32MM0064GPM064",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 16, 64, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0128GPM064, "PIC32MM0128GPM064",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 16, 128, 5.75 },
	{ MicrochipPIC32Device::PIC32MM0256GPM064, "PIC32MM0256GPM064",
		MicrochipPIC32Device::FAMILY_MM, MicrochipPIC32Device::CPU_MAPTIV, 32, 256, 5.75 }
};

MicrochipPIC32Device::MicrochipPIC32Device(
	unsigned int devid, unsigned int stepping,
	unsigned int idcode, JtagInterface* iface, size_t pos)
 : MicrochipMicrocontroller(idcode, iface, pos, 5)
{
	m_devid = devid;
	m_stepping = stepping;

	//Look up device info in the table and make sure it exists
	m_devinfo = NULL;
	for(auto& x : g_devinfo)
	{
		if(x.devid == devid)
			m_devinfo = &x;
	}

	if(!m_devinfo)
	{
		throw JtagExceptionWrapper(
			"Invalid PIC32 JTAG IDCODE",
			"");
	}
}

void MicrochipPIC32Device::PostInitProbes(bool /*quiet*/)
{
	//Reset both TAPS
	EnterMtapMode();
	ResetToIdle();
	EnterEjtagMode();
	ResetToIdle();

	//Get our implementation code
	GetImpCode();
}

/**
	@brief Destructor
 */
MicrochipPIC32Device::~MicrochipPIC32Device()
{
}

JtagDevice* MicrochipPIC32Device::CreateDevice(
	unsigned int devid, unsigned int stepping, unsigned int idcode, JtagInterface* iface, size_t pos)
{
	//TODO: Sanity checks
	return new MicrochipPIC32Device(devid, stepping, idcode, iface, pos);
}

std::string MicrochipPIC32Device::GetDescription()
{
	char srev[256];
	snprintf(srev, sizeof(srev), "Microchip %s (%u KB SRAM, %u KB code flash, %.2f KB boot flash, stepping %u)",
		m_devinfo->name,
		m_devinfo->sram_size,
		m_devinfo->program_flash_size,
		m_devinfo->boot_flash_size,
		m_stepping);

	return string(srev);
}

EjtagImplementationCodeRegister MicrochipPIC32Device::GetImpCode()
{
	//TODO: dont do this unless in MTAP mode!
	EnterEjtagMode();
	SetIR(INST_IMPCODE);

	//Get the data
	EjtagImplementationCodeRegister impcode;
	uint32_t dummy = 0;
	ScanDR((uint8_t*)&dummy, (uint8_t*)&impcode.word, 32);

	//DEBUG: print results
	/*
	LogDebug("Implementation code = %08x\n", impcode.word);
	LogDebug("    64-bit CPU:          %d\n", impcode.bits.processor_is_64);
	LogDebug("    EJTAG DMA supported: %d\n", !impcode.bits.no_ejtag_dma);
	LogDebug("    MIPS16 supported:    %d\n", impcode.bits.mips16_supported);
	switch(impcode.bits.asid_size)
	{
		case 0:
			LogDebug("    ASID size:           None\n");
			break;
		case 1:
			LogDebug("    ASID size:           6 bit\n");
			break;
		case 2:
			LogDebug("    ASID size:           8 bit\n");
			break;
		case 3:
			LogDebug("    ASID size:           Reserved\n");
			break;
	}
	LogDebug("    DINT supported:      %d\n", impcode.bits.dint_supported);
	LogDebug("    R3K privilege mode:  %d\n", impcode.bits.r3k_priv);
	switch(impcode.bits.ejtag_version)
	{
		case 0:
			LogDebug("    EJTAG version:       1.0 / 2.0\n");
			break;

		case 1:
			LogDebug("    EJTAG version:       2.5\n");
			break;

		case 2:
			LogDebug("    EJTAG version:       2.6\n");
			break;

		case 3:
			LogDebug("    EJTAG version:       3.1\n");
			break;

		default:
			LogDebug("    EJTAG version:       Reserved\n");
			break;
	}
	*/
	return impcode;
}

MicrochipPIC32DeviceStatusRegister MicrochipPIC32Device::GetStatus()
{
	//TODO: dont do this unless in EJTAG mode
	EnterMtapMode();

	//MCHP_STATUS is a nop that just returns status in the capture value
	MicrochipPIC32DeviceStatusRegister status;
	status.word = SendMchpCommand(MCHP_STATUS);

	//DEBUG: print results
	/*
	LogDebug("Status = %02x\n", status.word);
	LogDebug("    Unprotected:  %d\n", status.bits.code_protect_off);
	LogDebug("    NVM err:      %d\n", status.bits.nvm_error);
	LogDebug("    Config ready: %d\n", status.bits.cfg_rdy);
	LogDebug("    Flash busy:   %d\n", status.bits.flash_busy);
	LogDebug("    Flash enable: %d\n", status.bits.flash_en);
	LogDebug("    Reset:        %d\n", status.bits.reset_active);
	*/

	return status;
}

void MicrochipPIC32Device::EnterSerialExecMode()
{
	//Reset the device and verify the reset took effect
	EnterMtapMode();
	SendMchpCommand(MCHP_ASSERT_RST);
	auto stat = GetStatus();
	if(!stat.bits.reset_active)
	{
		throw JtagExceptionWrapper(
			"Device should be in reset, but isn't!",
			"");
	}

	//Select the EJTAG TAP, then select JTAG boot mode
	EnterEjtagMode();
	SetIR(INST_DEBUGBOOT);

	//De-assert reset and verify that happened
	EnterMtapMode();
	SendMchpCommand(MCHP_DE_ASSERT_RST);
	stat = GetStatus();
	if(stat.bits.reset_active)
	{
		throw JtagExceptionWrapper(
			"Device should not be in reset, but is!",
			"");
	}

	//Enable flash access
	SendMchpCommand(MCHP_FLASH_ENABLE);

	//Force a debug exception and execute a NOP
	SerialExecuteInstruction(0x0, true);
}

EjtagControlRegister MicrochipPIC32Device::WaitForEjtagMemoryOperation(bool first)
{
	//Wait for CPU to request a memory transaction
	EjtagControlRegister write_reg;
	EjtagControlRegister read_reg;
	while(true)
	{
		write_reg.word = 0;						//default to zero
		write_reg.bits.proc_access	= 1;		//don't clear processor-access bit
		write_reg.bits.probe_enable	= 1;		//enable debug probe
		write_reg.bits.debug_vector_pos	= 1;	//debug exception traps to DMSEG (emulated memory)

		//bit 12 = request debug interrupt exception
		//This seems critical when entering debug mode and isn't documented.
		//Possible that MCHP_ASSERT_RST doesn't have quite the same effect as a MCLR reset,
		//and thus we need to do this to force a debug exception?
		if(first)
		{
			write_reg.bits.debug_irq = 1;
			first = false;
		}

		//Poll control register and wait for PrAcc to be set
		SetIR(INST_CONTROL);
		ScanDR((uint8_t*)&write_reg.word, (uint8_t*)&read_reg.word, 32);
		if(read_reg.bits.proc_access)
			break;
	}

	if(read_reg.bits.access_size != 2)
		LogWarning("    Request size isn't word (got %d)\n", read_reg.bits.access_size);

	return read_reg;
}

/**
	@brief Executes a single MIPS32 instruction in serial-exec mode
 */
void MicrochipPIC32Device::SerialExecuteInstruction(uint32_t insn, bool first)
{
	//TODO: use fastdata for this (this is slow but works)

	//Select control register
	EnterEjtagMode();

	//Wait for the CPU to request an instruction
	EjtagControlRegister read_reg = WaitForEjtagMemoryOperation(first);

	//Read the address register so we know what address it's trying to read
	uint32_t capture = 0;
	uint32_t zero = 0;
	SetIR(INST_ADDRESS);
	ScanDR((uint8_t*)&zero, (uint8_t*)&capture, 32);
	//LogDebug("    Read address = %08x\n", capture);

	//Sanity check it
	if(read_reg.bits.access_size != 2)
		LogWarning("    Request size isn't word (got %d)\n", read_reg.bits.access_size);
	if(read_reg.bits.proc_we)
		LogWarning("    Exec request isn't read (write 0x%08x)\n", capture);

	if( (capture == 0xff200200) && !first)
		LogWarning("Target CPU appears to have reset!\n");

	//Send the data
	SetIR(INST_DATA);
	ScanDR((uint8_t*)&insn, (uint8_t*)&capture, 32);
	//LogDebug("    Read capture = %08x\n", capture);

	//Send the control command to execute the instruction
	EjtagControlRegister write_reg;
	write_reg.word = 0;						//default to zero
	write_reg.bits.proc_access	= 0;		//clear processor-access bit
	write_reg.bits.probe_enable	= 1;		//enable debug probe
	write_reg.bits.debug_vector_pos	= 1;	//debug exception traps to DMSEG (emulated memory)
	SetIR(INST_CONTROL);
	ScanDR((uint8_t*)&write_reg.word, (uint8_t*)&read_reg.word, 32);
}

void MicrochipPIC32Device::SerialExecuteMemoryWrite(uint32_t addr, uint32_t data)
{
	//microMIPS? different insn encoding than I'm used to
	SerialExecuteInstruction( (addr & 0xffff0000) | 0x41a4 );	//lui a0, addr_hi
	SerialExecuteInstruction( (addr << 16) | 0x5084 );			//ori a0, a0, addr_lo

	SerialExecuteInstruction( (data & 0xffff0000) | 0x41a5 );	//lui a1, data_hi
	SerialExecuteInstruction( (data << 16) | 0x50a5 );			//ori a1, a1, data_lo

	SerialExecuteInstruction( 0x0000f8a4);						//sw a1, 0(a0)

	//for pic32mx???
	//SerialExecuteInstruction(0x3c040000 | (addr >> 16) );	//lui a0, addr_hi
	//SerialExecuteInstruction(0x34840000 | (addr & 0xffff));	//ori a0, a0, addr_lo
	//SerialExecuteInstruction(0x3c050000 | (data >> 16) );	//lui a1, data_hi
	//SerialExecuteInstruction(0x34a50000 | (data & 0xffff));	//ori a1, a1, data_lo
	//SerialExecuteInstruction(0xac850000);					//sw a1, 0(a0)
}

uint32_t MicrochipPIC32Device::SerialExecuteMemoryRead(uint32_t addr)
{
	//TODO: use fastdata for this (this is slow but works)

	//"verify memory without PE" has wrong instruction encoding in PIC32MM flash prog spec
	//MIPS32 vs microMIPS?
	SerialExecuteInstruction(0xff2041b3 );						//lui s3, 0xff20
	SerialExecuteInstruction( (addr & 0xffff0000) | 0x41a8 );	//lui t0, addr_hi
	SerialExecuteInstruction( (addr << 16) | 0x5108);			//ori t0, t0, addr_lo

	SerialExecuteInstruction(0x0000fd28);						//lw t1, 0(t0)
	SerialExecuteInstruction(0x0000f933);						//sw t1, 0(s3)
	SerialExecuteInstruction(0x0c000c00);						//nop, nop

	//Wait for the CPU to request something
	uint32_t capture = 0;
	uint32_t zero = 0;
	while(true)
	{
		EjtagControlRegister read_reg = WaitForEjtagMemoryOperation();

		//If it's a read, feed a NOP
		if(!read_reg.bits.proc_we)
		{
			uint32_t nop = 0x0c000c00;

			//Send the data
			SetIR(INST_DATA);
			ScanDR((uint8_t*)&nop, (uint8_t*)&capture, 32);
			//LogDebug("    Read capture = %08x\n", capture);

			//Send the control command to execute the instruction
			EjtagControlRegister write_reg;
			write_reg.word = 0;						//default to zero
			write_reg.bits.proc_access	= 0;		//clear processor-access bit
			write_reg.bits.probe_enable	= 1;		//enable debug probe
			write_reg.bits.debug_vector_pos	= 1;	//debug exception traps to DMSEG (emulated memory)
			SetIR(INST_CONTROL);
			ScanDR((uint8_t*)&write_reg.word, (uint8_t*)&read_reg.word, 32);
		}

		else
		{
			SetIR(INST_ADDRESS);
			ScanDR((uint8_t*)&zero, (uint8_t*)&capture, 32);

			//Sanity check it
			if(read_reg.bits.access_size != 2)
				LogWarning("    Request size isn't word (got %d)\n", read_reg.bits.access_size);
			if(!read_reg.bits.proc_we)
			{
				if(capture == 0xff200200)
					LogWarning("Target CPU appears to have reset!\n");
				else
					LogWarning("    Request isn't write (read 0x%08x)\n", capture);
			}

			//Get the data
			SetIR(INST_DATA);
			ScanDR((uint8_t*)&zero, (uint8_t*)&capture, 32);

			//Send the control command to complete the instruction
			EjtagControlRegister write_reg;
			write_reg.word = 0;						//default to zero
			write_reg.bits.proc_access	= 0;		//clear processor-access bit
			write_reg.bits.probe_enable	= 1;		//enable debug probe
			write_reg.bits.debug_vector_pos	= 1;	//debug exception traps to DMSEG (emulated memory)
			SetIR(INST_CONTROL);
			ScanDR((uint8_t*)&write_reg.word, (uint8_t*)&read_reg.word, 32);

			break;
		}
	}

	return capture;
}

void MicrochipPIC32Device::Reset()
{
	throw JtagExceptionWrapper(
		"Soft reset not implemented for this CPU yet",
		"");
}

bool MicrochipPIC32Device::IsProgrammed()
{
	ResetToIdle();

	//Go into debug-boot mode
	EnterSerialExecMode();

	//We don't have to read all memory!
	//If the reset vector is blank, then the chip is "blank" as in it won't boot.
	//This is different from a full blank-check-after-erase.
	uint32_t vector = SerialExecuteMemoryRead(0xbfc00000);

	//Blank? Device is blank
	if(vector == 0xffffffff)
		return false;

	else
		return true;
	/*
	//Turn off secondary oscillator (requires unlock sequence)
	//OSCCON = 0
	SerialExecuteMemoryWrite(0xBF803B30, 0xaa996655);
	SerialExecuteMemoryWrite(0xBF803B30, 0x556699aa);
	SerialExecuteMemoryWrite(0xBF802000, 0x00000000);
	SerialExecuteMemoryWrite(0xBF803B30, 0x00000000);

	//Turn on LED on RA2-3-4 (should not have to touch PPS)
	//ANSELA (0xBF80_2600) = 32'h0;
	//TRISA (0xBF80_2610) = 32'h03;
	//PORTA (0xBF80_2620) = 32'h1c;
	SerialExecuteMemoryWrite(0xBF802600, 0x00000000);
	SerialExecuteMemoryWrite(0xBF802610, 0x00000003);
	SerialExecuteMemoryWrite(0xBF802620, 0x0000001c);

	for(int i=0; i<5; i++)
	{
		SerialExecuteMemoryWrite(0xBF802620, 0x0000001c);
		usleep(500 * 1000);
		SerialExecuteMemoryWrite(0xBF802620, 0x00000000);
		usleep(500 * 1000);
	}
	*/
}

void MicrochipPIC32Device::Erase()
{
	LogIndenter li;
	LogVerbose("Bulk erase...\n");

	//Bulk erase
	EnterMtapMode();
	SendMchpCommand(MCHP_ERASE);

	//Clear reset
	SendMchpCommand(MCHP_DE_ASSERT_RST);

	//Poll every 10 ms until status is clear
	while(true)
	{
		auto stat = GetStatus();
		if(!stat.bits.flash_busy && stat.bits.cfg_rdy)
			break;
		usleep(10 * 1000);
	}

	//Read back

	ResetToIdle();
	EnterSerialExecMode();

	LogVerbose("Blank checking boot flash...\n");
	uint32_t flash_base = 0xBFC00000;
	uint32_t flash_end = flash_base + m_devinfo->boot_flash_size * 1024;
	for(uint32_t ptr = flash_base; ptr < flash_end; ptr += 4)
	{
		if( (ptr & 0xfff) == 0)
			LogDebug("    %08x / %08x\n", ptr, flash_end);

		uint32_t flash_value = SerialExecuteMemoryRead(ptr);
		if(flash_value != 0xffffffff)
		{
			LogDebug("Got non-blank value 0x%08x at address 0x%08x\n", flash_value, ptr);
			return;
		}
	}


	//Read back all memory
	LogVerbose("Blank checking program flash...\n");
	flash_base = 0xBD000000;
	flash_end = flash_base + m_devinfo->program_flash_size * 1024;
	for(uint32_t ptr = flash_base; ptr < flash_end; ptr += 4)
	{
		if( (ptr & 0xfff) == 0)
			LogDebug("    %08x / %08x\n", ptr, flash_end);

		uint32_t flash_value = SerialExecuteMemoryRead(ptr);
		if(flash_value != 0xffffffff)
		{
			LogDebug("Got non-blank value 0x%08x at address 0x%08x\n", flash_value, ptr);
			return;
		}
	}

	LogDebug("Device is blank\n");
}

void MicrochipPIC32Device::Program(FirmwareImage* /*image*/)
{
	throw JtagExceptionWrapper(
		"Not implemented",
		"");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JTAG support stuff, mode switching, etc

void MicrochipPIC32Device::EnterMtapMode()
{
	SetIR(INST_MTAP_SW_MCHP);
	ResetToIdle();
}

/**
	@brief Sends a MTAP command. Requires TAP to be in MCHP mode, not EJTAG mode.
 */
uint8_t MicrochipPIC32Device::SendMchpCommand(uint8_t cmd)
{
	unsigned char capture;
	SetIR(INST_MTAP_COMMAND);
	ScanDR(&cmd, &capture, 8);
	return capture;
}

void MicrochipPIC32Device::EnterEjtagMode()
{
	SetIR(INST_MTAP_SW_EJTAG);
	ResetToIdle();
}
