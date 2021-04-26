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

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <termios.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <uart_if.h>
#include <config.h>
#include <bsl.h>
#include <utils.h>

//*****************************************************************************
// Buffer for send and receive ************************************************
//*****************************************************************************

uint8_t sendBuffer[270] = {0};
uint8_t receiveBuffer[270] = {0};

//*****************************************************************************
// Commands *******************************************************************
//*****************************************************************************

#define RX_DATA_BLOCK_RESP_CMD	0x3B
#define RX_DATA_BLOCK_RESP_NL	0x02
#define RX_DATA_BLOCK_RESP_NH	0x00

#define PASSWORD_LENGTH 		32
#define RX_PASSWORD_NL			0x21
#define RX_PASSWORD_NH			0x00
#define RX_PASSWORD_RESP_CMD	0x3B
#define RX_PASSWORD_RESP_NL		0x02
#define RX_PASSWORD_RESP_NH		0x00

#define MASS_ERASE_NL		0x01
#define MASS_ERASE_NH		0x00
#define MASS_ERASE_RESP_CMD	0x3B
#define MASS_ERASE_RESP_NL	0x02
#define MASS_ERASE_RESP_NH	0x00

#define LOAD_PC_NL			0x04
#define LOAD_PC_NH			0x00

#define TX_DATA_BLOCK_NL	0x06
#define TX_DATA_BLOCK_NH	0x00
#define TX_DATA_RESP_CMD	0x3A	


#define RX_PASSWORD 		0x11  
#define TX_DATA_BLOCK		0x18
#define RX_DATA_BLOCK		0x10
#define MASS_ERASE			0x15
#define LOAD_PC				0x17

#define HEADER				0x80

#define ACK 				0x00
#define HEADER_INCORRECT	0x51
#define CHECKSUM_INCORRECT	0x52
#define PACKET_SIZE_ZERO	0x53
#define PACKET_SIZE_EXCEEDS	0x54
#define UNKNOWN_ERROR		0x55
#define UNKNOWN_BAUDRATE	0x56
#define PACKET_SIZE_ERROR	0x57

#define GetCKL(cs)			(uint8_t)(cs)
#define GetCKH(cs)			(uint8_t)(cs >> 8)

//*****************************************************************************
// Write The Default Password *************************************************
// Uses the write password function to write the default password (0xFF) ******
//*****************************************************************************

bool WritePasswordDefault()
{
	uint8_t password[PASSWORD_LENGTH] = {0};
	uint16_t loopIndex = 0;

	for (loopIndex = 0; loopIndex < PASSWORD_LENGTH; loopIndex++)
	{
		password[loopIndex] = 0xFF;
	}

	return WritePassword(password, PASSWORD_LENGTH);
}

//*****************************************************************************
// Write the BSL password *****************************************************
// password: The password for BSL *********************************************
// passwordSize: The size of the password array *******************************
//*****************************************************************************

bool WritePassword(uint8_t* password, uint16_t passwordSize)
{
	uint16_t checksum = 0;

	if (passwordSize != PASSWORD_LENGTH)
	{
#if DEBUG
		printf("Password is incorrect size: 0x%x\n", passwordSize);
#endif
		return false;
	}
	sendBuffer[0] = (uint8_t)(HEADER);
	sendBuffer[1] = RX_PASSWORD_NL;
	sendBuffer[2] = RX_PASSWORD_NH;
	sendBuffer[3] = RX_PASSWORD;

	memcpy(&sendBuffer[4], password, PASSWORD_LENGTH);

	checksum = CalculateChecksum(&sendBuffer[3], (uint16_t)(PASSWORD_LENGTH + 1));
	sendBuffer[PASSWORD_LENGTH + 4] = GetCKL(checksum);
	sendBuffer[PASSWORD_LENGTH + 5] = GetCKH(checksum);

	UART_SendByteArray(sendBuffer, PASSWORD_LENGTH + 6);
	UART_ReadByteArray(receiveBuffer, 8);


	MsDelay(5);

	if ((receiveBuffer[0] == ACK)
		&&(receiveBuffer[1] == HEADER)
		&&(receiveBuffer[2] == RX_PASSWORD_RESP_NL)
		&&(receiveBuffer[3] == RX_PASSWORD_RESP_NH)
		&&(receiveBuffer[4] == RX_PASSWORD_RESP_CMD)
		&&(receiveBuffer[5] == 0x00))
	{
		return true;
	}

	return false;
}

