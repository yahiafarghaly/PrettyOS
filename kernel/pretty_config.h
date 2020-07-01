/*****************************************************************************
MIT License

Copyright (c) 2020 Yahia Farghaly Ashour

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef __PRETTY_CONFIG_H_
#define __PRETTY_CONFIG_H_

/*
*******************************************************************************
*                       OS Miscellaneous Configurations                       *
*******************************************************************************
*/

#define OS_MAX_PRIO_ENTRIES  (3U)           /* Number of priority entries (levels), minimum value = 1 */

#define OS_MAX_EVENTS        (3U)          /* Number of used events                                  */

/*
*******************************************************************************
*                             Data Types Sizes                                *
*******************************************************************************
*/

typedef CPU_tWORD       OS_tRet;            /* Fit to the easiest type of memory for CPU   */

typedef CPU_t32U        OS_TICK;            /* Clock tick counter                          */

typedef CPU_t08U        OS_PRIO;            /* Max task priority can hold                  */

typedef CPU_t08U        OS_STATUS;          /* Task status                                 */

typedef CPU_t16U        OS_SEM_COUNT;       /* Semaphore value                             */


#endif /* __PRETTY_CONFIG_H_ */
