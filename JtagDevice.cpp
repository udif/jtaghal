/***********************************************************************************************************************
*                                                                                                                      *
* ANTIKERNEL v0.1                                                                                                      *
*                                                                                                                      *
* Copyright (c) 2012-2018 Andrew D. Zonenberg                                                                          *
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
	@brief Implementation of JtagDevice
 */

#include "jtaghal.h"
#include "JEDECVendorID_enum.h"
#include "UserVID_enum.h"
#include "UserPID_enum.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

/**
	@brief Initializes this device

	NOTE: Do not do probes/scans of the chain during the constructor, because we haven't initialized all TAPs yet.
	This means that shift operations may not yet have the correct padding etc.

	Any initialization that involves querying the chain should be done in PostInitProbes().

	@param idcode	The ID code of this device
	@param iface	The JTAG adapter this device was discovered on
	@param pos		Position in the chain that this device was discovered
	@param irlength	Length of the JTAG instruction register
 */
JtagDevice::JtagDevice(uint32_t idcode, JtagInterface* iface, size_t pos, size_t irlength)
: m_irlength(irlength)
, m_idcode(idcode)
, m_iface(iface)
, m_pos(pos)
{
	memset(m_cachedIR, 0xFF, sizeof(m_cachedIR));
}

/**
	@brief Default virtual destructor
 */
JtagDevice::~JtagDevice()
{
}

/**
	@brief Creates a JtagDevice given an ID code

	@param idcode	The ID code of this device
	@param iface	The JTAG adapter this device was discovered on
	@param pos		Position in the chain that this device was discovered

	@return A valid JtagDevice object, or NULL if the vendor ID was not recognized.
 */
JtagDevice* JtagDevice::CreateDevice(uint32_t idcode, JtagInterface* iface, size_t pos)
{
	//Rightmost bit is always a zero, ignore it
	unsigned int idcode_s = (idcode >> 1) & 0x7ff;

	//Switch on the ID code and create the appropriate device
	switch(idcode_s)
	{
	case VENDOR_ID_ARM:
	case VENDOR_ID_ARM_TEST:
		return ARMDevice::CreateDevice(idcode, iface, pos);
		break;

	case VENDOR_ID_FREESCALE:
		return FreescaleDevice::CreateDevice(idcode, iface, pos);

	case VENDOR_ID_MICROCHIP:
		return MicrochipDevice::CreateDevice(idcode, iface, pos);

	case VENDOR_ID_PHILIPS:
		LogWarning("[JtagDevice] Philips not implemented. Is this an older XPLA3 die?\n");
		//FIXME: Check for CoolRunner XPLA3 device IDs and call XilinxDevice for them
		//See Xilinx AR#14761
		break;

	case VENDOR_ID_STMICRO:
		return STMicroDevice::CreateDevice(idcode, iface, pos);

	case VENDOR_ID_XILINX:
		return XilinxDevice::CreateDevice(idcode, iface, pos);

	default:
		//TODO: Throw exception instead?
		LogError("[JtagDevice] WARNING: Manufacturer ID 0x%x not recognized (%08x)\n", idcode_s, idcode);
		return NULL;
	}

	return NULL;

}

/**
	@brief Returns the JEDEC ID code of this device.
 */
