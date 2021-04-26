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
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <uart_if.h>
#include <config.h>
#include <utils.h>
#include <bsl.h>
#include <time.h>

char *UARTDevice;
char TITXTHexFile [80];

int ResetPin;
int TestPin;

//*****************************************************************************
// MSP430FR Image to use ******************************************************
//*****************************************************************************

#define NUM_SUPPORTED_SECTION   10

uint8_t * fram;
uint32_t fram_address[NUM_SUPPORTED_SECTION] = {0};
uint32_t fram_length_of_sections[NUM_SUPPORTED_SECTION] = {0};
uint32_t fram_sections = 0;
uint32_t fram_total_image_size = 0;

//*****************************************************************************
// Program the MSP430FR with the specified image ******************************
//*****************************************************************************
bool ProgramMSP430()
{
	bool numberOfErrors = 0;
	bool result = true;
	uint16_t section = 0;
	uint8_t * framStartOfData = 0;

	for (section = 0; section < fram_sections; section++)
	{
        printf("\n\n****Section: %d****\n\n", section);
        printf("Address: 0x%X\n", fram_address[section]);
        if (section == 0)
        {
            framStartOfData = fram;
        }
        else
        {
            framStartOfData = framStartOfData + fram_length_of_sections[section - 1];
        }


        result = WriteLargeDataToMemory(fram_address[section], fram_length_of_sections[section], framStartOfData);
        if (!result)
        {
        	printf("Write Large Data To Memory failed\n");
        	return false;
        }
        else
        {
        	printf("Write Large Data To Memory successful\n");
        }

	}

	if (numberOfErrors != 0)
	{
		return false;
	}
	return true;
}

bool CheckArgs(int argc, char * argv[])
{
	if ( argc == 5 ) /* argc should be 2 for correct execution */
    {
        printf("Reset Pin: %s\n", argv[1]);
        printf("Test Pin: %s\n", argv[2]);
        printf("UART Device: %s\n", argv[3]);
        
        ResetPin = atoi(argv[1]);
        printf("Reset Pin set to GPIO: %d\n", RESET_PIN);
        TestPin = atoi(argv[2]);
        printf("Test Pin set to GPIO: %d\n", TEST_PIN);
        UARTDevice = argv[3];
        printf("UART Device set to: %s\n", UARTDevice);
        strcpy(TITXTHexFile, argv[4]);
        printf("TI TXT Hex: %s\n", TITXTHexFile);

        return true;
    }

    return false;
}

bool ParseTITXTHexFile()
{
	char * line = NULL;
    size_t len = 0;
#if DEBUG
    ssize_t read;
    uint32_t index = 0;
#endif

	FILE *file = fopen(TITXTHexFile, "r");
	if ( file == 0 )
	{
		printf( "Could not open TI TXT Hex file\n" );
		return false;
	}

	while(getline(&line, &len, file) != -1)
	{
		char hexInChars[8] = {0};
	    char * split;
	    if (line[0] == '@')
	    {
	        if (fram_sections > 0)
	        {
	            fram_total_image_size += fram_length_of_sections[fram_sections - 1];
	        }
	        strcpy(hexInChars, "0x");
	        strcpy((char *)(hexInChars+2), &(line[1]));
	        uint32_t address = (uint32_t)strtol(hexInChars, NULL, 0);
	        fram_address[fram_sections] = address;
	        fram_sections++;
	    }
	    else if (line[0] == 'q')
	    {
	        fram_total_image_size += fram_length_of_sections[fram_sections - 1];
	        break;
	    }
	    else
	    {
	        split = strtok(line, " \r");
	        while(split != NULL)
	        {
	        	if (split[0] != '\n')
            	{
	            	fram_length_of_sections[fram_sections - 1]++;
	            }
	            split = strtok(NULL, " \r");
	        }
	    }
	}

	fclose(file);

	file = fopen(TITXTHexFile, "r");
	if ( file == 0 )
	{
		printf( "Could not open TI TXT Hex file\n" );
		return false;
	}

	fram = (uint8_t *) malloc(fram_total_image_size * sizeof(uint8_t));
	uint8_t * bytePtrForMemory = (uint8_t *)(fram);

    while(getline(&line, &len, file) != -1)
    {
        char hexInChars[8] = {0};
        char * split;
        if (line[0] == '@')
        {
            //Nothing to do
        }
        else if (line[0] == 'q')
        {
            //Done
            break;
        }
        else
        {
            split = strtok(line, " \r");
            while(split != NULL)
            {
            	if (split[0] != '\n')
            	{
	                strcpy(hexInChars, "0x");
	                strcpy((char *)(hexInChars+2), split);
	                uint8_t byte = (uint8_t)strtol(hexInChars, NULL, 0);
	                *(bytePtrForMemory++) = byte;
            	}
                split = strtok(NULL, " \r");
            }
        }
    }
    fclose(file);

#if DEBUG
    printf("\nfram: \n");
    for (index = 0; index < fram_total_image_size; index++)
    {
    	if (index % 8 == 0 && index != 0)
    	{
    		printf("\n");
    	}
    	printf("0x%x, ", fram[index]);
    }
    printf("\n");

    printf("\nfram_address: \n");
    for (index = 0; index < fram_sections; index++)
    {
    	if (index % 8 == 0 && index != 0)
    	{
    		printf("\n");
    	}
    	printf("0x%x, ", fram_address[index]);
    }
    printf("\n");

    printf("\nfram_length_of_sections \n");
    for (index = 0; index < fram_sections; index++)
    {
    	if (index % 8 == 0 && index != 0)
    	{
    		printf("\n");
    	}
    	printf("%d, ", fram_length_of_sections[index]);
    }
    printf("\n\n");
#endif

	return true;
}

#define MAX_RETRY	5

int main(int argc, char * argv[])
{
	bool result = true;

	if (!CheckArgs(argc, argv))
	{
		printf("Error in your command line arguments!\n");
		printf("%s <reset pin> <test pin> <uart device> <firmware>\n", argv[0]);
		printf("Serial DTR pin = 2\n");
		printf("Serial RTS pin = 4\n");
		return 1;
	}

	if (!ParseTITXTHexFile())
	{
		printf("Parse TI TXT Hex file failed!\n");
		return 1;
	}

	UART_Initialize(UARTDevice);
	printf("Waiting for timeout to ensure input buffer is empty!\n");
	UART_Flush();
	printf("Starting the BSL commands!\n");
	uint8_t retry = MAX_RETRY;

	while (retry)
	{
		printf("\n\n*****New Attempt*****\n\n");
		BSLEntrySequence();
		MsDelay(100);
		result = WritePasswordDefault();
		if (!result)
		{
			printf("Write password failed\n");
			retry --;
			continue;
		}
		printf("Write Password was successful\n");
		uint8_t deviceIDBytes[2] = {0};
		result = ReadMemory(0x1A04, 2, deviceIDBytes);
		if (!result)
		{
			printf("Device ID read failed\n");
			retry --;
			continue;
		}
		uint16_t deviceID = deviceIDBytes[0] << 8 | deviceIDBytes[1];
		printf("Device ID: 0x%X\n", deviceID);

		result = ProgramMSP430();
		if (!result)
		{
			printf("\nMSP430 programming failed\n");
			retry --;
			continue;
		}

		free(fram);

		printf("\nMSP430 programmed successfully\n");
		Reset();
		printf("Device is reset\n");
		UART_Close();
		return 0;
	}

	UART_Close();
	printf("Max retries exceeded\n");
	return 1;
}


