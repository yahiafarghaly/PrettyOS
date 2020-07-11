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

extern OS_TASK_TCB* volatile        OS_currentTask;
extern CPU_t08U     volatile        OS_IntNestingLvl;
extern CPU_t08U     volatile        OS_LockSchedNesting;
extern OS_TASK_TCB*  const          OS_TblTaskPtr;

extern void OS_EVENT_allocate (OS_EVENT** pevent);
extern void OS_EVENT_free (OS_EVENT* pevent);
extern void OS_Event_TaskPend (OS_EVENT* pevent);
extern void OS_Event_TaskRemove (OS_TASK_TCB* ptcb, OS_EVENT *pevent);
extern void OS_Sched (void);
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
 *                 OS_ERRNO = { OS_ERR_NONE, OS_ERR_PRIO_INVALID, OS_ERR_PRIO_EXIST, OS_ERR_EVENT_CREATE_ISR, OS_ERR_EVENT_POOL_EMPTY}
 *
 * Note(s)      :   1) This function is used only from Task code level.
 *                  2) 'OSMutexPrio'      of returned (OS_EVENT*)   is the original priority task that owning the Mutex or 'OS_PRIO_RESERVED_MUTEX' if no task is owning the Mutex.
 *                     'OSMutexPrioCeilP' of returned (OS_EVENT*)   is the raised priority to reduce the priority inversion or 'OS_PRIO_RESERVED_MUTEX' if priority ceiling promotion is disabled.
 */
OS_EVENT*
OS_MutexCreate (OS_PRIO prio, OS_OPT opt)
{

    OS_TASK_TCB* pTCB;
    OS_EVENT*    pevent;

    if(!OS_IS_VALID_PRIO(prio))                                      /* Valid priority ?                                        */
    {
        OS_ERR_SET(OS_ERR_PRIO_INVALID);
        return ((OS_EVENT*)0U);
    }

    if(OS_IS_RESERVED_PRIO(prio))                                   /* Check that OS is not owning it.                          */
    {
        OS_ERR_SET(OS_ERR_PRIO_EXIST);
        return ((OS_EVENT*)0U);
    }

    if (OS_IntNestingLvl > 0U) {                                    /* Don't Create from an ISR.                                */
        OS_ERR_SET(OS_ERR_EVENT_CREATE_ISR);
        return ((OS_EVENT*)0U);
    }

    OS_CRTICAL_BEGIN();

    pTCB = &OS_TblTaskPtr[prio];

    if(pTCB->TASK_Stat == OS_TASK_STAT_DELETED &&
            pTCB->TASK_Stat != OS_TASK_STAT_RESERVED_MUTEX)         /* Mutex priority must be available to use.                 */
    {
        OS_ERR_SET(OS_ERR_PRIO_EXIST);                              /* Task is reserving this priority already ...              */
        OS_CRTICAL_END();                                           /* ... which cannot be used as priority ceiling.            */
        return ((OS_EVENT*)0U);
    }

    pTCB->TASK_Stat = OS_TASK_STAT_RESERVED_MUTEX;                  /* Reserve This TCB entry for Mutex use.                    */

    OS_EVENT_allocate(&pevent);                                     /* Allocate a free event object.                            */
    if(pevent == ((OS_EVENT*)0U))
    {
        pTCB->TASK_Stat = OS_TASK_STAT_DELETED;                    /* No more free event objects, Release the TCB entry.        */
        OS_ERR_SET(OS_ERR_EVENT_POOL_EMPTY);
        OS_CRTICAL_END();
        return (pevent);
    }

    OS_CRTICAL_END();

    pevent->OSEventType      = OS_EVENT_TYPE_MUTEX;                /* Store the Event type.                                     */
    pevent->OSEventPtr       = ((OS_EVENT*)0U);                    /* Initial, Not related to any tasks.                        */
    pevent->OSEventsTCBHead  = ((OS_TASK_TCB*)0U);                 /* Initial, No tasks are pended on this event.               */
    pevent->OSMutexPrio      = OS_PRIO_RESERVED_MUTEX;             /* Initial, No task is owning the Mutex.                     */
                                                                   /* According to 'opt', Assign PCP or OS_PRIO_RESERVED_MUTEX to indicate a PCP is disabled. */
    pevent->OSMutexPrioCeilP = ((opt == OS_MUTEX_PRIO_CEIL_DISABLE) ? (OS_PRIO_RESERVED_MUTEX) : prio) ;

    OS_ERR_SET(OS_ERR_NONE);
    return (pevent);
}


/*
 * Function:  OS_MutexPend
 * --------------------
 * Waits for a mutual exclusion semaphore.
 *
 * Arguments    :   pevent      is a pointer to the OS_EVENT object associated with the Mutex.
 *
 *                  timeout     is an optional timeout period (in clock ticks).  If non-zero, your task will
 *                              wait for the resource up to the amount of time specified by this argument.
 *                              If you specify 0, however, your task will wait forever at the specified
 *                              mutex or, until the resource becomes available (or the event occurs).
 *
 * Returns      :   OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_EVENT_PEND_ISR,
 *                               OS_ERR_MUTEX_PCP_LOWER, OS_ERR_EVENT_PEND_ABORT, OS_ERR_EVENT_TIMEOUT, OS_ERR_EVENT_PEND_LOCKED }
 *
 * Note(s)      :   1) This function must used only from Task code level and not an ISR.
 *                  2) The task that owns the Mutex must not pend on any other events while it's owning the Mutex. Otherwise, you create a possible inversion priority bug.
 *                  3) [For the current implementation], Don't change the priority of the task that owns the Mutex at run time.
 */
