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

#define OS_TICKS_PER_SEC     (100U)        /* Number of ticks per second.                            */

#define OS_MAX_NUMBER_TASKS  (128U)        /* Number of used tasks  (Must be multiple of 8).         */

#define OS_MAX_EVENTS        (3U)          /* Number of used events                                  */

/*
*******************************************************************************
*                             Data Types Sizes                                *
*******************************************************************************
*/
typedef CPU_t16U        OS_SEM_COUNT;       /* Define max number of semaphore count limit. */


#endif /* __PRETTY_CONFIG_H_ */
