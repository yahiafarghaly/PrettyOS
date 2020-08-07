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

#if(OS_AUTO_CONFIG_INCLUDE_EVENTS == OS_CONFIG_ENABLE)

#if OS_CONFIG_MAX_EVENTS < 1U
	#error  "OS_CONFIG_MAX_EVENTS must be >= 1"
#endif

/*
*******************************************************************************
*                               Global Variables                              *
*******************************************************************************
*/
OS_EVENT  OSEventsMemoryPool[OS_CONFIG_MAX_EVENTS];
OS_EVENT* volatile pEventFreeList;

/*
 * Function:  OS_Event_FreeListInit
 * --------------------
 * Initialize the memory pool of a free list of available events.
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

    for(i = 0; i < (OS_CONFIG_MAX_EVENTS - 1U);i++)
    {
        OSEventsMemoryPool[i].OSEventPtr      = &OSEventsMemoryPool[i+1];
        OSEventsMemoryPool[i].OSEventType     = OS_EVENT_TYPE_UNUSED;
        OSEventsMemoryPool[i].OSEventsTCBHead = ((OS_TASK_TCB*)0U);
    }

    OSEventsMemoryPool[OS_CONFIG_MAX_EVENTS - 1U].OSEventPtr       = ((OS_EVENT*)0U);
    OSEventsMemoryPool[OS_CONFIG_MAX_EVENTS - 1U].OSEventType      = OS_EVENT_TYPE_UNUSED;
    OSEventsMemoryPool[OS_CONFIG_MAX_EVENTS - 1U].OSEventsTCBHead  = ((OS_TASK_TCB*)0U);

    pEventFreeList = &OSEventsMemoryPool[0];
}

/*
 * Function:  OS_EVENT_allocate
 * --------------------
 * Get an allocated OS_EVENT object.
 *
 * Arguments    :   pevent   is a pointer to a pointer to the allocated OS_EVENT object.
 *
 * Returns      :   => `pevent` as a form of return by reference ( pointer to the allocated object).
 *                  => ((OS_EVENT*)0U) in case there is no more event blocks available.
 *
 * Notes        :   1) This function for internal use.
 */
void
OS_EVENT_allocate(OS_EVENT** pevent)
{
    if(((OS_EVENT*)0U) == pEventFreeList)
    {
        *pevent = ((OS_EVENT*)0U);
        return;
    }

    *pevent = pEventFreeList;
    pEventFreeList = pEventFreeList->OSEventPtr;    /* Go to the next free event object.    */
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
    pevent->OSEventsTCBHead = ((OS_TASK_TCB*)0U);
    pevent->OSEventCount    = (0U);
    pevent->OSEventPtr      = pEventFreeList;       /* Return to the event free pool.       */
    pEventFreeList          = pevent;
    pevent                  = ((OS_EVENT*)0U);
}

/*
 * Function:  OS_Event_TaskInsert
 * --------------------
 * Insert a task to an event's wait list according to its priority.
 * The function works by placing the TCBs that wait for a certain event (pointed by `pevent`)
 * in a sorted linked-list in descending order of TCBs' priority.
 *
 * Arguments    : ptcb    is a pointer to TCB object where `pevent` will be stored into
 *                pevent  is a pointer to an allocated OS_EVENT object.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 *                  2) Interrupts must be disabled at this call.
 */
void
OS_Event_TaskInsert(OS_TASK_TCB* ptcb, OS_EVENT *pevent)
{
    OS_PRIO prio;
    OS_TASK_TCB* currentTCBPtr;

    ptcb->TASK_Event = pevent;                                      	/* Store the event pointer inside the current TCB.              */
    prio          	 = ptcb->TASK_priority;

    currentTCBPtr = pevent->OSEventsTCBHead;

    if(currentTCBPtr == ((OS_TASK_TCB*)0U))
    {
        ptcb->OSTCB_NextPtr = ((OS_TASK_TCB*)0U);                        /* Place at the head.                                           */
        pevent->OSEventsTCBHead  = ptcb;
    }
    else if(currentTCBPtr->TASK_priority <= prio)
    {
        ptcb->OSTCB_NextPtr = currentTCBPtr;
        pevent->OSEventsTCBHead  = ptcb;
    }
    else
    {
        while (currentTCBPtr->OSTCB_NextPtr != ((OS_TASK_TCB*)0U)        /* Walk-Through the list to place the TCB in the correct order. */
                && currentTCBPtr->OSTCB_NextPtr->TASK_priority > prio)
        {
            currentTCBPtr = currentTCBPtr->OSTCB_NextPtr;
        }
        ptcb->OSTCB_NextPtr = currentTCBPtr->OSTCB_NextPtr;
        currentTCBPtr->OSTCB_NextPtr = ptcb;
    }
}

