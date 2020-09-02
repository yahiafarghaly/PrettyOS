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
 * Author   : 	Yahia Farghaly Ashour
 *
 * Purpose  :	Event Flag Implementation.
 *

                                        +-----------------------------------------------------------+
                                        |                                                           |
                                        |     +-----------------+                                   |
                                        |     |                 |                                   |
                                        |     |                 |                                   |
                                        |     |                 |                                   |
                                        |     |                 |                                   |
                                        v     v               OS|EVENT_FLAG_NODE                 OS_EVENT_FLAG_NODE
 OS_EVENT_FLAG_GRP   +------------------+-----+-----+         +----------------+                 +----------------+
+-----------------+  |         OSEventType          |         | |.pFlagGroup   |                 |  |.pFlagGroup  |
                     |                              |         +----------------+                 +----------------+
                     +------------------------------+         |                |                 |                |
                     |         OSFlagBits           |         +----------------+                 +----------------+
                     |          00000101			|         |                +-------------->  |.pFlagNodeNext  +------------>  NULL
                     +------------------------------+         +----------------+                 +----------------+
                     |        pFlagNodeHead         |         |                |                 |                |
                     |                              +-------> |                |                 |                |
                     +------------------------------+         |                |                 |                |
                                                              +----------------+                 +----------------+
                                                              |		00001010   |                 |		00000000  | .OSFlagBits
                                                              +----------------+                 +----------------+
                                                              |                |                 |                |
                                                              |         +      |                 |         +      | .pTCBFlagNode
                                                              +----------------+                 +----------------+
                                                                        |                                  |
                                                                        |                                  |
                                                                        |                                  |
                                                                        |                                  |
                                                                        |                                  |
                                                                        |                                  |
                                                              +-----------------+                +-----------------+
                                                              |   |     |       |                |   |     |       |
                                                              |   |     |       |                |   |     |       |
                                                              |  +++    v       |                |  +++    v       | .pEventFlagNode
                                                              |                 |                |                 |
                                                              |                 |                |                 |
                                                              |                 |                |                 |
                                                              |                 |                |                 |
                                                              |                 |                |                 |
                                                              +-----------------+                +-----------------+
                                                                    OS_TASK_TCB                      OS_TASK_TCB



 *
 * Language	:  C
 *
 * Set 1 tab = 4 spaces for better comments readability.
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"
#include "pretty_shared.h"

#if (OS_CONFIG_FLAG_EN == OS_CONFIG_ENABLE)

#if OS_CONFIG_MAX_EVENT_FLAGS < 1U
	#error  "OS_CONFIG_MAX_EVENT_FLAGS must be >= 1"
#endif

/*
*******************************************************************************
*                               Local Variables                               *
*******************************************************************************
*/

OS_EVENT_FLAG_GRP OSFlagGroupMemoryPool [OS_CONFIG_MAX_EVENT_FLAGS];
OS_EVENT_FLAG_GRP* volatile pFlagGroupFreeList;

/*
*******************************************************************************
*                               Local Functions                               *
*******************************************************************************
*/

/* Fast allocation of OS_EVENT_FLAG_GRP object.								  	*/
static inline OS_EVENT_FLAG_GRP* OS_EventFlagGroup_allocate (void)
{
	OS_EVENT_FLAG_GRP* peventFlagGrp;
	peventFlagGrp = pFlagGroupFreeList;
	if(pFlagGroupFreeList != OS_NULL(OS_EVENT_FLAG_GRP))
	{
		pFlagGroupFreeList = (OS_EVENT_FLAG_GRP*)pFlagGroupFreeList->pFlagNodeHead;	/* Move to the next free object. */
	}
	return (peventFlagGrp);
}

