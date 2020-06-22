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

OS_EVENT     OS_EVENT_tbl[OS_EVENT_TBL_SIZE];
OS_EVENT     *pEventFreeList;
OS_EVENT     **pEvents;

extern void OS_SetReady(OS_PRIO prio);
extern void OS_RemoveReady(OS_PRIO prio);


/*
 * Function:  OS_EventTaskWait
 * --------------------
 * Initialize the wait list.
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
    CPU_t16U i;

    for(i = 0; i < (OS_EVENT_TBL_SIZE - 1U);i++)
    {
        OS_EVENT_tbl[i].nextEventPtr    =   &OS_EVENT_tbl[i+1];
        OS_EVENT_tbl[i].eventType       =   OS_EVENT_TYPE_UNUSED;
        OS_EVENT_tbl[i].TCBPtr          =   ((OS_TASK_TCB*)0U);
    }

    OS_EVENT_tbl[OS_EVENT_TBL_SIZE - 1U].nextEventPtr    =   ((OS_EVENT*)0U);
    OS_EVENT_tbl[OS_EVENT_TBL_SIZE - 1U].eventType       =   OS_EVENT_TYPE_UNUSED;
    OS_EVENT_tbl[OS_EVENT_TBL_SIZE - 1U].TCBPtr          =   ((OS_TASK_TCB*)0U);

    pEventFreeList = &OS_EVENT_tbl[0];                                              /* Make it point to the first element.                                  */
    pEvents        = ((OS_EVENT**)0U);
}

/*
 * Function:  OS_EVENT_GetBlock
 * --------------------
 * Get a free OS_EVENT object.
 *
 * Arguments    :   pevent   is a pointer to the allocated OS_EVENT object.
 *
 * Returns      :  `pevent` as a form of return by reference.
 *
 * Notes        :   1) This function for internal use.
 */
void inline
OS_EVENT_GetBlock(OS_EVENT* pevent)
{
    if(((OS_EVENT*)0U) == pEventFreeList)
    {
        pevent = ((OS_EVENT*)0U);
        return;
    }

    pevent = pEventFreeList;
    pEventFreeList = pEventFreeList->nextEventPtr;
}

/*
 * Function:  OS_EVENT_ReturnBlock
 * --------------------
 * Return an allocated OS_EVENT object to the free list of OS_EVENT objects.
 *
 * Arguments    : pevent   is a pointer to a previous the allocated OS_EVENT object.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 *                  2) `pevent` become an invalid pointer after this call.
 */
void inline
OS_EVENT_ReturnBlock(OS_EVENT* pevent)
{
    pevent->eventType       = OS_EVENT_TYPE_UNUSED;
    pevent->TCBPtr          =  ((OS_TASK_TCB*)0U);
    pevent->nextEventPtr    = pEventFreeList;
    pEventFreeList          = pevent;
    pevent                  = ((OS_EVENT*)0U);
}

/*
 * Function:  OS_EventTaskWait
 * --------------------
 * Remove a task pointed by `pevent` from the ready list and place in a wait list.
 *
 * Arguments    : pevent   is a pointer to an OS_EVENT object for which the task will be waiting for.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 */
void
OS_EventTaskWait(OS_EVENT* pevent)
{
    OS_EVENT** pEventsHead  = pEvents;
    OS_PRIO prio            = pevent->TCBPtr->TASK_priority;

    OS_RemoveReady(prio);                                                       /* Remove task from the ready list                                              */

                                                                                /* Place the task in the wait list                                              */
    if(pEvents == ((OS_EVENT**)0U))                                             /* First waited event to insert ?                                               */
    {
        *pEvents = pevent;
        (*pEvents)->nextEventPtr = ((OS_EVENT*)0U);
    }
    else
    {
        if(prio > (*pEvents)->TCBPtr->TASK_priority)                            /* Insert the highest priority waited task at beginning                         */
        {
            pevent->nextEventPtr = *pEvents;
            *pEvents = pevent;
        }
        else if((*pEvents)->nextEventPtr == ((OS_EVENT*)0U))                    /* Keep the highest priority waited task at beginning                           */
        {
            (*pEvents)->nextEventPtr = pevent;
            pevent->nextEventPtr = ((OS_EVENT*)0U);
        }
        else
        {
            while((*pEvents)->nextEventPtr
                    && prio < (*pEvents)->nextEventPtr->TCBPtr->TASK_priority)   /* Walk-Through the list until the next event has a less priority than `prio` */
            {
                *pEvents = (*pEvents)->nextEventPtr;
            }

            if((*pEvents)->nextEventPtr)                                         /* Between two events                                                         */
            {
                pevent->nextEventPtr = (*pEvents)->nextEventPtr;
                (*pEvents)->nextEventPtr = pevent;
            }
            else                                                                /* Insert at the tail                                                          */
            {
                (*pEvents)->nextEventPtr = pevent;
                pevent->nextEventPtr = ((OS_EVENT*)0U);
            }
            pEvents = pEventsHead;                                              /* Reset pEvents position                                                       */
        }
    }
}