void
OS_MutexPend (OS_EVENT* pevent, OS_TICK timeout)
{
    OS_PRIO PCP;                                            /* Priority Ceiling Priority                                 */

    if (pevent == (OS_EVENT*)0U) {                          /* Validate 'pevent'                                         */
        OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
        return;
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_MUTEX) {       /* Validate event type                                       */
        OS_ERR_SET(OS_ERR_EVENT_TYPE);
        return;
    }

    if (OS_IntNestingLvl > 0U) {
        OS_ERR_SET(OS_ERR_EVENT_PEND_ISR);                  /* Doesn't make sense to wait inside an ISR.                 */
        return;
    }

    if (OS_LockSchedNesting > 0U) {
        OS_ERR_SET(OS_ERR_EVENT_PEND_LOCKED);               /* Should not wait when scheduler is locked.                 */
        return;
    }

    OS_CRTICAL_BEGIN();

    PCP = pevent->OSMutexPrioCeilP;                         /* Get PCP value.                                            */

    if(pevent->OSMutexPrio == OS_PRIO_RESERVED_MUTEX)       /* Is Mutex available for the calling task ?                 */
    {
        pevent->OSMutexPrio = OS_currentTask->TASK_priority;/* Save task priority which owning the mutex.                */
        pevent->OSEventPtr  = (OS_EVENT*)OS_currentTask;    /* Point to the owning task.                                 */
        if(PCP != OS_PRIO_RESERVED_MUTEX)                   /* Is priority ceiling is enabled.                           */
        {
            if(PCP < OS_currentTask->TASK_priority)
            {
                OS_CRTICAL_END();
                OS_ERR_SET(OS_ERR_MUTEX_LOWER_PCP);
            }
        }
        else
        {
            OS_CRTICAL_END();
            OS_ERR_SET(OS_ERR_NONE);
        }
        return;
    }
                                                            /* The Mutex is owned by another task.                      */
    if(PCP != OS_PRIO_RESERVED_MUTEX)                       /* Is priority ceiling is enabled.                          */
    {
        /*TODO:Priority Ceiling protocol*/
    }

    OS_currentTask->TASK_Stat      |= OS_TASK_STATE_PEND_MUTEX;  /* Otherwise, pend on mutex.                           */
    OS_currentTask->TASK_PendStat   = OS_STAT_PEND_OK;
    OS_currentTask->TASK_Ticks      = timeout;
    if(timeout > 0U)
    {
        OS_BlockTime(OS_currentTask->TASK_priority);        /* Put in a time block state until mutex may be released.   */
        OS_currentTask->TASK_Stat |= OS_TASK_STAT_DELAY;
    }

    OS_Event_TaskPend(pevent);                              /* Place the current TCB in the pending list.                */

    OS_CRTICAL_END();

    OS_Sched();                                             /* Preempt another task.                                     */

    OS_CRTICAL_BEGIN();                                     /* We're back again ...                                      */

    switch (OS_currentTask->TASK_PendStat) {                /* ... See if it was timed-out or aborted.                   */
        case OS_STAT_PEND_OK:
            OS_ERR_SET(OS_ERR_NONE);                        /* Indicate that the current task owns the resource.         */
             break;

        case OS_STAT_PEND_ABORT:
            OS_ERR_SET(OS_ERR_EVENT_PEND_ABORT);            /* Indicate that we aborted before getting the mutex.        */
             break;

        case OS_STAT_PEND_TIMEOUT:
        default:
            OS_Event_TaskRemove(OS_currentTask, pevent);    /* Release the current task from pending on mutex.           */
            OS_ERR_SET(OS_ERR_EVENT_TIMEOUT);               /* Indicate that we didn't get the mutex within Time out.    */
             break;
    }

    OS_currentTask->TASK_Stat     &= ~(OS_TASK_STATE_PEND_MUTEX);
    OS_currentTask->TASK_PendStat  =  OS_STAT_PEND_OK;
    OS_currentTask->OSEventPtr     = ((OS_EVENT*)0U);       /* Unlink the event from the current TCB.                    */

    OS_CRTICAL_END();
}

/*
 * Function:  OS_MutexPost
 * --------------------
 * Signal a mutual exclusion semaphore.
 *
 * Arguments    :   pevent      is a pointer to the OS_EVENT object associated with the Mutex.
 *
 * Returns      :   OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_EVENT_PEND_ISR,
 *                               OS_ERR_MUTEX_PCP_LOWER, OS_ERR_EVENT_PEND_ABORT, OS_ERR_EVENT_TIMEOUT, OS_ERR_EVENT_PEND_LOCKED }
 *
 * Notes        :   1) This function must used only from Task code level.
 */
void
OS_MutexPost (OS_EVENT* pevent)
{

    if (OS_IntNestingLvl > 0U) {
        OS_ERR_SET(OS_ERR_EVENT_PEND_ISR);                  /* Doesn't make sense to post inside an ISR.                 */
        return;
    }

    if (pevent == (OS_EVENT*)0U) {                          /* Validate 'pevent'                                         */
        OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
        return;
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_MUTEX) {       /* Validate event type                                       */
        OS_ERR_SET(OS_ERR_EVENT_TYPE);
        return;
    }

    OS_CRTICAL_BEGIN();

    /*TODO: */

    OS_CRTICAL_END();

    return (OS_ERR_NONE);
}