/* Fast deallocation of OS_EVENT_FLAG_GRP object.							   	*/
static inline void OS_EventFlagGroup_deallocate (OS_EVENT_FLAG_GRP* peventFlagGrp)
{
	peventFlagGrp->pFlagNodeHead 		= (OS_EVENT_FLAG_NODE*)pFlagGroupFreeList;
	pFlagGroupFreeList 					= peventFlagGrp;
	pFlagGroupFreeList->OSEventType 	= OS_EVENT_TYPE_UNUSED;
	pFlagGroupFreeList->OSFlagCurrent	= 0U;
}

/*
 * Pend the current running task + Setup the node member variables.				*/
static inline void OS_EventFlag_PendCurrentTask(OS_EVENT_FLAG_GRP* pflagGrp, OS_EVENT_FLAG_NODE* pflagNode,
												OS_FLAG flags_pattern_wait, OS_FLAG_WAIT wait_type,
												OS_TICK timeout)
{
    OS_currentTask->TASK_Stat |= OS_TASK_STATE_PEND_FLAG;
    OS_currentTask->TASK_PendStat = OS_STAT_PEND_OK;
    OS_currentTask->TASK_Ticks = timeout;

    if(timeout > 0U)
    {
        OS_BlockTime(OS_currentTask->TASK_priority);		/* Add time delay block.								*/
        OS_currentTask->TASK_Stat |= OS_TASK_STAT_DELAY;
    }

    pflagNode->OSFlagWaited 	= flags_pattern_wait;		/* Save the flags we're waiting for.					*/
    pflagNode->OSFlagWaitType 	= wait_type;				/* Save the type of wait.								*/
    pflagNode->pTCBFlagNode		= OS_currentTask;			/* Link to task's TCB.									*/
    pflagNode->pFlagGroup		= pflagGrp;					/* Link to the parent event flag group.					*/
    pflagNode->pFlagNodeNext	= pflagGrp->pFlagNodeHead;	/* Always insert node at the beginning of the list.		*/
    pflagGrp->pFlagNodeHead		= pflagNode;				/* Reset the head to the new node.						*/

    OS_RemoveReady(OS_currentTask->TASK_priority);			/* Finally, Remove task's TCB from the ready state.		*/
}

/*
 * Remove a node event from a list of node events.	(Assume A valid event node pointer)								*/
static void OS_EventFlag_UnlinkFlagNodeFromList(OS_EVENT_FLAG_NODE* pflagNode)
{
	OS_EVENT_FLAG_NODE* pflagWalker;
	OS_EVENT_FLAG_NODE* pflagSlower;

	pflagWalker = pflagNode->pFlagGroup->pFlagNodeHead;		/* Get the head flag node.								*/

	if(pflagWalker == pflagNode)							/* Is the node in the head of the wait list ?			*/
	{														/* Yes, update the new head.							*/
		pflagNode->pFlagGroup->pFlagNodeHead 	= pflagNode->pFlagGroup->pFlagNodeHead->pFlagNodeNext;
		pflagNode->pFlagNodeNext				= OS_NULL(OS_EVENT_FLAG_NODE);
	}
															/* No, Search linearly in the wait list.				*/
	while(pflagWalker->pFlagNodeNext != OS_NULL(OS_EVENT_FLAG_NODE))
	{
		pflagSlower = pflagWalker;							/* A slower node than the walker.						*/
		pflagWalker = pflagWalker->pFlagNodeNext;			/* Goto the next node.									*/
		if(pflagWalker == pflagNode)						/* is it matching the node ? 							*/
		{
			pflagSlower->pFlagNodeNext = pflagWalker->pFlagNodeNext;
			pflagWalker->pFlagNodeNext = OS_NULL(OS_EVENT_FLAG_NODE);
			break;
		}
	}
															/* Unlink the node from TCB.							*/
	pflagNode->pTCBFlagNode		= OS_NULL(OS_TASK_TCB);
}

