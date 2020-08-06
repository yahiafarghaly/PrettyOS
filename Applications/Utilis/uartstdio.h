//*****************************************************************************
//
// uartstdio.h - Prototypes for the UART console functions.
//
// Copyright (c) 2007-2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 9453 of the Stellaris Firmware Development Package.
//
// Modified: Yahia Farghaly<yahiafarghaly@gmail.com>
// Modification Date: 06/06/2020
//*****************************************************************************
#ifndef UARTSTDIO_H_
#define UARTSTDIO_H_

#include "../bsp.h"

/* Acts like standard C printf() except it doesn't support float printing. */
extern void UARTprintf(const char *pcString, ...);

#if !(defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(_WIN32))
	#define printf(...) UARTprintf(__VA_ARGS__)
#else
	#define printf(...) do { BSP_Write_to_Console(__VA_ARGS__);}while(0)
#endif	/* An Operating System not a microcontroller.	*/

#endif /* UARTSTDIO_H_ */
