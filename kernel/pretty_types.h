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
 * Purpose  : PrettyOS data types.
 *
 * Language:  C
 * 
 * Set 1 tab = 4 spaces for better comments readability.
 */

#ifndef __PRETTY_TYPES_H_
#define __PRETTY_TYPES_H_


#ifdef __cplusplus
extern "C" {
#endif


/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/

#include <pretty_arch.h>
#include "pretty_config.h"


/*
*******************************************************************************
*                                OS Typedefs                                  *
*******************************************************************************
*/

                                            /* Fit tasks priority to the suitable data type.                                    */

#if   (OS_CONFIG_TASK_COUNT - 1) <= (0x00000000000000FF)         
    typedef CPU_t08U        OS_PRIO;
#elif (OS_CONFIG_TASK_COUNT - 1) <= (0x000000000000FFFF)
    typedef CPU_t16U        OS_PRIO;
#elif (OS_CONFIG_TASK_COUNT - 1) <= (0x00000000FFFFFFFF)
    typedef CPU_t32U        OS_PRIO;
#elif (OS_CONFIG_TASK_COUNT - 1) <= (0xFFFFFFFFFFFFFFFF)
    typedef CPU_t64U        OS_PRIO;
#endif


                                            /* Fit OS_FLAG to the suitable data type.                                           */
#if  (OS_FLAGS_NBITS == 8U)
    typedef	CPU_t08U	    OS_FLAG;
#elif (OS_FLAGS_NBITS == 16U)
    typedef	CPU_t16U	    OS_FLAG;
#elif (OS_FLAGS_NBITS == 32U)
    typedef	CPU_t32U	    OS_FLAG;
#elif (OS_FLAGS_NBITS == 64U)
    typedef	CPU_t64U	    OS_FLAG;
#else
	#error "OS_FLAGS_NBITS can only be 8, 16, 32 or 64 bits. "
#endif


typedef OS_PRIO                      OS_TASK_COUNT;              /* By analogy as Number of tasks <= Max. Priority levels.      */

typedef CPU_t08U                     OS_BOOLEAN;                 /* For true/false values.                                      */

typedef CPU_t08U                     OS_OPT;                     /* For options values.                                         */

typedef CPU_t08U                     OS_STATUS;                  /* For status values.                                          */

typedef CPU_t32U                     OS_TICK;                    /* Clock tick counter.                                         */

typedef CPU_tWORD                    OS_tRet;                    /* Fit to the easiest type of memory for CPU.                  */

typedef CPU_tSTK                   	 OS_tSTACK;                  /* OS task stack which should be word aligned.                 */

typedef CPU_t08U					 OS_FLAG_WAIT;				 /* OS Flag wait type for holding type of event flag wait.		*/



/*
*******************************************************************************
*                           OS Structures Definition                          *
*******************************************************************************
*/

/* ------------------- OS Generic Doubly Linked List ---------------------- */
typedef struct LIST_ITEM
{
  CPU_tWORD itemVal;                    /* The value being listed. This is used to sort the list in Ascending order. 					*/
  void * pOwner;                        /* Pointer to the object (Usually a TCB) that contains the list item.  							*/
  void * pList;                    		/* Pointer to the list in which this list item is placed (if any). 								*/
  volatile struct LIST_ITEM * next;     /* Pointer to the next 		ListItem in the list.  												*/
  volatile struct LIST_ITEM * prev;   	/* Pointer to the previous 	ListItem in the list. 												*/
}List_Item;

typedef struct LIST
{
  volatile List_Item* head;				/* The head of the list item linked list.														*/
  volatile List_Item* end;				/* The tail of the list item linked list.														*/
  volatile CPU_tWORD  itemsCnt;			/* The Number of Items in the list items.														*/
} List;

/* ---------------------- OS EDF Scheduler Params --------------------------- */

typedef struct os_edf_sched_params		OS_EDF_SCHED_PARAMS;
struct os_edf_sched_params
{
	OS_TICK	tick_arrive;
	OS_TICK tick_relative_deadline;
	OS_TICK tick_absolate_deadline;
	OS_OPT	task_type;
	OS_TICK task_period;
};

/* ------------------------ OS Task TCB Structure --------------------------- */

typedef struct os_task_event    		OS_EVENT;
typedef struct os_event_flag_node 		OS_EVENT_FLAG_NODE;
typedef struct os_task_tcb      		OS_TASK_TCB;
struct os_task_tcb
{
    CPU_tPtr    TASK_SP;        			/* Current Task's Stack Pointer (Must be at offset 0x0 from struct base address)*/

#if (OS_CONFIG_CPU_SOFT_STK_OVERFLOW_DETECTION == OS_CONFIG_ENABLE)
    void*       TASK_SP_Limit;              /* Task's stack pointer limit to for stack overflow detection.                  */
#endif

    OS_TICK     TASK_Ticks;     			/* Current Task's timeout    													*/

    OS_PRIO     TASK_priority;  			/* Task Priority
    																*/
#if (OS_CONFIG_EDF_EN == OS_CONFIG_ENABLE)
    OS_EDF_SCHED_PARAMS	EDF_params;
    List_Item* 			pListItemOwner;
#endif

    OS_STATUS   TASK_Stat;      			/* Task Status 																	*/

#if (OS_AUTO_CONFIG_INCLUDE_EVENTS 		== OS_CONFIG_ENABLE)

    OS_STATUS   TASK_PendStat;  			/* Task Pend Status 															*/

    OS_EVENT*   TASK_Event;     			/* Pointer to the attached event to this TCB. 									*/

    OS_TASK_TCB* OSTCB_NextPtr;      		/* Pointer to a TCB, In case of multiple TCBs pending on the same event object.	*/

#endif

#if (OS_CONFIG_FLAG_EN 					== OS_CONFIG_ENABLE)
    OS_FLAG		OSFlagReady;				/* Flags which made this TCB ready.												*/
#endif

#if (OS_CONFIG_TCB_TASK_ENTRY_STORE_EN 	== OS_CONFIG_ENABLE)

    void (*TASK_EntryAddr)(void*);
    void*  TASK_EntryArg;

#endif

#if (OS_CONFIG_TCB_EXTENSION_EN 		== OS_CONFIG_ENABLE)

    void*		OSTCBExtension; 			/* Pointer to user definable data for TCB extension.		 			   		*/

#endif

};


/* ------------------------ OS Event (ECB) Structure ------------------------ */
  
struct os_task_event
{
    CPU_t08U        OSEventType;            /* Event type                                                         			*/

    OS_EVENT*       OSEventPtr;             /* Pointer to 1) Queue structure of Free Events.
     	 	 	 	 	 	 	 	 	 	 	 	 	  2) or to a mailbox message [ (void*)0 means an Empty mailbox. ]
     	 	 	 	 	 	 	 	 	 	 	 	 	  3) or to a OS_TASK_TCB object which is owning a mutex. 			*/

    OS_TASK_TCB*    OSEventsTCBHead;        /* Pointer to the List of waited TCBs depending on this event.        			*/

    union{
        OS_SEM_COUNT    OSEventCount;       /* Semaphore Count                                                    			*/
        struct{
            OS_PRIO    OSMutexPrio;         /* The original priority task that owning the Mutex.
            									or 'OS_PRIO_RESERVED_MUTEX' if no task is owning the Mutex.                 */

            OS_PRIO    OSMutexPrioCeilP;    /* The raised priority to reduce the priority inversion bug.
            								 	 or 'OS_PRIO_RESERVED_MUTEX' if priority ceiling promotion is disabled.		*/
        };
    };
};

                                            /* OS services based on OS_EVENT structure.                                     */
typedef OS_EVENT                    		OS_SEM;

typedef OS_EVENT                    		OS_MUTEX;

typedef	OS_EVENT		            		OS_MAILBOX;

/* -------------------------- OS Event Flag Structure ---------------------- */

typedef struct os_event_flag_group 			OS_EVENT_FLAG_GRP;

struct os_event_flag_group
{
    CPU_t08U        	OSEventType;        /* Event type  ( Should be OS_EVENT_TYPE_FLAG )                                 */
    OS_FLAG				OSFlagCurrent;		/* Is a series of flags (i.e. bits) that holds the current status of events. 	*/
    OS_EVENT_FLAG_NODE*	pFlagNodeHead;		/* Pointer to the list of waited tasks of flags nodes for events.				*/
};

struct os_event_flag_node
{
	OS_EVENT_FLAG_GRP* 	pFlagGroup;			/* Pointer to the event flag group object related to this flag node. 			*/
	OS_EVENT_FLAG_NODE*	pFlagNodeNext;		/* Next flag node in this event flag group.										*/
	OS_TASK_TCB*		pTCBFlagNode;		/* Pointer to the TCB attached to this event flag node.							*/
	OS_FLAG				OSFlagWaited;		/* Flags (i.e bits) which are waited to meet to trigger the event flag.			*/
	OS_FLAG_WAIT		OSFlagWaitType;		/* Type of Flags (i.e bits) action to trigger the event flag.					*/
};

/* --------------------------- OS Memory Structure -------------------------- */

typedef struct os_memory        			OS_MEMORY;
struct os_memory
{
    void*	  		partitionBaseAddr;		/* The base address of a partition from which memory blocks will be allocated.	*/
    void*	  		nextFreeBlock;			/* Points to the next OS_MEMORY Block or the next free memory block.			*/
    OS_MEMORY_BLOCK blockSize;				/* Block memory size.															*/
    OS_MEMORY_BLOCK blockCount;				/* Total number of the memory blocks inside the partition.						*/
    OS_MEMORY_BLOCK blockFreeCount;			/* The number of the memory blocks which is currently available from partition. */
    										/* The number of used memory blocks is equal to (blockCount - blockFreeCount).	*/
};

/* ---------------------------- OS Time Structure -------------------------- */

typedef struct os_task_time     			OS_TIME;
struct os_task_time
{
    CPU_t08U hours;
    CPU_t08U minutes;
    CPU_t08U seconds;
    CPU_t16U milliseconds;
};

#ifdef __cplusplus
}
#endif

#endif /* __PRETTY_TYPES_H_ */