static OS_BOOLEAN OS_EventFlag_MakeTaskReady (OS_EVENT_FLAG_NODE *pnode,
                                                OS_FLAG flags_ready,
                                                OS_STATUS TASK_StatEventMask,
                                                OS_STATUS TASK_PendStat)
{
    OS_TASK_TCB  *ptcb;
    OS_BOOLEAN sched;

    ptcb                    = pnode->pTCBFlagNode;          /* Point to TCB of waiting task                                     */
    ptcb->TASK_Stat        &= ~(TASK_StatEventMask);        /* Clear the event type bit.                                        */
    ptcb->TASK_PendStat     = TASK_PendStat;                /* pend status due to a post or abort operation.                    */

    ptcb->TASK_Ticks        = 0u;
    OS_UnBlockTime(ptcb->TASK_priority);

    ptcb->OSFlagReady       = flags_ready;                  /* Store the flags which caused in a ready state.                   */

    if((ptcb->TASK_Stat & OS_TASK_STAT_SUSPENDED)           /* Make task ready if it's not suspended.                           */
            == OS_TASK_STAT_READY)
    {
        OS_SetReady(ptcb->TASK_priority);
        sched = OS_TRUE;
    }
    else
    {
    	sched = OS_FAlSE;
    }

    OS_EventFlag_UnlinkFlagNodeFromList(pnode);             /* Unlink it from wait list.                                        */

    return (sched);
}

/*
*******************************************************************************
*                               Global Functions                              *
*******************************************************************************
*/

/* Initialize the memory pool of the free list of OS_EVENT_FLAG_GRP objects.  */
void OS_Event_Flag_FreeListInit(void)
{
    CPU_t32U i;

    for(i = 0; i < (OS_CONFIG_MAX_EVENT_FLAGS - 1U);i++)
    {
    	OSFlagGroupMemoryPool[i].pFlagNodeHead 	= (OS_EVENT_FLAG_NODE*)&OSFlagGroupMemoryPool[i+1];
    	OSFlagGroupMemoryPool[i].OSEventType   	= OS_EVENT_TYPE_UNUSED;
    	OSFlagGroupMemoryPool[i].OSFlagCurrent 	= 0U;
    }

    OSFlagGroupMemoryPool[OS_CONFIG_MAX_EVENT_FLAGS - 1U].pFlagNodeHead 	= OS_NULL(OS_EVENT_FLAG_NODE);
    OSFlagGroupMemoryPool[OS_CONFIG_MAX_EVENT_FLAGS - 1U].OSEventType  		= OS_EVENT_TYPE_UNUSED;
    OSFlagGroupMemoryPool[OS_CONFIG_MAX_EVENT_FLAGS - 1U].OSFlagCurrent  	= 0U;

    pFlagGroupFreeList = &OSFlagGroupMemoryPool[0];
}

/*
 * Function:  OS_EVENT_FlagCreate
 * ------------------------------
 * Creates an event flag group.
 *
 * Arguments    : initial_flags    is the initial value for the event flags (i.e bits).
 *
 * Returns      :  != (OS_EVENT_FLAG_GRP*)0U  is a pointer to an event flag group.
 *                 == (OS_EVENT_FLAG_GRP*)0U  if no more event flag group is available.
 *
 *                 OS_ERRNO = { OS_ERR_NONE, OS_ERR_FLAG_GRP_POOL_EMPTY, OS_ERR_EVENT_CREATE_ISR }
 *
 * Notes        :   1) This function is called only from a task level code.
 */