//*****************************************************************************
// Read the MSP memory ********************************************************
// startAddress: The address to start reading *********************************
// lenght: The length of the memory block to be read **************************
// dataResult: The array to contain the data read *****************************
//*****************************************************************************

bool ReadMemory(uint32_t startAddress, uint8_t lenght, uint8_t * dataResult)
{
	uint16_t checksum = 0;
	uint16_t loopIndex = 0;
	uint8_t uartError;


	sendBuffer[0] = (uint8_t)(HEADER);
	sendBuffer[1] = (uint8_t)(TX_DATA_BLOCK_NL);
	sendBuffer[2] = (uint8_t)(TX_DATA_BLOCK_NH);
	sendBuffer[3] = TX_DATA_BLOCK;

	sendBuffer[4] = (uint8_t)(startAddress & 0x00ff);
	sendBuffer[5] = (uint8_t)((startAddress >> 8) & 0x00ff);
	sendBuffer[6] = (uint8_t)((startAddress >> 16) & 0x00ff);
	sendBuffer[7] = (uint8_t)(lenght & 0x00ff);
	sendBuffer[8] = (uint8_t)((lenght >> 8) & 0x00ff);

	checksum = CalculateChecksum(&sendBuffer[3], 6);
	sendBuffer[9] = (uint8_t)(checksum);
	sendBuffer[10] = (uint8_t)(checksum >> 8);

	UART_SendByteArray(sendBuffer, 5 + 6);
	MsDelay(5);
	UART_ReadByteArrayWithTimeout(receiveBuffer, lenght + 7, READ_TIMEOUT, &uartError);

	if ((receiveBuffer[0] != ACK)
		&&(receiveBuffer[1] != HEADER))
	{
#if DEBUG
		printf("HEADER is incorrect: 0x%x\n", receiveBuffer[0]);
#endif
		return false;
	}
	else
	{
		for (loopIndex = 0; loopIndex < lenght; loopIndex ++)
		{
			dataResult[loopIndex] = receiveBuffer[5 + loopIndex];
		}

	}

	return true;
}

//*****************************************************************************
// Write data to MSP memory ***************************************************
// startAddress: The address to start the memory write ************************
// length: The length of the data to be writtem *******************************
// data: The array containing the data to write ******************************* 
//*****************************************************************************

bool WriteMemory(uint32_t startAddress, uint8_t lenght, uint8_t * data)
{
	uint16_t checksum = 0;
	uint8_t uartError;

	sendBuffer[0] = (uint8_t)(HEADER);
	sendBuffer[1] = (uint8_t)((lenght + 4) & 0x00ff);
	sendBuffer[2] = (uint8_t)(((lenght + 4) >> 8) & 0x00ff);
	sendBuffer[3] = RX_DATA_BLOCK;

	sendBuffer[4] = (uint8_t)(startAddress & 0x00ff);
	sendBuffer[5] = (uint8_t)((startAddress >> 8) & 0x00ff);
	sendBuffer[6] = (uint8_t)((startAddress >> 16) & 0x00ff);

	memcpy(&sendBuffer[7], data, lenght);

	checksum = CalculateChecksum(&sendBuffer[3], lenght + 4);
	sendBuffer[lenght + 7] = (uint8_t)(checksum);
	sendBuffer[lenght + 8] = (uint8_t)(checksum >> 8);

	UART_SendByteArray(sendBuffer, lenght + 9);
	UART_ReadByteArrayWithTimeout(receiveBuffer, 8, READ_TIMEOUT, &uartError);
	MsDelay(5);

	if ((receiveBuffer[0] == ACK)
		&&(receiveBuffer[1] == HEADER)
		&&(receiveBuffer[2] == RX_DATA_BLOCK_RESP_NL)
		&&(receiveBuffer[3] == RX_DATA_BLOCK_RESP_NH)
		&&(receiveBuffer[4] == RX_DATA_BLOCK_RESP_CMD)
		&&(receiveBuffer[5] == 0x00))
	{
		return true;
	}

	return false;
}

//*****************************************************************************
// Write large data array to memory. This includes data with length larger ****
// than MAX_UART_BSL_BUFFER_SIZE **********************************************
// startAddress: The address to start the memory write ************************
// length: The length of the data to be writtem *******************************
// data: The array containing the data to write ******************************* 
//*****************************************************************************

#define MAX_UART_BSL_BUFFER_SIZE 200

