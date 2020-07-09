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

/*
*******************************************************************************
*                                Semaphore functions                          *
*******************************************************************************
*/

/*
 * Function:  OS_SemCreate
 * --------------------
 * Creates a semaphore.
 *
 * Arguments    : cnt    is the initial value for the semaphore.  If the value is 0, no resource is
 *                       available (or no event has occurred).
 *                       You initialize the semaphore to a non-zero value to specify how many
 *                       resources are available.
 *
 * Returns      : An 'OS_EVENT' object pointer of type semaphore (OS_EVENT_TYPE_SEM) to be used with
 *                 other semaphore functions.
 *
 * Notes        :   1) This function must used only from Task code level and not an ISR.
 */
OS_EVENT*
OS_SemCreate (OS_SEM_COUNT cnt)
{
    OS_EVENT* pevent = (OS_EVENT*)0U;

    if(OS_IntNestingLvl > 0U)                           /* Create only from task level.                      */
    {
        return ((OS_EVENT *)0U);
    }

    OS_CRTICAL_BEGIN();
    OS_EVENT_allocate(&pevent);                         /* Allocate an event object.                         */
    OS_CRTICAL_END();

    if(pevent == ((OS_EVENT*)0U))
    {
        return (pevent);
    }

    pevent->OSEventType     = OS_EVENT_TYPE_SEM;
    pevent->OSEventPtr      = ((OS_EVENT*)0U);         /* Unlink from the free list of queue events.         */
    pevent->OSEventCount    = cnt;
    pevent->OSEventsTCBHead = ((OS_TASK_TCB*)0U);      /* Initialize that no tasks waiting on this semaphore.*/

    return (pevent);
}

/*
 * Function:  OS_SemPend
 * --------------------
 * Waits for a semaphore.
 *
 * Arguments    :   pevent      is a pointer to the OS_EVENT object associated with the semaphore.
 *
 *                  timeout     is an optional timeout period (in clock ticks).  If non-zero, your task will
 *                              wait for the resource up to the amount of time specified by this argument.
 *                              If you specify 0, however, your task will wait forever at the specified
 *                              semaphore or, until the resource becomes available (or the event occurs).
 *
 * Returns      :   OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_PEND_ISR, OS_ERR_EVENT_PEND_LOCKED
 *                  OS_ERR_EVENT_PEND_ABORT, OS_STAT_PEND_TIMEOUT and OS_RET_OK
 *
 * Notes        :   1) This function must used only from Task code level and not an ISR.
 */
