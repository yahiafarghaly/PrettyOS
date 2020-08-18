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
 * Purpose  :	Event Flag Implementation.
 *
 *
 *
 * Language:  C
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

OS_EVENT_FLAG_GRP*
OS_EVENT_FlagCreate (OS_FLAG initial_flags)
{
	OS_EVENT_FLAG_GRP* pFlagGrp;
	CPU_SR_ALLOC();

    if(OS_IntNestingLvl > 0U)                           	/* Create only from task level code.	                	*/
    {
        OS_ERR_SET(OS_ERR_EVENT_CREATE_ISR);
        return OS_NULL(OS_EVENT_FLAG_GRP);
    }

    OS_CRTICAL_BEGIN();

    pFlagGrp = OS_EventFlagGroup_allocate();
    if(pFlagGrp == OS_NULL(OS_EVENT_FLAG_GRP))
    {
    	OS_CRTICAL_END();
    	OS_ERR_SET(OS_ERR_EVENT_POOL_EMPTY);
    	return OS_NULL(OS_EVENT_FLAG_GRP);
    }

    pFlagGrp->OSFlagBits	= initial_flags;				/* Set the desired set of initial bits for this flag group.	*/
    pFlagGrp->OSEventType	= OS_EVENT_TYPE_FLAG;			/* Setup the right type of the event.						*/
    pFlagGrp->pFlagNodeHead	= OS_NULL(OS_EVENT_FLAG_NODE);	/* Initially, No waiting tasks on this event.				*/

    OS_CRTICAL_END();
    OS_ERR_SET(OS_ERR_NONE);
    return (pFlagGrp);
}


#endif
