/*
 * pretty_event.c
 *
 *  Created on: Jun 21, 2020
 *      Author: yf
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"

/*
*******************************************************************************
*                               Global Variables                              *
*******************************************************************************
*/
OS_EVENT     OSEventsMemoryPool[OS_MAX_EVENTS];
OS_EVENT     *pEventFreeList;

/*
*******************************************************************************
*                                    Externs                                  *
*******************************************************************************
*/

extern OS_TASK_TCB* OS_currentTask;

extern void OS_SetReady(OS_PRIO prio);
extern void OS_RemoveReady(OS_PRIO prio);

/*
 * Function:  OS_Event_FreeListInit
 * --------------------
 * Initialize the pool memory of the free list of available events.
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 */
void
OS_Event_FreeListInit(void)
{
    CPU_t32U i;

    for(i = 0; i < (OS_MAX_EVENTS - 1U);i++)
    {
        OSEventsMemoryPool[i].OSEventPtr  = &OSEventsMemoryPool[i+1];
        OSEventsMemoryPool[i].OSEventType = OS_EVENT_TYPE_UNUSED;
        OSEventsMemoryPool[i].OSEventTCBs = ((OS_TASK_TCB**)0U);
    }

    OSEventsMemoryPool[OS_MAX_EVENTS - 1U].OSEventPtr  = ((OS_EVENT*)0U);
    OSEventsMemoryPool[OS_MAX_EVENTS - 1U].OSEventType = OS_EVENT_TYPE_UNUSED;
    OSEventsMemoryPool[OS_MAX_EVENTS - 1U].OSEventTCBs = ((OS_TASK_TCB**)0U);

    pEventFreeList = &OSEventsMemoryPool[0];
}

/*
 * Function:  OS_EVENT_allocate
 * --------------------
 * Get an allocated OS_EVENT object.
 *
 * Arguments    :   pevent   is a pointer to the allocated OS_EVENT object.
 *
 * Returns      :  `pevent` as a form of return by reference ( pointer to the allocated object).
 *                  ((OS_EVENT*)0U) in case there is no event blocks available.
 *
 * Notes        :   1) This function for internal use.
 */
void
OS_EVENT_allocate(OS_EVENT* pevent)
{
    if(((OS_EVENT*)0U) == pEventFreeList)
    {
        pevent = ((OS_EVENT*)0U);
        return;
    }

    pevent = pEventFreeList;
    pEventFreeList = pEventFreeList->OSEventPtr;
}

/*
 * Function:  OS_EVENT_free
 * --------------------
 * Return an allocated OS_EVENT object to the free list of OS_EVENT objects.
 *
 * Arguments    : pevent   is a pointer to a previous allocated OS_EVENT object.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 *                  2) `pevent` become an invalid pointer after this call.
 */
void
OS_EVENT_free(OS_EVENT* pevent)
{
    pevent->OSEventType     = OS_EVENT_TYPE_UNUSED;
    pevent->OSEventTCBs     = ((OS_TASK_TCB**)0U);
    pevent->OSEventCount    = (0U);
    pevent->OSEventPtr      = pEventFreeList;
    pEventFreeList          = pevent;
    pevent                  = ((OS_EVENT*)0U);
}


/*
 * Function:  OS_Event_TaskPend
 * --------------------
 * Insert the current running TCB into a wait list and remove it from the ready list.
 * The function is working by placing the TCBs that wait for a certain event (pointed by `pevent`)
 * in a sorted linked-list in increasing order of TCBs' priority.
 *
 * Arguments    : pevent   is a pointer to an allocated OS_EVENT object.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 *                  2) This function should be called by the pend functions(e.g, semaphore,.. etc)
 */
void
OS_Event_TaskPend (OS_EVENT *pevent)
{
    OS_PRIO prio;
    OS_TASK_TCB* currentTCBPtr;

    OS_currentTask->OSEventPtr = pevent;                            /* Store the event pointer inside the current TCB.              */
    prio          = OS_currentTask->TASK_priority;

    currentTCBPtr = *(pevent->OSEventTCBs);

    if (currentTCBPtr == ((OS_TASK_TCB*)0U)
            || currentTCBPtr->TASK_priority <= prio)
    {
        OS_currentTask->OSTCBPtr = currentTCBPtr;                   /* Place at the head.                                           */
        currentTCBPtr            = OS_currentTask;                  /* Reset (pevent->OSEventTCBs) location.                        */
    }
    else
    {
        while (currentTCBPtr->OSTCBPtr != ((OS_TASK_TCB*)0U)        /* Walk-Through the list to place the TCB in the correct order. */
                && currentTCBPtr->OSTCBPtr->TASK_priority > prio)
        {
            currentTCBPtr = currentTCBPtr->OSTCBPtr;
        }
        OS_currentTask->OSTCBPtr = currentTCBPtr->OSTCBPtr;
        currentTCBPtr->OSTCBPtr = OS_currentTask;
    }

    OS_RemoveReady(prio);                                           /* Remove from the ready list.                                  */
}

/*
 * Function:  OS_Event_TaskRemove
 * --------------------
 * Remove a task from an event's wait list.
 *
 * Arguments    : ptcb    is a pointer to TCB object where it has event pointed by `pevent`
 *                pevent  is a pointer to an allocated OS_EVENT object.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 */
void
OS_Event_TaskRemove (OS_TASK_TCB* ptcb, OS_EVENT *pevent)
{
    OS_PRIO  prio;
    OS_TASK_TCB* currentTCBPtr;

    prio          = ptcb->TASK_priority;
    currentTCBPtr = *(pevent->OSEventTCBs);

    if (currentTCBPtr == ((OS_TASK_TCB*)0U))                /* Empty List                       */
    {
        return;
    }
    else if(currentTCBPtr->OSTCBPtr == ((OS_TASK_TCB*)0U))  /* One element in the list          */
    {
        if(prio == currentTCBPtr->TASK_priority)
        {
            *(pevent->OSEventTCBs) = ((OS_TASK_TCB*)0U);
        }
        else
        {
            return;
        }
    }
    else
    {
        while (currentTCBPtr->OSTCBPtr != ((OS_TASK_TCB*)0U)
                && currentTCBPtr->OSTCBPtr->TASK_priority != prio)
        {
            currentTCBPtr = currentTCBPtr->OSTCBPtr;
        }
        currentTCBPtr->OSTCBPtr = currentTCBPtr->OSTCBPtr->OSTCBPtr;
    }

    ptcb->OSEventPtr = ((OS_EVENT*)0U);                      /* Unlink OS_EVENT from TCB         */
}

