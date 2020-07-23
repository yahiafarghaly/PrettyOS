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
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"
#include "pretty_shared.h"

/*
*******************************************************************************
*                                Mutex functions                              *
*******************************************************************************
*/

/*
 * Function:  OS_MutexCreate
 * --------------------
 * Creates a mutual exclusion semaphore.
 *
 * Arguments    :   prio    is the priority to use when accessing the mutual exclusion semaphore.
 *                          In other words, when the mutex is acquired and a higher priority task attempts
 *                          to obtain the mutex, then the priority of the task owning the mutex is rasied to
 *                          this priority to solve the potential problem (inversion priority) cased when this solution is not provided.
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
    OS_EVENT*    pevent;

    if(!OS_IS_VALID_PRIO(prio))                                     /* Valid priority ?                                       */
    {
        OS_ERR_SET(OS_ERR_PRIO_INVALID);
        return ((OS_EVENT*)0U);
    }

    if(OS_IS_RESERVED_PRIO(prio))                                  /* Check that OS is not owning it.                          */
    {
        OS_ERR_SET(OS_ERR_PRIO_EXIST);
        return ((OS_EVENT*)0U);
    }

    if (OS_IntNestingLvl > 0U) {                                   /* Don't Create from an ISR.                                */
        OS_ERR_SET(OS_ERR_EVENT_CREATE_ISR);
        return ((OS_EVENT*)0U);
    }

    OS_CRTICAL_BEGIN();

    if(opt == OS_MUTEX_PRIO_CEIL_ENABLE)
    {
        if(OS_tblTCBPrio[prio] != OS_NULL(OS_TASK_TCB))            /* Mutex priority must be available to use.                 */
        {
            OS_ERR_SET(OS_ERR_PRIO_EXIST);                         /* TCB entry is reserved for this priority ...              */
            OS_CRTICAL_END();                                      /* ... which cannot be used as priority ceiling.            */
            return ((OS_EVENT*)0U);
        }

        OS_tblTCBPrio[prio] = OS_TCB_MUTEX_RESERVED;               /* Reserve This TCB entry for Mutex use.                     */
    }

    OS_EVENT_allocate(&pevent);                                    /* Allocate a free event object.                             */
    if(pevent == ((OS_EVENT*)0U))
    {
        OS_tblTCBPrio[prio] = OS_NULL(OS_TASK_TCB);                /* No more free event objects, Release the TCB entry.        */
        OS_ERR_SET(OS_ERR_EVENT_POOL_EMPTY);
        OS_CRTICAL_END();
        return (pevent);
    }

    OS_CRTICAL_END();

    pevent->OSEventType      = OS_EVENT_TYPE_MUTEX;                /* Store the Event type.                                     */
    pevent->OSEventPtr       = ((OS_EVENT*)0U);                    /* Initial, Not related to any tasks.                        */
    pevent->OSEventsTCBHead  = ((OS_TASK_TCB*)0U);                 /* Initial, No tasks are pended on this event.               */
    pevent->OSMutexPrio      = OS_PRIO_RESERVED_MUTEX;             /* Initial, No task is owning the Mutex.                     */

    if(OS_MUTEX_PRIO_CEIL_DISABLE == opt)
    {
        pevent->OSMutexPrioCeilP = OS_PRIO_RESERVED_MUTEX;         /* OS_PRIO_RESERVED_MUTEX to indicate a PCP is disabled.     */
    }
    else
    {
        pevent->OSMutexPrioCeilP = prio;                           /* Store PCP value.                                          */
    }

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
    OS_PRIO         pcp;                                    /* Priority Ceiling Priority                                 */
    OS_PRIO         owner_prio;
    OS_TASK_TCB*    ptcb_owner;
    OS_EVENT*       pevent_owner;
    CPU_t08U        ready;                                  /* Flag to indicate that the task was ready.                 */

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

    pcp = pevent->OSMutexPrioCeilP;                         /* Get PCP value.                                            */

    if(pevent->OSMutexPrio == OS_PRIO_RESERVED_MUTEX)       /* Is Mutex available for the calling task ?                 */
    {
        pevent->OSMutexPrio = OS_currentTask->TASK_priority;/* Save task priority which owning the mutex.                */
        pevent->OSEventPtr  = (OS_EVENT*)OS_currentTask;    /* Point to the owning task TCB.                             */

        if((pcp != OS_PRIO_RESERVED_MUTEX) &&               /* Is priority ceiling is enabled.                           */
                (pcp < OS_currentTask->TASK_priority))      /* PCP should be higher than the Mutex owner task.           */
        {
            OS_CRTICAL_END();
            OS_ERR_SET(OS_ERR_MUTEX_LOWER_PCP);             /* indicate that PCP should not be lower than owner priority.*/
        }
        else
        {
            OS_CRTICAL_END();
            OS_ERR_SET(OS_ERR_NONE);
        }

        return;                                             /* We are done here, the current task is owning the Mutex.  */
    }
                                                            /* The Mutex is owned by another task.                      */
                                                            
    if(pcp != OS_PRIO_RESERVED_MUTEX)                       /* Is priority ceiling is enabled ?                         */
    {
        owner_prio = pevent->OSMutexPrio;                   /* Priority of task owning the Mutex.                       */
        ptcb_owner  = (OS_TASK_TCB*)pevent->OSEventPtr;     /* TCB entry of task owning the Mutex.                      */

        if(owner_prio < pcp)                                /* No need to ceil if owner priority is higher than PCP.    */
        {
            if(owner_prio < OS_currentTask->TASK_priority)  /* ... neither if owner is higher than the current task.    */
            {
                if(ptcb_owner->TASK_Stat == OS_TASK_STAT_READY)
                {
                    OS_RemoveReady(ptcb_owner->TASK_priority);
                    ready = OS_TRUE;
                }
                else
                {
                    if(ptcb_owner->TASK_Stat & OS_TASK_STAT_DELAY)/* If it waits any delay..                            */
                    {
                        OS_UnBlockTime(ptcb_owner->TASK_priority);/* ... Unblock it from delay table.                   */
                    }

                    pevent_owner = ptcb_owner->OSEventPtr;
                    if(pevent_owner != ((OS_EVENT*)0U))           /* If it waits any events..                           */
                    {
                        OS_Event_TaskRemove(ptcb_owner, pevent_owner); /* ... Remove from event list.                   */
                    }
                    ready = OS_FAlSE;
                }

                ptcb_owner->TASK_priority   = pcp;          /* Change owner task priority to PCP value.                 */

                /* 'ready' Flag is necessary here, since if owner has events at its own priority, These events
                 * should moved properly in the new priority (i.e PCP).                                                 */

                if(ready == OS_TRUE)
                {
                    OS_SetReady(pcp);
                }
                else
                {
                    if(ptcb_owner->TASK_Stat & OS_TASK_STAT_DELAY)/* If it was waiting any delay..                      */
                    {
                        OS_BlockTime(pcp);                        /* ... block it at PCP priority.                      */
                    }

                    if(pevent_owner != ((OS_EVENT*)0U))
                    {
                        OS_Event_TaskInsert(ptcb_owner, pevent_owner);/* ... Add to event list.                         */
                    }
                }

                OS_tblTCBPrio[pcp]  = ptcb_owner;               /* Point to the TCB entry of PCP priority.              */

                /* Continue to pend the current task and hopefully, the PCP's Task will be scheduled first.             */
            }
        }
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
            OS_ERR_SET(OS_ERR_NONE);                        /* Indicate that the current task owns the Mutex.            */
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
 * Returns      :   OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_EVENT_POST_ISR,
 *                               OS_ERR_MUTEX_PCP_LOWER }
 *
 * Notes        :   1) This function must used only from Task code level.
 */
