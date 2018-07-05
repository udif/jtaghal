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
	@brief Declaration of ARMAPBDevice
 */

#ifndef ARMAPBDevice_h
#define ARMAPBDevice_h

#include <stdlib.h>

#include "ARMDebugPeripheralIDRegister.h"

class ARMDebugMemAccessPort;

/**
	@brief A device attached to an ARM APB bus (may be a debug core or something else)

	\ingroup libjtaghal
 */
class ARMAPBDevice
{
public:
	ARMAPBDevice(ARMDebugMemAccessPort* ap, uint32_t address, ARMDebugPeripheralIDRegisterBits idreg);
	virtual ~ARMAPBDevice();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// General device info

	virtual std::string GetDescription() =0;
	virtual void PrintInfo() =0;

protected:

	///reads a register given the offset from our base address
	uint32_t ReadRegisterByOffset(uint32_t offset);

	///reads a register given the index into a 32-bit register space
	uint32_t ReadRegisterByIndex(uint32_t index);

	///The Mem-AP
	ARMDebugMemAccessPort* m_ap;

	///The peripheral ID register
	ARMDebugPeripheralIDRegisterBits m_idreg;

	///Base address of the device
	uint32_t m_address;
};

#endif