OS_EVENT_FLAG_GRP*
OS_EVENT_FlagCreate (OS_FLAG initial_flags)
{
	OS_EVENT_FLAG_GRP* pflagGrp;
	CPU_SR_ALLOC();

    if(OS_IntNestingLvl > 0U)                           	/* Create only from task level code.	                	*/
    {
        OS_ERR_SET(OS_ERR_EVENT_CREATE_ISR);
        return OS_NULL(OS_EVENT_FLAG_GRP);
    }

    OS_CRTICAL_BEGIN();

    pflagGrp = OS_EventFlagGroup_allocate();
    if(pflagGrp == OS_NULL(OS_EVENT_FLAG_GRP))
    {
    	OS_CRTICAL_END();
    	OS_ERR_SET(OS_ERR_FLAG_GRP_POOL_EMPTY);
    	return OS_NULL(OS_EVENT_FLAG_GRP);
    }

    pflagGrp->OSFlagCurrent	= initial_flags;				/* Set the desired set of initial bits for this flag group.	*/
    pflagGrp->OSEventType	= OS_EVENT_TYPE_FLAG;			/* Setup the right type of the event.						*/
    pflagGrp->pFlagNodeHead	= OS_NULL(OS_EVENT_FLAG_NODE);	/* Initially, No waiting tasks on this event.				*/

    OS_CRTICAL_END();
    OS_ERR_SET(OS_ERR_NONE);
    return (pflagGrp);
}

/*
 * Function:  OS_EVENT_FlagPend
 * ------------------------------
 * Wait for a combination of bits (i.e flags) to be happened. Whether these combinations are SET of ANY/ALL bits or
 * CLEAR of ANY/ALL bits.
 *
 * Arguments    :	pflagGrp				is a pointer to the desired event flag group.
 *
 * 					flags_pattern_wait		is the pattern of bits (i.e flags) positions which the function will wait for according to
 * 											wait type. If the desired bits to wait for is bit no.1 and bit no.2	then 'flags_pattern_wait'
 * 											is 0x06 (000110).
 *
 * 					wait_type				is the type of waiting for the bits pattern :
 *
 *											OS_FLAG_WAIT_CLEAR_ALL	:	waits for ALL bits in the 'flags_pattern_wait' position to be Cleared. (i.e become 0).
 *											OS_FLAG_WAIT_CLEAR_ANY	:	waits for ANY bits in the 'flags_pattern_wait' position to be Cleared. (i.e become 0).
 *											OS_FLAG_WAIT_SET_ALL	:	waits for ALL bits in the 'flags_pattern_wait' position to be Set. 	   (i.e become 1).
 *											OS_FLAG_WAIT_SET_ANY	:	waits for ANY bits in the 'flags_pattern_wait' position to be Set. 	   (i.e become 1).
 *
 *					reset_flags_on_exit		If it's set to OS_TRUE, then any bit defined in the 'flags_pattern_wait' will be reset
 *											in the event flag group to the value before posting the event.
 *
 *											If it's set to OS_FALSE, then non of bits in the ' flags_pattern_wait' will be altered when the call returns.
 *
 *
 * 					timeout					is an optional timeout period (in clock ticks).  If non-zero, your task will wait for the event to the amount of
 * 											time specified in the argument. If it's zero, it will wait forever till the event occurred.
 *
 * Returns      :	The flag(s) (i.e bit(s)) which caused the event flag group to be triggered and meet the the desired event flag.
 * 					or (0) in case of timeout or the event is aborted.
 *
 *                 OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEND_ISR, OS_ERR_EVENT_PEND_LOCKED, OS_ERR_FLAG_PGROUP_NULL,
 *                 				OS_ERR_FLAG_WAIT_TYPE, OS_ERR_EVENT_PEND_ABORT, OS_ERR_EVENT_TIMEOUT, OS_ERR_EVENT_TYPE }
 *
 * Notes        :   1) This function is called only from a task level code.
 */
