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

/*
 * Author   : Yahia Farghaly Ashour
 *
 * Purpose  : A single header file to contain all external functions or variables which are commonly used by the prettyOS internal code.
 *            These variables or function APIs listed here should not be used or interacted with the application level code.
 *
 * Language:  C
 *
 * Set 1 tab = 4 spaces for better comments readability.
 */

#ifndef __PRETTY_SHARED_H_
#define __PRETTY_SHARED_H_

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"

/*
*******************************************************************************
*                               External variables                            *
*******************************************************************************
*/

extern CPU_tWORD    volatile        OS_Running;

extern OS_TASK_TCB* volatile        OS_currentTask;
extern OS_TASK_TCB*                 OS_tblTCBPrio [OS_CONFIG_TASK_COUNT];

extern CPU_t08U     volatile        OS_IntNestingLvl;
extern CPU_t08U     volatile        OS_LockSchedNesting;

#if (OS_CONFIG_SYSTEM_TIME_SET_GET_EN == OS_CONFIG_ENABLE)
extern OS_TICK		volatile		OS_TickTime;
#endif

extern OS_ERR                       OS_ERRNO;


/*
*******************************************************************************
*                               External functions                            *
*******************************************************************************
*/

extern void OS_Sched (void);
extern void OS_MemoryByteClear (CPU_t08U* pdest, CPU_t32U size);

extern void OS_Event_FreeListInit (void);

extern void OS_EVENT_allocate   (OS_EVENT** pevent);
extern void OS_EVENT_free       (OS_EVENT* pevent);

extern void OS_Event_TaskInsert (OS_TASK_TCB* ptcb, OS_EVENT *pevent);
extern void OS_Event_TaskRemove (OS_TASK_TCB* ptcb, OS_EVENT *pevent);
extern void OS_Event_TaskPend   (OS_EVENT* pevent);
extern OS_PRIO OS_Event_TaskMakeReady(OS_EVENT* pevent,
                                      void* pmsg,
                                      OS_STATUS TASK_StatEventMask,
                                      OS_STATUS TASK_PendStat);

extern void OS_SetReady    (OS_PRIO prio);
extern void OS_RemoveReady (OS_PRIO prio);

extern void OS_BlockTime   (OS_PRIO prio);
extern void OS_UnBlockTime (OS_PRIO prio);

extern void OS_TCB_ListInit (void);

extern void OS_Memory_Init (void);

#endif /* __PRETTY_SHARED_H_ */