unsigned int JtagDevice::GetIDCode()
{
	return m_idcode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JTAG interface helpers

/**
	@brief Wrapper around JtagInterface::SetIRDeferred()

	See JtagInterface documentation for more details.
*/
void JtagDevice::SetIRDeferred(const unsigned char* data, int count)
{
	//ceil of bytes, but faster
	int bytecount = (count >> 3);
	if(count & 7)
		bytecount ++;

	if( (m_irlength < 32) && (0 == memcmp(data, &m_cachedIR, bytecount)))
	{
		//do nothing, cache hit
		return;
	}

	else
		m_iface->SetIRDeferred((int)m_pos, data, count);

	memcpy(m_cachedIR, data, bytecount);
}

/**
	@brief Wrapper around JtagInterface::SetIR()

	See JtagInterface documentation for more details.
 */
void JtagDevice::SetIR(const unsigned char* data, int count)
{
	//ceil of bytes, but faster
	int bytecount = (count >> 3);
	if(count & 7)
		bytecount ++;

	if(count > 32)
	{
		throw JtagExceptionWrapper(
			"Invalid IR value (too long)",
			"");
	}

	if( (0 == memcmp(data, &m_cachedIR, bytecount)) )
	{
		//do nothing, cache hit
		return;
	}

	else
		m_iface->SetIR((int)m_pos, data, count);

	memcpy(m_cachedIR, data, bytecount);
}

/**
	@brief Wrapper around JtagInterface::Commit()

	See JtagInterface documentation for more details.
 */
void JtagDevice::Commit()
{
	m_iface->Commit();
}

/**
	@brief Wrapper around JtagInterface::SetIR()

	See JtagInterface documentation for more details.
 */
void JtagDevice::SetIR(const unsigned char* data, unsigned char* data_out, int count)
{
	//ceil of bytes, but faster
	int bytecount = (count >> 3);
	if(count & 7)
		bytecount ++;


	if(count > 32)
	{
		throw JtagExceptionWrapper(
			"Invalid IR value (too long)",
			"");
	}

	m_iface->SetIR((int)m_pos, data, data_out, count);
	memcpy(m_cachedIR, data, bytecount);
}

/**
	@brief Wrapper around JtagInterface::ScanDR()

	See JtagInterface documentation for more details.
 */
void JtagDevice::ScanDR(const unsigned char* send_data, unsigned char* rcv_data, int count)
{
	m_iface->ScanDR((int)m_pos, send_data, rcv_data, count);
}

/**
	@brief Wrapper around JtagInterface::IsSplitScanSupported()

	See JtagInterface documentation for more details.
 */
bool JtagDevice::IsSplitScanSupported()
{
	return m_iface->IsSplitScanSupported();
}

/**
	@brief Wrapper around JtagInterface::ScanDRSplitWrite()

	See JtagInterface documentation for more details.
 */
void JtagDevice::ScanDRSplitWrite(const unsigned char* send_data, unsigned char* rcv_data, int count)
{
	m_iface->ScanDRSplitWrite((int)m_pos, send_data, rcv_data, count);
}

/**
	@brief Wrapper around JtagInterface::ScanDRSplitRead()

	See JtagInterface documentation for more details.
 */
void JtagDevice::ScanDRSplitRead(unsigned char* rcv_data, int count)
{
	m_iface->ScanDRSplitRead((int)m_pos, rcv_data, count);
}

/**
	@brief Wrapper around JtagInterface::ScanDRDeferred()

	See JtagInterface documentation for more details.
 */
void JtagDevice::ScanDRDeferred(const unsigned char* send_data, int count)
{
	m_iface->ScanDRDeferred((int)m_pos, send_data, count);
}

/**
	@brief Wrapper around JtagInterface::EnterShiftDR()

	See JtagInterface documentation for more details.
 */
void JtagDevice::EnterShiftDR()
{
	m_iface->EnterShiftDR();
}

/**
	@brief Wrapper around JtagInterface::ShiftData()

	See JtagInterface documentation for more details.
 */
void JtagDevice::ShiftData(const unsigned char* send_data, unsigned char* rcv_data, int count)
{
	m_iface->ShiftData((int)m_pos, send_data, rcv_data, count);
}

/**
	@brief Wrapper around JtagInterface::SendDummyClocks()

	See JtagInterface documentation for more details.
 */
void JtagDevice::SendDummyClocks(int n)
{
	m_iface->SendDummyClocks(n);
}

/**
	@brief Wrapper around JtagInterface::SendDummyClocksDeferred()

	See JtagInterface documentation for more details.
 */
void JtagDevice::SendDummyClocksDeferred(int n)
{
	m_iface->SendDummyClocksDeferred(n);
}

/**
	@brief Wrapper around JtagInterface::ResetToIdle()

	See JtagInterface documentation for more details.
 */
void JtagDevice::ResetToIdle()
{
	m_iface->ResetToIdle();
}

/**
	@brief Prints lots of general debug / system information
 */
void JtagDevice::PrintInfo()
{
	LogNotice("%2d: %s\n", (int)m_pos, GetDescription().c_str());
	LogIndenter li;

	//Is it programmable? If so, get some more details
	ProgrammableDevice* pprog = dynamic_cast<ProgrammableDevice*>(this);
	if(pprog != NULL)
	{
		LogNotice("Device is programmable\n");
		{
			LogIndenter li;

			if(pprog->IsProgrammed())
				LogNotice("Device is configured\n");
			else
				LogNotice("Device is blank\n");
		}

		//Is it an FPGA? If so, get FPGA-specific information
		//Get the serial number only if the device is blank since some FPGAs (most Xilinx f.ex)
		//can't get the S/N when they're configured
		FPGA* pfpga = dynamic_cast<FPGA*>(this);
		if(pfpga != NULL)
		{
			LogNotice("Device is an FPGA\n");
			LogIndenter li;
			if(pprog->IsProgrammed())
			{
				JtagFPGA* jf = dynamic_cast<JtagFPGA*>(pfpga);
				if(jf)
				{
					unsigned int vid;
					unsigned int pid;
					if(jf->GetUserVIDPID(vid, pid))
					{
						LogNotice("Detected user VID/PID code\n");
						LogIndenter li;

						string vendor = "unknown";
						string product = "unknown";

						if(vid == VID_AZONENBERG)
						{
							vendor = "Andrew Zonenberg";
							switch(pid)
							{
								case PID_AZONENBERG_ANTIKERNEL_NOC:
									product = "Antikernel NoC Interface";
									break;

								case PID_AZONENBERG_SPI_INDIRECT:
									product = "SPI Indirect Programming";
									break;
							}
						}

						LogNotice("idVendor  = 0x%06x (%s)\n", vid, vendor.c_str());
						LogNotice("idProduct = 0x%02x     (%s)\n", pid, product.c_str());
					}

					else
						LogNotice("User VID/PID not specified or unreadable\n");
				}
				else
				{
					LogNotice("Not a JTAG-capable FPGA\n");
				}
			}
		}

		//Is it a CPLD? If so, get CPLD-specific information
		CPLD* pcpld = dynamic_cast<CPLD*>(this);
		if(pcpld != NULL)
			printf("Device is a CPLD\n");
	}

	//Is it debuggable? If so, get some more details
	DebuggerInterface* pdebug = dynamic_cast<DebuggerInterface*>(this);
	if(pdebug != NULL)
	{
		LogNotice("Device is a debug interface\n");
		/*
		size_t ntargets = pdebug->GetNumTargets();
		printf("    %zu targets present\n", ntargets);
		for(size_t i=0; i<ntargets; i++)
			printf("        %zu: %s\n", i, pdebug->GetTarget(i)->GetDescription().c_str());
		*/
	}

	//Does it have a serial number? If so, get more details
	SerialNumberedDevice* pserial = dynamic_cast<SerialNumberedDevice*>(this);
	if(pserial != NULL)
	{
		//If the device is programmable, check if it requires a reset before printing the serial number
		bool skipRead = false;
		if(pprog)
		{
			if(pserial->ReadingSerialRequiresReset() && pprog->IsProgrammed())
			{
				int bitlen = pserial->GetSerialNumberLengthBits();
				LogNotice("Device has unique serial number (%d bits long), but cannot read without a reset\n", bitlen);
				skipRead = true;
			}
		}

		//Read the serial number if we can do so without a reboot, or if it's blank
		if(!skipRead)
		{
			LogNotice("Device has unique serial number (%d bits long)\n", pserial->GetSerialNumberLengthBits());
			LogIndenter li;
			LogNotice("%s\n", pserial->GetPrettyPrintedSerialNumber().c_str() );
		}
	}

	//Does it have read protection? If so, get more details
	LockableDevice* plock = dynamic_cast<LockableDevice*>(this);
	if(plock != NULL)
	{
		LogNotice("Device has security bits\n");
		LogIndenter li;

		UncertainBoolean locked = plock->IsDeviceReadLocked();
		if(locked.GetValue())
			LogNotice("Device is read locked (%s)\n", locked.GetCertaintyAsText());
		else
			LogNotice("Device is not read locked (%s)\n", locked.GetCertaintyAsText());
	}
}