OS_FLAG
OS_EVENT_FlagPend (OS_EVENT_FLAG_GRP* pflagGrp, OS_FLAG flags_pattern_wait, OS_FLAG_WAIT wait_type, OS_BOOLEAN reset_flags_on_exit, OS_TICK timeout)
{
	OS_EVENT_FLAG_NODE	flag_node;							/* Allocate the event node on the task's stack.				*/
	OS_FLAG				flags_pattern_ready;
	OS_BOOLEAN			pend_ok;
	CPU_SR_ALLOC();

    if (OS_IntNestingLvl > 0U) {
        OS_ERR_SET(OS_ERR_EVENT_PEND_ISR);                 	/* Doesn't make sense to wait inside an ISR.                */
        return ((OS_FLAG)0U);
    }

    if (OS_LockSchedNesting > 0U) {
    	OS_ERR_SET(OS_ERR_EVENT_PEND_LOCKED);              	/* Should not wait when scheduler is locked.                */
    	return ((OS_FLAG)0U);
    }

	if(pflagGrp == OS_NULL(OS_EVENT_FLAG_GRP))				/* Validate Event Group Type Pointer.						*/
	{
		OS_ERR_SET(OS_ERR_FLAG_PGROUP_NULL);
		return ((OS_FLAG)0U);
	}

    if (pflagGrp->OSEventType != OS_EVENT_TYPE_FLAG) {      /* Validate event type (First Byte of any Event type)       */
        OS_ERR_SET(OS_ERR_EVENT_TYPE);
		return ((OS_FLAG)0U);
    }

    OS_CRTICAL_BEGIN();


    														/* Check the wait type and pend ...							*/
    switch(wait_type)
    {

    case OS_FLAG_WAIT_CLEAR_ALL:

    	flags_pattern_ready = (OS_FLAG)(~pflagGrp->OSFlagCurrent & flags_pattern_wait);
        if(flags_pattern_ready == flags_pattern_wait)		/* Are flags matching to be ready ?							*/
        {
        	goto RESET_FLAGS_ON_EXIT;						/* Yes, Reset flags if necessary and return.				*/
        }

    	break;

    case OS_FLAG_WAIT_CLEAR_ANY:

    	flags_pattern_ready = (OS_FLAG)(~pflagGrp->OSFlagCurrent & flags_pattern_wait);
        if(flags_pattern_ready != (OS_FLAG)0U)				/* Are flags matching to be ready ?							*/
        {
        	goto RESET_FLAGS_ON_EXIT;						/* Yes, Reset flags if necessary and return.				*/
        }

    	break;

    case OS_FLAG_WAIT_SET_ALL:
    														/* Check which flags are ready. 							*/
    	flags_pattern_ready = (OS_FLAG)(pflagGrp->OSFlagCurrent & flags_pattern_wait);
    	if(flags_pattern_ready == flags_pattern_wait)		/* Are flags matching to be ready ?							*/
    	{
    		goto RESET_FLAGS_ON_EXIT;						/* Yes, Reset flags if necessary and return.				*/
    	}
    	break;

    case OS_FLAG_WAIT_SET_ANY:
    														/* Check which flags are ready. 							*/
		flags_pattern_ready = (OS_FLAG)(pflagGrp->OSFlagCurrent & flags_pattern_wait);
		if(flags_pattern_ready != (OS_FLAG)0U)				/* Are flags matching to be ready ?							*/
		{
			goto RESET_FLAGS_ON_EXIT;						/* Yes, Reset flags if necessary and return.				*/
		}
		break;

    default:
    	OS_CRTICAL_END();
		OS_ERR_SET(OS_ERR_FLAG_WAIT_TYPE);
		return ((OS_FLAG)0U);
    }
    														/* Pend the current task till event is occurred or timeout.	*/
    OS_EventFlag_PendCurrentTask(pflagGrp,&flag_node,flags_pattern_wait,wait_type,timeout);

    OS_CRTICAL_END();

    OS_Sched();												/* Preempt another HPT.										*/

    OS_CRTICAL_BEGIN();										/* We are back again ------------------------------------>  */

    pend_ok = OS_FAlSE;
    switch (OS_currentTask->TASK_PendStat) {                /* ... See if it was timed-out or aborted.                  */

        case OS_STAT_PEND_ABORT:
        	OS_ERR_SET(OS_ERR_EVENT_PEND_ABORT);            /* Indicate that we aborted.                                */
            pend_ok = OS_FAlSE;
        	break;

        case OS_STAT_PEND_TIMEOUT:
            OS_ERR_SET(OS_ERR_EVENT_TIMEOUT);				/* Indicate that we didn't get event within Time out.       */
            pend_ok = OS_FAlSE;
            break;

        case OS_STAT_PEND_OK:								/* Indicate that we get the desired flags event.			*/
        default:
        	pend_ok = OS_TRUE;
        	OS_ERR_SET(OS_ERR_NONE);
        	break;
    }
    														/* Clear Pending & Task' status bits.			 			*/
    OS_currentTask->TASK_Stat     &= ~(OS_TASK_STATE_PEND_FLAG);
    OS_currentTask->TASK_PendStat  =  OS_STAT_PEND_OK;

    if(pend_ok == OS_FAlSE)									/* Check if it's Okay ?										*/
    {														/* No, the event is aborted or timeout. So unlink the event.*/
    	OS_EventFlag_UnlinkFlagNodeFromList(&flag_node);	/* Unlink the node from the wait list. [O(n) time ]			*/
    	flags_pattern_ready = (OS_FLAG)0U;					/* Zeros returned flags since it wasn't ready.				*/
    }
    else													/* Yes, Event(s) has occurred.								*/
    {
    	flags_pattern_ready = OS_currentTask->OSFlagReady;	/* Get the ready flags which task was waiting for.			*/
    }

RESET_FLAGS_ON_EXIT:
    if(reset_flags_on_exit == OS_TRUE)
    {
        switch(wait_type)									/* Reset the flags to the opposite of what's posted.		*/
        {
        case OS_FLAG_WAIT_CLEAR_ALL:
        case OS_FLAG_WAIT_CLEAR_ANY:
        	pflagGrp->OSFlagCurrent |= (flags_pattern_ready);
        	break;

        case OS_FLAG_WAIT_SET_ALL:
        case OS_FLAG_WAIT_SET_ANY:
        	pflagGrp->OSFlagCurrent &= ~(flags_pattern_ready);
            break;
        default:
        	OS_CRTICAL_END();
    		OS_ERR_SET(OS_ERR_FLAG_WAIT_TYPE);
    		return ((OS_FLAG)0U);
        }
    }

	OS_CRTICAL_END();										/* < ------------------------------------------------------ */
	OS_ERR_SET(OS_ERR_NONE);
	return (flags_pattern_ready);							/* Return the flags which caused task to be ready.			*/
}

