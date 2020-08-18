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
                     |          XXXX XXXXXXXXXXXXXXX|         |                +-------------->  |.pFlagNodeNext  +------------>  NULL
                     +------------------------------+         +----------------+                 +----------------+
                     |        pFlagNodeHead         |         |                |                 |                |
                     |                              +-------> |                |                 |                |
                     +------------------------------+         |                |                 |                |
                                                              +----------------+                 +----------------+
                                                              XXXXX XXX XXX XXXX                 XX      X XXXXXXX| .OSFlagBits
                                                              +----------------+                 +----------------+
                                                              |                |                 |                |
                                                              |   ^     +      |                 |   ^     +      | .pTCBFlagNode
                                                              +----------------+                 +----------------+
                                                                  |     |                            |     |
                                                                  |     |                            |     |
                                                                  |     |                            |     |
                                                                  |     |                            |     |
                                                                  |     |                            |     |
                                                                  |     |                            |     |
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

/* Fast allocation of OS_EVENT_FLAG_GRP object.								  */
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

/* Fast deallocation of OS_EVENT_FLAG_GRP object.							   */
static inline void OS_EventFlagGroup_deallocate (OS_EVENT_FLAG_GRP* peventFlagGrp)
{
	peventFlagGrp->pFlagNodeHead = (OS_EVENT_FLAG_NODE*)pFlagGroupFreeList;
	pFlagGroupFreeList = peventFlagGrp;
	pFlagGroupFreeList->OSEventType = OS_EVENT_TYPE_UNUSED;
	pFlagGroupFreeList->OSFlagBits	= 0U;
}

static inline void OS_EventFlag_PendCurrentTask(OS_EVENT_FLAG_GRP* pflagGrp, OS_EVENT_FLAG_NODE* pflagNode, OS_FLAG flags_pattern_wait, OS_FLAG_WAIT wait_type, OS_TICK timeout)
{
    OS_currentTask->TASK_Stat |= OS_TASK_STATE_PEND_FLAG;
    OS_currentTask->TASK_PendStat = OS_STAT_PEND_OK;
    OS_currentTask->TASK_Ticks = timeout;

    if(timeout > 0U)
    {
        OS_BlockTime(OS_currentTask->TASK_priority);		/* Add time delay block.								*/
        OS_currentTask->TASK_Stat |= OS_TASK_STAT_DELAY;
    }

    OS_currentTask->pEventFlagNode = pflagNode;				/* Link TCB to its Event Flag Node.						*/

    pflagNode->OSFlagBits 		= flags_pattern_wait;		/* Save the flags we're waiting for.					*/
    pflagNode->OSFlagWaitType 	= wait_type;				/* Save the type of wait.								*/
    pflagNode->pTCBFlagNode		= OS_currentTask;			/* Link to task's TCB.									*/
    pflagNode->pFlagGroup		= pflagGrp;					/* Link to the parent event flag group.					*/
    pflagNode->pFlagNodeNext	= pflagGrp->pFlagNodeHead;	/* Always insert node at the beginning of the list.		*/
    pflagGrp->pFlagNodeHead		= pflagNode;				/* Reset the head to the new node.						*/

    OS_RemoveReady(OS_currentTask->TASK_priority);			/* Finally, Remove task's TCB from the ready state.		*/
}

static inline void OS_EventFlag_UnlinkFlagNodeFromList(OS_EVENT_FLAG_NODE* pflagNode)
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
															/* Unlink the TCB from this node.						*/
	pflagNode->pTCBFlagNode->pEventFlagNode = OS_NULL(OS_EVENT_FLAG_NODE);
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
    	OSFlagGroupMemoryPool[i].OSFlagBits 	= 0U;
    }

    OSFlagGroupMemoryPool[OS_CONFIG_MAX_EVENT_FLAGS - 1U].pFlagNodeHead = OS_NULL(OS_EVENT_FLAG_NODE);
    OSFlagGroupMemoryPool[OS_CONFIG_MAX_EVENT_FLAGS - 1U].OSEventType  	= OS_EVENT_TYPE_UNUSED;
    OSFlagGroupMemoryPool[OS_CONFIG_MAX_EVENT_FLAGS - 1U].OSFlagBits  	= 0U;

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

    pflagGrp->OSFlagBits	= initial_flags;				/* Set the desired set of initial bits for this flag group.	*/
    pflagGrp->OSEventType	= OS_EVENT_TYPE_FLAG;			/* Setup the right type of the event.						*/
    pflagGrp->pFlagNodeHead	= OS_NULL(OS_EVENT_FLAG_NODE);	/* Initially, No waiting tasks on this event.				*/

    OS_CRTICAL_END();
    OS_ERR_SET(OS_ERR_NONE);
    return (pflagGrp);
}

OS_FLAG
OS_EVENT_FlagPend (OS_EVENT_FLAG_GRP* pflagGrp, OS_FLAG flags_pattern_wait, OS_FLAG_WAIT wait_type, OS_TICK timeout)
{
	OS_EVENT_FLAG_NODE	flag_node;
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
    														/* Check which flags are ready. 							*/
    flags_pattern_ready = (OS_FLAG)(pflagGrp->OSFlagBits & flags_pattern_wait);
    if(flags_pattern_ready == flags_pattern_wait)			/* Are flags matching to be ready ?							*/
    {
    	OS_CRTICAL_END();
    	return (flags_pattern_ready);						/* Yes, no need to continue. Return to the caller.			*/
    }
    														/* No, Check the wait type and pend ...						*/
    switch(wait_type)
    {
    case OS_FLAG_WAIT_CLEAR_ALL:
    	break;
    case OS_FLAG_WAIT_CLEAR_ANY:
    	break;
    case OS_FLAG_WAIT_SET_ALL:
    	break;
    case OS_FLAG_WAIT_SET_ANY:
    	break;
    default:
    	OS_CRTICAL_END();
		OS_ERR_SET(OS_ERR_FLAG_WAIT_TYPE);
		return ((OS_FLAG)0U);
    	break;
    }
    														/* Pend the current task till event occurs or timeout.		*/
    OS_EventFlag_PendCurrentTask(pflagGrp,&flag_node,flags_pattern_wait,wait_type,timeout);

    OS_CRTICAL_END();

    OS_Sched();												/* Preempt another HPT.										*/

    OS_CRTICAL_BEGIN();										/* We are back again ... 									*/

    pend_ok = OS_FAlSE;
    switch (OS_currentTask->TASK_PendStat) {                /* ... See if it was timed-out or aborted.                  */

        case OS_STAT_PEND_ABORT:
        	OS_ERR_SET(OS_ERR_EVENT_PEND_ABORT);            /* Indicate that we aborted.                                */
            break;

        case OS_STAT_PEND_TIMEOUT:
            OS_ERR_SET(OS_ERR_EVENT_TIMEOUT);				/* Indicate that we didn't get event within Time out.       */
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
    {
    	OS_EventFlag_UnlinkFlagNodeFromList(&flag_node);	/* Unlink the node from the wait list.						*/
    	OS_currentTask->pEventFlagNode	= OS_NULL(OS_EVENT_FLAG_NODE);
    	flags_pattern_ready = (OS_FLAG)0U;
    	return (flags_pattern_ready);
    }
    else
    {

    }

}

OS_FLAG
OS_EVENT_FlagPost (OS_EVENT_FLAG_GRP* pflagGrp, OS_FLAG flags_pattern_wait, OS_OPT flags_options)
{
	CPU_SR_ALLOC();

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
}


#endif 		/* OS_CONFIG_FLAG_EN */