bool WriteLargeDataToMemory(uint32_t startAddress, uint32_t length, uint8_t * data)
{
	uint32_t currentAddress = startAddress;
	uint32_t currentLength = length;
	uint8_t * currentData = data;
	bool done = false;
	bool result = true;

	while(!done)
	{
		if (currentLength <= 0)
		{
			done = true;
		}
		else if (currentLength < MAX_UART_BSL_BUFFER_SIZE)
		{
			result = WriteMemory(currentAddress, currentLength, currentData);
			if (!result)
			{
				return result;
			}
			done = true;
		}
		else
		{
			result = WriteMemory(currentAddress, MAX_UART_BSL_BUFFER_SIZE, currentData);
			if (!result)
			{
				return result;
			}
			currentAddress += MAX_UART_BSL_BUFFER_SIZE;
			currentData += MAX_UART_BSL_BUFFER_SIZE;
			currentLength -= MAX_UART_BSL_BUFFER_SIZE;
		}
	}

	return true;
}

//*****************************************************************************
// Perform a mass erase *******************************************************
//*****************************************************************************

bool MassErase()
{
	uint16_t checksum = 0;
	uint8_t uartError;

	sendBuffer[0] = (uint8_t)(HEADER);
	sendBuffer[1] = MASS_ERASE_NL;
	sendBuffer[2] = MASS_ERASE_NH;
	sendBuffer[3] = MASS_ERASE;

	checksum = CalculateChecksum(&sendBuffer[3], 1);
	sendBuffer[4] = (uint8_t)(checksum);
	sendBuffer[5] = (uint8_t)(checksum >> 8);

	UART_SendByteArray(sendBuffer, 6);
	UART_ReadByteArrayWithTimeout(receiveBuffer, 8, READ_TIMEOUT, &uartError);
	MsDelay(5);

	if ((receiveBuffer[0] == ACK)
		&&(receiveBuffer[1] == HEADER)
		&&(receiveBuffer[2] == MASS_ERASE_RESP_NL)
		&&(receiveBuffer[3] == MASS_ERASE_RESP_NH)
		&&(receiveBuffer[4] == MASS_ERASE_RESP_CMD)
		&&(receiveBuffer[5] == 0x00))
	{
		return true;
	}

	return false;
}

//*****************************************************************************
// Load the program counter with the specified address ************************
//*****************************************************************************

bool LoadPC(uint32_t startAddress)
{
	uint16_t checksum = 0;

	sendBuffer[0] = (uint8_t)(HEADER);
	sendBuffer[1] = LOAD_PC_NL;
	sendBuffer[2] = LOAD_PC_NH;
	sendBuffer[3] = LOAD_PC;

	sendBuffer[4] = (uint8_t)(startAddress & 0x00ff);
	sendBuffer[5] = (uint8_t)((startAddress >> 8) & 0x00ff);
	sendBuffer[6] = (uint8_t)((startAddress >> 16) & 0x00ff);

	checksum = CalculateChecksum(&sendBuffer[3], 4);
	sendBuffer[7] = (uint8_t)(checksum);
	sendBuffer[8] = (uint8_t)(checksum >> 8);

	UART_SendByteArray(sendBuffer, 9);
	MsDelay(5);

	return true;
}

//*****************************************************************************
// Calculate the CRC16 ********************************************************
// data_p: Pointer to the array containing the data for CRC16 *****************
// lenght: The length of the data for CRC16 ***********************************
//*****************************************************************************

uint16_t CalculateChecksum(const uint8_t* data_p, uint16_t length){
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}

//*****************************************************************************
// Enter BSL mode using RST and TST pins **************************************
//*****************************************************************************

void BSLEntrySequence()
{
	UART_SetPinHigh(TEST_PIN);
	UART_SetPinHigh(RESET_PIN);
	MsDelay(200);

	UART_SetPinLow(TEST_PIN);
	usDelay(800);
	UART_SetPinLow(RESET_PIN);
	usDelay(20);

	UART_SetPinHigh(TEST_PIN);
	usDelay(20);
	UART_SetPinLow(TEST_PIN);
	usDelay(20);

	UART_SetPinHigh(TEST_PIN);
	usDelay(20);
	UART_SetPinHigh(RESET_PIN);
	usDelay(20);
	UART_SetPinLow(TEST_PIN);
	usDelay(20);

}

//*****************************************************************************
// Reset the MSP. Exits the BSL Mode ******************************************
//*****************************************************************************

void Reset()
{
	UART_SetPinLow(TEST_PIN);
	UART_SetPinHigh(RESET_PIN);
	MsDelay(200);

	UART_SetPinLow(RESET_PIN);
	usDelay(100);
	UART_SetPinHigh(RESET_PIN);
	usDelay(100);
}