/*
 * Function:  OS_EVENT_FlagPost
 * ------------------------------
 * Post a combination of bits (i.e flags) to an event flag group. Whether these combinations are SET of ANY/ALL bits or
 * CLEAR of ANY/ALL bits.
 *
 * Arguments    :	pflagGrp				is a pointer to the desired event flag group.
 *
 * 					flags_pattern_wait		is the pattern of bits (i.e flags) positions which the function will POST according to
 * 											'flags_options' type.
 *
 * 					flags_options			OS_FLAG_SET		Set the bits in the positions of 'flags_pattern_wait'
 * 											OS_FLAG_CLEAR	Clear the bits in the positions of 'flags_pattern_wait'
 *
 *
 * Returns      :	The new value of the bits which are changed in the event flag group.
 *
 *                 OS_ERRNO = { OS_ERR_NONE, OS_ERR_FLAG_PGROUP_NULL, OS_ERR_FLAG_WAIT_TYPE, OS_ERR_EVENT_TYPE }
 *
 * Notes        :   1) This function is called from a task code or an ISR code.
 */
OS_FLAG
OS_EVENT_FlagPost (OS_EVENT_FLAG_GRP* pflagGrp, OS_FLAG flags_pattern_wait, OS_OPT flags_options)
{
    OS_FLAG 	flags_ready;
    OS_FLAG		flags_current;
    OS_BOOLEAN 	sched;
    OS_EVENT_FLAG_NODE* pEventFlagNode;
	CPU_SR_ALLOC();

	if(pflagGrp == OS_NULL(OS_EVENT_FLAG_GRP))				/* Validate Event Group Type Pointer.						        */
	{
		OS_ERR_SET(OS_ERR_FLAG_PGROUP_NULL);
		return ((OS_FLAG)0U);
	}

    if (pflagGrp->OSEventType != OS_EVENT_TYPE_FLAG) {      /* Validate event type (First Byte of any Event type)               */
        OS_ERR_SET(OS_ERR_EVENT_TYPE);
		return ((OS_FLAG)0U);
    }

    OS_CRTICAL_BEGIN();

    switch(flags_options)                                   /* Perform the desired operation on the event group flag.           */
    {
        case OS_FLAG_SET:
            pflagGrp->OSFlagCurrent 	|= (flags_pattern_wait);
        break;

        case OS_FLAG_CLEAR:
        	 pflagGrp->OSFlagCurrent 	&= ~(flags_pattern_wait);
        break;

        default:
            OS_CRTICAL_END();
            OS_ERR_SET(OS_ERR_FLAG_OPT_TYPE);
            return (OS_FLAG)0U;
        break;
    }

    pEventFlagNode  = pflagGrp->pFlagNodeHead;              /* Let's Check that for each event node, Has it met its event ?     */
    while(pEventFlagNode != OS_NULL(OS_EVENT_FLAG_NODE))
    {
        switch(pEventFlagNode->OSFlagWaitType)              /* Check event waiting type.                                        */
        {
            case OS_FLAG_WAIT_CLEAR_ALL:
            	 flags_ready = (pEventFlagNode->OSFlagWaited & ~(pflagGrp->OSFlagCurrent));
				 if(flags_ready == pEventFlagNode->OSFlagWaited)
				 {
					 sched = OS_EventFlag_MakeTaskReady(pEventFlagNode,flags_ready,OS_TASK_STATE_PEND_FLAG,OS_STAT_PEND_OK);
				 }
				 break;

            case OS_FLAG_WAIT_CLEAR_ANY:
            	 flags_ready = (pEventFlagNode->OSFlagWaited & ~(pflagGrp->OSFlagCurrent));
				 if(flags_ready != (OS_FLAG)0U)
				 {
					 sched = OS_EventFlag_MakeTaskReady(pEventFlagNode,flags_ready,OS_TASK_STATE_PEND_FLAG,OS_STAT_PEND_OK);
				 }
				 break;

            case OS_FLAG_WAIT_SET_ALL:
                flags_ready = (pEventFlagNode->OSFlagWaited  & (pflagGrp->OSFlagCurrent));
                if(flags_ready == pEventFlagNode->OSFlagWaited)
                {
                	sched = OS_EventFlag_MakeTaskReady(pEventFlagNode,flags_ready,OS_TASK_STATE_PEND_FLAG,OS_STAT_PEND_OK);
                }
                break;
            case OS_FLAG_WAIT_SET_ANY:

				flags_ready = (pEventFlagNode->OSFlagWaited  & (pflagGrp->OSFlagCurrent));
				if(flags_ready != (OS_FLAG)0U)
				{
					sched = OS_EventFlag_MakeTaskReady(pEventFlagNode,flags_ready,OS_TASK_STATE_PEND_FLAG,OS_STAT_PEND_OK);
				}
				break;

            default:
                OS_CRTICAL_END();
                OS_ERR_SET(OS_ERR_FLAG_WAIT_TYPE);
                return ((OS_FLAG)0U);
        }

        pEventFlagNode = pEventFlagNode->pFlagNodeNext;
    }

    OS_CRTICAL_END();
    
    if(sched == OS_TRUE)
    {
    	OS_Sched();											/* Preempt if it's ready. 											*/
    }

    OS_CRTICAL_BEGIN();
    flags_current = pflagGrp->OSFlagCurrent;
    OS_CRTICAL_END();

    return (flags_current);
}


#endif 		/* OS_CONFIG_FLAG_EN */
