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
#include <stddef.h>
#include <termios.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <uart_if.h>
#include <config.h>
#include <time.h>
#include <utils.h>

int fd;
struct termios uart4, old;
uint8_t rec[1024];


//*****************************************************************************
// Initialize UART ************************************************************
// Opens the serial port (UART communication) *********************************
//*****************************************************************************
void UART_Initialize(char *device)
{
	char buf[30] = "/dev/ttyS4";
	if (device) {
	    strncpy(buf, device, sizeof(buf));
	}
	printf("Initializing UART: %s\n", buf);
	fd = open(buf, O_RDWR | O_NOCTTY);
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (fd < 0)
		printf("port failed to open\n");
	tcgetattr(fd, &old);
	bzero(&uart4, sizeof(uart4));
	uart4.c_cflag = B9600 | CS8 | CLOCAL | CREAD | PARENB;
	uart4.c_iflag = IGNPAR | ICRNL;
	uart4.c_oflag = 0;
	uart4.c_lflag = 0;

	uart4.c_cc[VTIME] = 0;
	uart4.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &uart4);
}

//*****************************************************************************
// Send byte through UART *****************************************************
// byte: the value of the byte to write ***************************************
//*****************************************************************************
void UART_SendByte(uint8_t byte)
{
	write(fd, &byte, 1);
}

//*****************************************************************************
// Send byte array through UART ***********************************************
// byteArray: the array of the bytes to write *********************************
// size: the size of the data array *******************************************
//*****************************************************************************
void UART_SendByteArray(uint8_t * byteArray, uint16_t size)
{
	write(fd, byteArray, size);
}

//*****************************************************************************
// Read byte through UART *****************************************************
//*****************************************************************************
uint8_t UART_ReadByte()
{
	while (1)
	{
		if (read(fd, &rec, 1) > 0)
		{
			return rec[0];
		}
	}
}

//*****************************************************************************
// Read byte through UART with the specified timeout **************************
// timeout: number of seconds before timeout **********************************
// error: data containor for error (whether the communication timed out) ******
//*****************************************************************************
uint8_t UART_ReadByteWithTimeout(uint16_t timeout, uint8_t * error)
{
	uint32_t t1, t2, total;
	t1 = GetTime();
#if DEBUG
		printf("Receiving One Byte with timeout\n");
#endif
	while (1)
	{
		if (read(fd, &rec, 1) > 0)
		{
			error = UART_Successful;
#if DEBUG
			printf("0x%x\n", rec[0]);
			printf("Done Receiving One Byte\n");
#endif
			return rec[0];
		}
		t2 = GetTime();

		total = t2 - t1;
		if (total > timeout)
		{
			*error = UART_Timeout;
#if DEBUG
			printf("Receive Timeout\n", rec[0]);
#endif
			return 0xFF;
		}
	}
}

//*****************************************************************************
// Read byte array through UART ***********************************************
// byteArray: The data read from UART is stored in this array *****************
// size: the size of the data to read *****************************************
//*****************************************************************************
void UART_ReadByteArray(uint8_t * byteArray, uint16_t size)
{
	int index = 0;
#if DEBUG
			printf("Receiving Data:\n", rec[0]);
#endif
	while (size)
	{
		if (read(fd, &rec, 1) > 0)
		{
#if DEBUG
			printf("0x%x\n", rec[0]);
#endif
			byteArray[index++] = rec[0];
			size--;
		}
	}
#if DEBUG
			printf("Done Receiving Data\n");
#endif
}

//*****************************************************************************
// Read byte array through UART ***********************************************
// byteArray: The data read from UART is stored in this array *****************
// size: the size of the data to read *****************************************
// timeout: number of seconds before timeout **********************************
// error: data containor for error (whether the communication timed out) ******
//*****************************************************************************
void UART_ReadByteArrayWithTimeout(uint8_t * byteArray, uint16_t size, 
	uint32_t timeout, uint8_t * error)
{
	uint32_t t1, t2, total;
	t1 = GetTime();
	int index = 0;
#if DEBUG
			printf("Receiving Data With Timeout:\n", rec[0]);
#endif
	while (size)
	{
		if (read(fd, &rec, 1) > 0)
		{
#if DEBUG
			printf("0x%x\n", rec[0]);
#endif
			byteArray[index++] = rec[0];
			size--;
		}

		t2 = GetTime();
		total = t2 - t1;
		if (total > timeout)
		{
			*error = UART_Timeout;
#if DEBUG
			printf("Receive Timeout\n");
#endif
			return;
		}

	}
#if DEBUG
			printf("Done Receiving Data\n");
#endif
}

//*****************************************************************************
// Flush the serial port if there is anything in the buffer *******************
//*****************************************************************************
void UART_Flush()
{
	uint8_t uartError = UART_Successful;
	uint8_t flushBuffer [10];
	while (uartError == UART_Successful)
	{
		UART_ReadByteArrayWithTimeout(flushBuffer, 10, 
			1, &uartError);
	}
}

//*****************************************************************************
// Close the serial port and end the UART communication ***********************
//*****************************************************************************
void UART_Close()
{
	close(fd);
}

//*****************************************************************************
// Set the serial port line and UART control pin high   ***********************
//*****************************************************************************
void UART_SetPinHigh(int pin)
{
   int flag = pin;
   ioctl(fd, TIOCMBIC, &flag);
}

//*****************************************************************************
// Set the serial port line and UART control pin low    ***********************
//*****************************************************************************
void UART_SetPinLow(int pin)
{
   int flag = pin;
   ioctl(fd, TIOCMBIS, &flag);
}
