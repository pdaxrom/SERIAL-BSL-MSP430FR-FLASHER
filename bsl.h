/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

//******************************************************************************
// Version history:
// 1.0 07/17             Initial version. (Nima Eskandari)
// 1.1 07/17             Added Comments. (Nima Eskandari)
//----------------------------------------------------------------------------
//   Designed 2017 by Texas Instruments
//
//   Nima Eskandari
//   Texas Instruments Inc.
//   August 2017
//   Built with CCS Version: Code Composer Studio v7
//******************************************************************************

#ifndef BSL_H__
#define BSL_H__

#include <stdint.h>
#include <stdbool.h>

extern int ResetPin;
extern int TestPin;

#define RESET_PIN	ResetPin
#define TEST_PIN	TestPin

uint16_t CalculateChecksum(const uint8_t* data_p, uint16_t length);
bool WritePasswordDefault();
bool WritePassword(uint8_t* password, uint16_t passwordSize);
bool ReadMemory(uint32_t startAddress, uint8_t lenght, uint8_t * dataResult);
bool WriteMemory(uint32_t startAddress, uint8_t lenght, uint8_t * data);
void BSLEntrySequence();
void Reset();
bool MassErase();
bool LoadPC(uint32_t startAddress);
bool WriteLargeDataToMemory(uint32_t startAddress, uint32_t length, uint8_t * data);

#endif