OS_tRet
OS_SemPend (OS_EVENT* pevent, OS_TICK timeout)
{
    OS_tRet ret;

    if (pevent == (OS_EVENT*)0U) {                          /* Validate 'pevent'                                         */
         return (OS_ERR_EVENT_PEVENT_NULL);
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {         /* Validate event type                                       */
        return (OS_ERR_EVENT_TYPE);
    }

    if (OS_IntNestingLvl > 0U) {
        return (OS_ERR_EVENT_PEND_ISR);                     /* Doesn't make sense to wait inside an ISR.                 */
    }

    if (OS_LockSchedNesting > 0U) {
        return (OS_ERR_EVENT_PEND_LOCKED);                  /* Should not wait when scheduler is locked.                 */
    }

    OS_CRTICAL_BEGIN();

    if(pevent->OSEventCount > 0U)                           /* If semaphore resource is available ...                    */
    {
        (pevent->OSEventCount)--;                           /* ... decrement it ...                                      */
        OS_CRTICAL_END();
        return (OS_ERR_NONE);                                 /* ... and return.                                           */
    }

    OS_currentTask->TASK_Stat |= OS_TASK_STATE_PEND_SEM;    /* Otherwise, pend on semaphore and wait for event to occur. */
    OS_currentTask->TASK_PendStat = OS_STAT_PEND_OK;
    OS_currentTask->TASK_Ticks = timeout;
    if(timeout > 0U)
    {
        OS_BlockTime(OS_currentTask->TASK_priority);
        OS_currentTask->TASK_Stat |= OS_TASK_STAT_DELAY;
    }

    OS_Event_TaskPend(pevent);                              /* Place the current TCB in the pending list.                */

    OS_CRTICAL_END();

    OS_Sched();                                             /* Preempt another task.                                     */

    OS_CRTICAL_BEGIN();                                     /* We're back again ...                                      */

    switch (OS_currentTask->TASK_PendStat) {                /* ... See if it was timed-out or aborted.                   */
        case OS_STAT_PEND_OK:
             ret  = OS_ERR_NONE;                              /* Indicate that the task owns the resource.                 */
             break;

        case OS_STAT_PEND_ABORT:
             ret = OS_ERR_EVENT_PEND_ABORT;                 /* Indicate that we aborted.                                 */
             break;

        case OS_STAT_PEND_TIMEOUT:
        default:
            OS_Event_TaskRemove(OS_currentTask, pevent);
             ret = OS_ERR_EVENT_TIMEOUT;                    /* Indicate that we didn't get event within Time out.        */
             break;
    }

    OS_currentTask->TASK_Stat     &= ~(OS_TASK_STATE_PEND_SEM);
    OS_currentTask->TASK_PendStat  =  OS_STAT_PEND_OK;
    OS_currentTask->OSEventPtr     = ((OS_EVENT*)0U);       /* Unlink the event from the current TCB.                    */

    OS_CRTICAL_END();

    return (ret);
}

/*
 * Function:  OS_SemPost
 * --------------------
 * Signal a semaphore.
 *
 * Arguments    :   pevent      is a pointer to the OS_EVENT object associated with the semaphore.
 *
 * Returns      :   OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_RET_OK
 *
 * Notes        :   1) This function can be called from a task code or an ISR.
 */
OS_tRet
OS_SemPost (OS_EVENT* pevent)
{
    if (pevent == (OS_EVENT*)0U) {                          /* Validate 'pevent'                                         */
         return (OS_ERR_EVENT_PEVENT_NULL);
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {         /* Validate event type                                       */
        return (OS_ERR_EVENT_TYPE);
    }

    OS_CRTICAL_BEGIN();

    if (pevent->OSEventsTCBHead != ((OS_TASK_TCB*)0U)) {    /* See if any task waiting for semaphore.                    */
        OS_Event_TaskMakeReady(pevent, (void *)0,           /* Make Highest priority task waiting on event be ready.     */
                            OS_TASK_STATE_PEND_SEM,
                            OS_STAT_PEND_OK);               /* OS_STAT_PEND_OK indicates a post operation.               */
        OS_CRTICAL_END();
        /* We don't need to increment the semaphore count here, since it's emulated by the design as this task is
         * preempted to the highest priority waiting task which takes the resource (decrement it again).
         * Also, this prevent other tasks (preempted to others than the one who waiting for the event) from
         * owning the resource.
         * On the other side, the pend() function is not performing any decrement operation if it is pended. */
        OS_Sched();                                         /* Call the scheduler, it may be the highest.                */
        return (OS_ERR_NONE);
    }

    (pevent->OSEventCount)++;                               /*TODO: Set a way of checking semaphore overflow             */

    OS_CRTICAL_END();

    return (OS_ERR_NONE);
}

/*
 * Function:  OS_SemPendNonBlocking
 * --------------------
 * Check if the resource is available or not. If it's available, the caller will get a resource.
 * If not available, the caller is not suspended unlike OS_SemPend().
 *
 * Arguments    :   pevent      is a pointer to the OS_EVENT object associated with the semaphore.
 *
 * Returns      :   > 0         If the resource is available or the event didn't occur. As a result, the semaphore is decremented to obtain the resource.
 *                  == 0        If the resource is not available or the event didn't occur.
 *                              Or `pevent` is not a valid pointer or it's not a semaphore type.
 *
 * Note(s)      :   1) This function can be called from a task code or an ISR.
 *              :   2) It's not recommended to be used within an ISR. An ISR is not supposed to obtain a semaphore.
 *                     A good practice is to post a semaphore from an ISR.
 */
OS_SEM_COUNT
OS_SemPendNonBlocking(OS_EVENT* pevent)
{
    OS_SEM_COUNT count;

    if (pevent == (OS_EVENT*)0U) {                          /* Validate 'pevent'                                            */
         return (0U);
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {         /* Validate event type                                          */
        return (0U);
    }

    OS_CRTICAL_BEGIN();

    count = pevent->OSEventCount;                           /* The available count.                                         */
    if(count > 0U)
    {
        (pevent->OSEventCount)--;                           /* The calling Task/ISR owns a semaphore resource.              */
    }

    OS_CRTICAL_END();

    return (count);
}

/*
 * Function:  OS_SemPendAbort
 * --------------------
 * Aborts and makes ready for tasks that wait for a semaphore event.
 * This function should not be treated as posting semaphore values unlike
 * It lets waiting tasks to not wait any more for resource availability.
 *
 * Arguments    :   pevent              is a pointer to the OS_EVENT object associated with the semaphore.
 *
 *                  opt                 Determine the abort operation.
 *                                      OS_SEM_ABORT_HPT (Default behavior) ==> Abort only higher priority waiting task.
 *                                      OS_SEM_ABORT_ALL                    ==> Abort all waiting priority tasks.
 *
 *                 abortedTasksCount    is pointer to an object to hold the number of aborted waited tasks.
 *
 * Returns      :   OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_EVENT_PEND_ABORT, OS_RET_OK
 *
 * Note(s)      :   1) This function can be called from a task code or an ISR.
 */
OS_tRet
OS_SemPendAbort(OS_EVENT* pevent, CPU_t08U opt, OS_TASK_COUNT* abortedTasksCount)
{
    OS_TASK_COUNT nAbortedTasks;

    if (pevent == (OS_EVENT*)0U) {                          /* Validate 'pevent'                                         */
         return (OS_ERR_EVENT_PEVENT_NULL);
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {         /* Validate event type                                       */
        return (OS_ERR_EVENT_TYPE);
    }

    OS_CRTICAL_BEGIN();

    if (pevent->OSEventsTCBHead != ((OS_TASK_TCB*)0U)) {    /* See if any task waiting for semaphore.                    */
        nAbortedTasks = 0U;
        switch(opt)
        {
        case OS_SEM_ABORT_ALL:
            while(pevent->OSEventsTCBHead != ((OS_TASK_TCB*)0U))
            {
                OS_Event_TaskMakeReady(pevent, (void *)0,
                                    OS_TASK_STATE_PEND_SEM,
                                    OS_STAT_PEND_ABORT);    /* OS_STAT_PEND_ABORT indicates abort operation.               */
                ++nAbortedTasks;
            }
            break;
        case OS_OPT_DEFAULT & OS_SEM_ABORT_HPT:
        default:
            OS_Event_TaskMakeReady(pevent, (void *)0,
                                OS_TASK_STATE_PEND_SEM,
                                OS_STAT_PEND_ABORT);
            ++nAbortedTasks;
            break;
        }

        OS_CRTICAL_END();

        OS_Sched();                                         /* Preempt HPT that no longer wait for an event ?              */

        if(abortedTasksCount != ((OS_TASK_COUNT*)0U))
        {
            *abortedTasksCount = nAbortedTasks;
        }
        return (OS_ERR_EVENT_PEND_ABORT);                   /* Indicate that we aborted.                                    */
    }

    OS_CRTICAL_END();

    if(abortedTasksCount != ((OS_TASK_COUNT*)0U))          /* No waiting tasks to abort.                                    */
    {
        *abortedTasksCount = 0U;
    }

    return (OS_ERR_NONE);
}