void
OS_MutexPost (OS_EVENT* pevent)
{
    OS_PRIO pcp;
    OS_PRIO owner_prio;
    OS_PRIO new_owner_prio;
    OS_TASK_TCB* ptcb_owner;

    if (OS_IntNestingLvl > 0U) {
        OS_ERR_SET(OS_ERR_EVENT_POST_ISR);                                  /* Doesn't make sense to post inside an ISR.                */
        return;
    }

    if (pevent == (OS_EVENT*)0U) {                                          /* Validate 'pevent'                                         */
        OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
        return;
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_MUTEX) {                       /* Validate event type                                       */
        OS_ERR_SET(OS_ERR_EVENT_TYPE);
        return;
    }

    OS_CRTICAL_BEGIN();

    pcp        = pevent->OSMutexPrioCeilP;
    owner_prio = pevent->OSMutexPrio;
    ptcb_owner  = (OS_TASK_TCB*)pevent->OSEventPtr;

    if(OS_currentTask != ptcb_owner)                                        /* Check that the poster is the owner of the Mutex.          */
    {
        OS_CRTICAL_END();
        OS_ERR_SET(OS_ERR_MUTEX_NO_OWNER);
        return;
    }

    if(pcp != OS_PRIO_RESERVED_MUTEX)                                       /* Is priority ceiling is enabled.                           */
    {
        if(OS_currentTask->TASK_priority == pcp)                            /* Is it's raised to PCP ?                                   */
        {                                                                   /* Restore the task's original priority.                     */
            /* At this point,
             * We're in a task, raised to PCP, So it's in a ready/runnable state and not pending on any other events or time delay.      */
            OS_RemoveReady(pcp);                                            /* Remove owner from ready state at PCP priority.            */

            OS_currentTask->TASK_priority   = owner_prio;                   /* Revert to the original priority.                          */

            OS_SetReady(owner_prio);                                        /* Set the owner to a ready state at the original priority.  */

            OS_tblTCBPrio[owner_prio]       = ptcb_owner;                   /* Reset the original priority to refers to original TCB.    */
            OS_tblTCBPrio[pcp]              = OS_TCB_MUTEX_RESERVED;        /* Reserve TCB entry again for a future Mutex use.           */

            /* After that: the HPT task, pending on this Mutex will be scheduled.                                                        */
        }
    }

    if (pevent->OSEventsTCBHead != ((OS_TASK_TCB*)0U))                      /* See if any task waiting for Mutex.                        */
    {
        new_owner_prio = OS_Event_TaskMakeReady(pevent, (void *)0,          /* Make Highest priority task waiting on event be ready.     */
                            OS_TASK_STATE_PEND_MUTEX,
                            OS_STAT_PEND_OK);                               /* OS_STAT_PEND_OK indicates a post operation.               */

        pevent->OSMutexPrio = new_owner_prio;                               /* Save task priority which owning the mutex.                */
        pevent->OSEventPtr  = (OS_EVENT*)OS_tblTCBPrio[new_owner_prio];     /* Point to the new owning task TCB.                         */

        if((pcp != OS_PRIO_RESERVED_MUTEX) && (pcp < new_owner_prio))       /* Is priority ceiling is enabled                            */
        {
            OS_CRTICAL_END();
            OS_Sched();
            OS_ERR_SET(OS_ERR_MUTEX_LOWER_PCP);                             /* Indicate that PCP should not be lower than owner priority.*/
        }
        else
        {
            OS_CRTICAL_END();
            OS_Sched();
            OS_ERR_SET(OS_ERR_NONE);
        }

        return;                                                             /* We are done here, the waited HPT task is owning the Mutex. */
    }
                                                                            /* No task is owning the Mutex after post.                    */
    pevent->OSMutexPrio = OS_PRIO_RESERVED_MUTEX;
    pevent->OSEventPtr  = ((OS_EVENT*)0U);

    OS_CRTICAL_END();
    OS_ERR_SET(OS_ERR_NONE);
}







