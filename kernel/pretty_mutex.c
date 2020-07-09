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

#include "pretty_os.h"


/*
*******************************************************************************
*                                    Externs                                  *
*******************************************************************************
*/

extern OS_TASK_TCB* volatile OS_currentTask;
extern CPU_t08U     OS_IntNestingLvl;
extern CPU_t08U     OS_LockSchedNesting;

extern void OS_EVENT_allocate (OS_EVENT** pevent);
extern void OS_EVENT_free (OS_EVENT* pevent);
extern void OS_Event_TaskPend (OS_EVENT* pevent);
extern void OS_Event_TaskRemove (OS_TASK_TCB* ptcb, OS_EVENT *pevent);
extern void OS_Sched (void);
extern void OS_BlockTime (OS_PRIO prio);
extern void OS_UnBlockTime (OS_PRIO prio);
extern OS_PRIO OS_Event_TaskMakeReady(OS_EVENT* pevent,void* pmsg, OS_STATUS TASK_StatEventMask, OS_STATUS TASK_PendStat);

extern OS_ERR OS_ERRNO;


/*
 * Function:  OS_MutexCreate
 * --------------------
 * Creates a mutual exclusion semaphore.
 *
 * Arguments    :   prio    is the priority to use when accessing the mutual exclusion semaphore.
 *                          In other words, when the mutex is acquired and a higher priority task attempts
 *                          to obtain the mutex, then the priority of the task owning the mutex is rasied to
 *                          this priority to solve the a potential problem (inversion priority) cased when this solution is not provided.
 *
 *                          It's assumed that you will specify a priority that HIGHER than ANY of the tasks competing for the mutex.
 *                          This term is usually called "Priority Ceiling Priority/Promotion/Protocol" or "PCP" for short.
 *
 *                  opt     Enable/Disable the Priority Ceiling protocol.
 *                          = OS_MUTEX_PRIO_CEIL_DISABLE    (Default)
 *                          = OS_MUTEX_PRIO_CEIL_ENABLE
 *
 * Returns      :  != (OS_EVENT*)0U  is a pointer to OS_EVENT object of type OS_EVENT_TYPE_MUTEX for the created mutex.
 *                 == (OS_EVENT*)0U  if error is found.
 *
 * Note(s)      :   1) This function is used only from Task code level.
 *                  2) 'OSMutexPrio' is the priority task that owning the mutex or OS_PRIO_RESERVED_MUTEX if no task is owning the mutex.
 *                     'OSMutexPrioCeilP' is the raised priority to reduce the priority inversion.
 *                  3) OSEventType is equal to (3U) when equals to (OS_EVENT_TYPE_MUTEX + OS_MUTEX_PRIO_CEIL_ENABLE),
 *                     So, If it's only OS_EVENT_TYPE_MUTEX, it means that priority ceiling promotion is disabled.
 */
OS_EVENT*
OS_MutexCreate (OS_PRIO prio, OS_OPT opt)
{


}
