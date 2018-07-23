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
	@brief Implementation of FreescaleDevice
 */

#include "jtaghal.h"
#include "JEDECVendorID_enum.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

/**
	@brief Initializes this device

	@param idcode	The ID code of this device
	@param iface	The JTAG adapter this device was discovered on
	@param pos		Position in the chain that this device was discovered
 */
FreescaleDevice::FreescaleDevice(unsigned int idcode, JtagInterface* iface, size_t pos)
: JtagDevice(idcode, iface, pos)
{
}

/**
	@brief Default virtual destructor
 */
FreescaleDevice::~FreescaleDevice()
{

}

/**
	@brief Creates a FreescaleDevice given an ID code

	@throw JtagException if the ID code supplied is not a valid Freescale device, or not a known family number

	@param idcode	The ID code of this device
	@param iface	The JTAG adapter this device was discovered on
	@param pos		Position in the chain that this device was discovered

	@return A valid JtagDevice object, or NULL if the vendor ID was not recognized.
 */
JtagDevice* FreescaleDevice::CreateDevice(unsigned int idcode, JtagInterface* iface, size_t pos)
{
	//Save the original ID code to give to the derived class
	unsigned int idcode_raw = idcode;

	//Rightmost bit is always a zero, ignore it
	idcode >>= 1;

	//Sanity check manufacturer ID
	if( (idcode & 0x7FF) != VENDOR_ID_FREESCALE)
	{
		throw JtagExceptionWrapper(
			"Invalid IDCODE supplied (wrong JEDEC manufacturer ID, not a Freescale device)",
			"");
	}
	idcode >>= 11;

	//Next 16 bits are part number
	unsigned int partnum = idcode & 0xffff;
	idcode >>= 16;

	//then last 4 are stepping
	unsigned int stepping = idcode;

	LogWarning("Unimplemented Freescale device (part=0x%x, stepping=0x%x)\n", partnum, stepping);
	return NULL;

	//Create the device
	//return FreescalePIC32Device::CreateDevice(partnum, stepping, idcode_raw, iface, pos);
}