/*
 * Function:  OS_Event_TaskPend
 * --------------------
 * Insert the current running TCB into a wait list and remove it from the ready list.
 *
 * Arguments    : pevent   is a pointer to an allocated OS_EVENT object.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 *                  2) This function should be called by the pend functions(e.g, semaphore,.. etc)
 *                  3) Interrupts must be disabled at this call.
 */
void
OS_Event_TaskPend (OS_EVENT *pevent)
{
    OS_Event_TaskInsert(OS_currentTask, pevent);     /* Insert the current running test into the waiting list        */
    OS_RemoveReady(OS_currentTask->TASK_priority);   /* Remove from the ready list.                                  */
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
 *                  2) Interrupts must be disabled at this call.
 */
void
OS_Event_TaskRemove (OS_TASK_TCB* ptcb, OS_EVENT *pevent)
{
    OS_PRIO  prio;
    OS_TASK_TCB* currentTCBPtr;

    prio = ptcb->TASK_priority;
    currentTCBPtr = pevent->OSEventsTCBHead;

    if (currentTCBPtr == OS_NULL(OS_TASK_TCB))                						/* Is an empty pended TCB list                       							*/
    {
        return;
    }
    else																			/* No ...																		*/
    {
        if(prio == currentTCBPtr->TASK_priority)									/* Is the head TCB priority equals to the TCB priority we desire to remove ?	*/
        {
            pevent->OSEventsTCBHead = pevent->OSEventsTCBHead->OSTCB_NextPtr;		/* Yes ... Move the head to the next TCB.										*/
            currentTCBPtr->OSTCB_NextPtr = OS_NULL(OS_TASK_TCB);					/* Clear the next of the previous head.											*/
        }
        else
        {
			while (currentTCBPtr->OSTCB_NextPtr != OS_NULL(OS_TASK_TCB)
					&& currentTCBPtr->OSTCB_NextPtr->TASK_priority != prio)			/* Loop to the end of the list or we find the desired priority to remove.		*/
			{
				currentTCBPtr = currentTCBPtr->OSTCB_NextPtr;
			}
			currentTCBPtr->OSTCB_NextPtr = currentTCBPtr->OSTCB_NextPtr->OSTCB_NextPtr;
        }
    }

    ptcb->TASK_Event = OS_NULL(OS_EVENT);                   						/* Disconnect the event from the given TCB.        								 */
}

/*
 * Function:  OS_Event_TaskMakeReady
 * --------------------
 * Make a task that was waiting for an event to occur be ready.
 *
 * Arguments    : pevent                is a pointer to an allocated OS_EVENT object.
 *                pmsg                  is a pointer to a message which is used by mailboxes.
 *                TASK_StatEventMask    is a mask that is used to clear the TASK_Stat member of TCB structure of the
 *                                      called post event function. For example, OS_SemPost() will pass OS_TASK_STATE_PEND_SEM.
 *
 *                TASK_PendStat         is used indicate the pend status of the task which was waiting for an event.
 *                                      OS_STAT_PEND_OK     => Task ready due to a post (or delete event object).
 *                                      OS_STAT_PEND_ABORT  => Task ready due to an abort.
 *
 * Returns      : The highest priority of the ready task that was waiting for an event.
 *
 *
 * Notes        :   1) This function for internal use.
 *                  2) This function should be called by the post functions(e.g, semaphore,.. etc)
 *                  3) Interrupts must be disabled at this call.
 */
OS_PRIO
OS_Event_TaskMakeReady(OS_EVENT* pevent,void* pmsg,
                       OS_STATUS TASK_StatEventMask,
                       OS_STATUS TASK_PendStat)
{
    OS_TASK_TCB* pHighTCB;

    pHighTCB = pevent->OSEventsTCBHead;                     /* Highest Priority Task waiting for an event.                      */

    pHighTCB->TASK_Ticks = 0U;                              /* The task is not waiting for event anymore So, let                */
    OS_UnBlockTime(pHighTCB->TASK_priority);                /* make sure that OS_TimerTick will not try to make it ready.       */

#if (OS_CONFIG_MAILBOX_EN == OS_CONFIG_ENABLE)

    pevent->OSEventPtr	= (OS_EVENT*) pmsg;					/* Send the message to the waiting task.							*/

#else

    (void)pmsg;

#endif

    pHighTCB->TASK_Stat &= ~(TASK_StatEventMask);           /* Clear the event type bit.                                        */
    pHighTCB->TASK_PendStat = TASK_PendStat;                /* pend status due to a post or abort operation.                    */
    if((pHighTCB->TASK_Stat & OS_TASK_STAT_SUSPENDED)       /* Make task ready if it's not suspended.                           */
            == OS_TASK_STAT_READY)
    {
        OS_SetReady(pHighTCB->TASK_priority);
    }

    OS_Event_TaskRemove(pHighTCB,pevent);                   /* Remove TCB from the wait list.                                   */

    return (pHighTCB->TASK_priority);                       /* Return ready task priority                                       */
}


#endif			/* OS_AUTO_CONFIG_INCLUDE_EVENTS */
