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
 * Author   :   Yahia Farghaly Ashour
 *
 * Purpose  :   PrettyOS Public Header APIs.
 *
 * Language :   C
 * 
 * Set 1 tab = 4 spaces for better comments readability.
 */

#ifndef __PRETTY_OS_H_
#define __PRETTY_OS_H_

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
*                               Return Codes                                  *
*******************************************************************************
*/
typedef enum {
    OS_ERR_NONE 				    =(0x00U),     /* Successful Operation.                           */
    OS_ERR_PARAM				    =(0x01U),     /* Invalid Supplied Parameter.                     */

    OS_ERR_PRIO_EXIST				=(0x02U),     /* The priority is already reserved.               */
    OS_ERR_PRIO_INVALID				=(0x03U),     /* The priority is not valid number to OS.         */

    OS_ERR_TASK_CREATE_ISR			=(0x04U),     /* Cannot create a task inside an ISR.             */
    OS_ERR_TASK_SUSPEND_IDLE		=(0x05U),     /* Cannot suspend an Idle task.                    */
    OS_ERR_TASK_SUSPEND_PRIO		=(0x06U),     /* Invalid priority task to suspend.               */
    OS_ERR_TASK_SUSPENDED			=(0x07U),     /* Task is already suspended.                      */
    OS_ERR_TASK_CREATE_EXIST		=(0x08U),     /* Cannot re-create an exist task.                 */
    OS_ERR_TASK_RESUME_PRIO		    =(0x09U),     /* Invalid priority task to resume.                */
    OS_ERR_TASK_NOT_EXIST			=(0x10U),     /* The task is not valid/exist.                    */
    OS_ERR_TASK_DELETE_ISR			=(0x11U),     /* Cannot delete a task from an ISR.               */
    OS_ERR_TASK_DELETE_IDLE		    =(0x12U),     /* Cannot delete the Idle task.                    */

    OS_ERR_EVENT_PEVENT_NULL		=(0x13U),     /* OS_EVENT* is a  NULL pointer.                   */
    OS_ERR_EVENT_TYPE				=(0x14U),     /* Invalid event type.                             */
    OS_ERR_EVENT_PEND_ISR			=(0x15U),     /* Cannot pend an event inside an ISR.             */
    OS_ERR_EVENT_PEND_LOCKED		=(0x16U),     /* Cannot pend an event while scheduler is locked. */
    OS_ERR_EVENT_PEND_ABORT			=(0x17U),     /* Waiting for an event is aborted.                */
    OS_ERR_EVENT_POST_ISR           =(0x18U),     /* Cannot Post inside an ISR.                      */
    OS_ERR_EVENT_TIMEOUT			=(0x19U),     /* Event is not occurred within event timeout.     */
    OS_ERR_EVENT_POOL_EMPTY         =(0x20U),     /* No more space for the an OS_EVENT object.       */
    OS_ERR_EVENT_CREATE_ISR         =(0x21U),     /* Cannot create this event type inside an ISR.    */

    OS_ERR_MUTEX_LOWER_PCP          =(0x22U),     /* Priority of current owning mutex is less than PCP defined with mutex. */
    OS_ERR_MUTEX_NO_OWNER           =(0x23U),     /* No task is owning the Mutex while posting it.	 */

	OS_ERR_MAILBOX_POST_NULL		=(0x24U),	  /* Posting a NULL pointer inside a mailbox.		 */
	OS_ERR_MAILBOX_FULL				=(0x25U),	  /* Indicates Full mailbox that cannot post into.	 */

	OS_ERR_SEM_OVERFLOW				=(0x26U),	  /* Indicates that the semaphore count reaches max. */

	OS_ERR_MEM_INVALID_ADDR			=(0x27U),	  /* Invalid Memory Address to work with.			 */
	OS_ERR_MEM_INVALID_BLOCK_SIZE	=(0x28U),	  /* Invalid Memory Block size.						 */
	OS_ERR_MEM_NO_FREE_BLOCKS		=(0x29U),	  /* No free blocks in a memory partition.			 */
	OS_ERR_MEM_FULL_PARTITION		=(0x30U)	  /* Memory Partition is full of free memory blocks. */
}OS_ERR;

extern OS_ERR OS_ERRNO;                           /* Holds the last error code returned by the last executed prettyOS function. */

#if(OS_CONFIG_ERRNO_EN == OS_CONFIG_ENABLE)
    #define OS_ERR_SET(err)  do { OS_ERRNO = (OS_ERR)err; }while(0);
#else
    #define OS_ERR_SET(err)  do { /* This should prevent compiler warnings. */ } while(0);
#endif

/*
*******************************************************************************
*                               OS Macros                                     *
*******************************************************************************
*/
#define OS_NULL(T)                    ((T*)0U)
#define OS_TRUE                         (1U)
#define OS_FAlSE                        (0U)

#define OS_HIGHEST_PRIO_LEVEL           (OS_CONFIG_TASK_COUNT - 1U)
#define OS_LOWEST_PRIO_LEVEL            (0U)
#define OS_TCB_MUTEX_RESERVED           ((OS_TASK_TCB*)1U)

/**************************** OS Reserved Priorities *************************/
/********* Your Application should not assign any of these priorities ********/
#define OS_IDLE_TASK_PRIO_LEVEL         (OS_LOWEST_PRIO_LEVEL)
#define OS_PRIO_RESERVED_MUTEX          (1U)

#define OS_IS_VALID_PRIO(_prio)     ((_prio >= OS_LOWEST_PRIO_LEVEL) && (_prio <= OS_HIGHEST_PRIO_LEVEL))

#define OS_IS_RESERVED_PRIO(_prio)  ((_prio == OS_IDLE_TASK_PRIO_LEVEL) ||  (_prio == OS_PRIO_RESERVED_MUTEX))

/*
*******************************************************************************
*              OS Task Status (Status codes for TASK_Stat)                    *
*******************************************************************************
*/
#define OS_TASK_STAT_READY          (0x00U)                      /* Ready.                        */
#define OS_TASK_STAT_DELAY          (0x01U)                      /* Delayed or Timeout.           */
#define OS_TASK_STAT_SUSPENDED      (0x02U)                      /* Suspended.                    */
#define OS_TASK_STATE_PEND_SEM      (0x04U)                      /* Pend on semaphore.            */
#define OS_TASK_STATE_PEND_MUTEX    (0x08U)                      /* Pend on mutex.                */
#define OS_TASK_STATE_PEND_MAILBOX	(0x10U)						 /* Pend on message arrival.	  */

#define OS_TASK_STAT_DELETED        (0xFFU)                      /* A deleted task or not created.*/

#define OS_TASK_STATE_PEND_ANY      (OS_TASK_STATE_PEND_SEM | OS_TASK_STATE_PEND_MUTEX | OS_TASK_STATE_PEND_MAILBOX)

/*
*******************************************************************************
*             TASK PEND STATUS (Status codes for TASK_PendStat)               *
*******************************************************************************
*/
#define  OS_STAT_PEND_OK            (0U)  /* Pending status OK, not pending, or pending complete     */
#define  OS_STAT_PEND_TIMEOUT       (1U)  /* Pending timed out                                       */
#define  OS_STAT_PEND_ABORT         (2U)  /* Pending aborted                                         */

/*
*******************************************************************************
*                             OS Events types                                 *
*******************************************************************************
*/
#define  OS_EVENT_TYPE_UNUSED           (0U)
#define  OS_EVENT_TYPE_SEM              (1U)
#define  OS_EVENT_TYPE_MUTEX            (2U)
#define  OS_EVENT_TYPE_MAILBOX			(3U)

/*
*******************************************************************************
*                               OS options                                    *
*******************************************************************************
*/
#define  OS_OPT_DEFAULT             (0U)                /* Default option of any services of PrettyOS opt        */

/***************************** Semaphores opt *********************************/
#define  OS_SEM_ABORT_HPT           (OS_OPT_DEFAULT)    /* Abort only higher priority waiting task.              */
#define  OS_SEM_ABORT_ALL           (1U)                /* Abort all waiting priority tasks.                     */

/*******************   Mutual Exclusion Semaphore opt *************************/
#define OS_MUTEX_PRIO_CEIL_DISABLE  (OS_OPT_DEFAULT)    /* Disable priority ceiling promotion for mutex.         */
#define OS_MUTEX_PRIO_CEIL_ENABLE   (1U)                /* Enable priority ceiling promotion for mutex.          */


/*
*******************************************************************************
*                                OS Typedefs                                  *
*******************************************************************************
*/

#if   (OS_CONFIG_TASK_COUNT - 1) <= (0x00000000000000FF)         /* Fit tasks priority to the correct data type. */
    typedef CPU_t08U        OS_PRIO;
#elif (OS_CONFIG_TASK_COUNT - 1) <= (0x000000000000FFFF)
    typedef CPU_t16U        OS_PRIO;
#elif (OS_CONFIG_TASK_COUNT - 1) <= (0x00000000FFFFFFFF)
    typedef CPU_t32U        OS_PRIO;
#elif (OS_CONFIG_TASK_COUNT - 1) <= (0xFFFFFFFFFFFFFFFF)
    typedef CPU_t64U        OS_PRIO;
#endif

typedef OS_PRIO                      OS_TASK_COUNT;              /* By analogy as #max priority level >= #tasks.  */
typedef CPU_t08U                     OS_BOOLEAN;                 /* For true/false values.                        */
typedef CPU_t08U                     OS_OPT;                     /* For options values.                           */
typedef CPU_t08U                     OS_STATUS;                  /* For status values.                            */
typedef CPU_t32U                     OS_TICK;                    /* Clock tick counter.                           */

typedef CPU_tWORD                    OS_tRet;                    /* Fit to the easiest type of memory for CPU.    */
typedef CPU_tSTK                   	 OS_tSTACK;                  /* OS task stack which is word aligned.          */

                                                                 /* OS various structures.                        */
typedef struct      os_task_event    OS_EVENT;
typedef struct      os_task_tcb      OS_TASK_TCB;
typedef struct      os_task_time     OS_TIME;
typedef struct		os_memory		 OS_MEMORY;
                                                                 /* OS services based on OS_EVENT structure.      */
typedef             OS_EVENT         OS_SEM;
typedef             OS_EVENT         OS_MUTEX;
typedef				OS_EVENT		 OS_MAILBOX;


/*
*******************************************************************************
*                           OS Structures Definition                          *
*******************************************************************************
*/
struct os_task_tcb
{
    CPU_tPtr    TASK_SP;        			/* Current Thread's Stack Pointer 												*/

    OS_TICK     TASK_Ticks;     			/* Current Thread's Time out 													*/

    OS_PRIO     TASK_priority;  			/* Task Priority 																*/

    OS_STATUS   TASK_Stat;      			/* Task Status 																	*/

#if (OS_AUTO_CONFIG_INCLUDE_EVENTS 		== OS_CONFIG_ENABLE)

    OS_STATUS   TASK_PendStat;  			/* Task pend status 															*/

    OS_EVENT*   TASK_Event;     			/* Pointer to this TCB event 													*/

    OS_TASK_TCB* OSTCB_NextPtr;      		/* Pointer to a TCB (In case of multiple events on the same event object).		*/

#endif

#if (OS_CONFIG_TCB_TASK_ENTRY_STORE_EN 	== OS_CONFIG_ENABLE)

    void (*TASK_EntryAddr)(void*);
    void*  TASK_EntryArg;

#endif

#if (OS_CONFIG_TCB_EXTENSION_EN 		== OS_CONFIG_ENABLE)

    void*		OSTCBExtension; 			/* Pointer to user definable data for TCB extension.		 			   		*/

#endif

};

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

struct os_memory {
    void*	  		partitionBaseAddr;			/* The base address of a partition from which memory blocks will be allocated.	*/
    void*	  		nextFreeBlock;				/* Points to the next OS_MEMORY Block or the next free memory block.			*/
    OS_MEMORY_BLOCK blockSize;					/* Block memory size.															*/
    OS_MEMORY_BLOCK blockCount;					/* Total number of the memory blocks inside the partition.						*/
    OS_MEMORY_BLOCK blockFreeCount;				/* The number of the memory blocks which is currently available from partition. */
    											/* The number of used memory blocks is equal to (blockCount - blockFreeCount).	*/
};

struct os_task_time
{
    CPU_t08U hours;
    CPU_t08U minutes;
    CPU_t08U seconds;
    CPU_t16U milliseconds;
};

/*
*******************************************************************************
*                             OS Core Functions                               *
*******************************************************************************
*/

/*
 * Function:  OS_Init
 * --------------------
 * Initialize the prettyOS services.
 *
 * Arguments    :  pStackBaseIdleTask    is a pointer to the bottom of the Idle task stack.(i.e stack[0] of the task).
 *                 priority              is the task stack size.
 *
 * Returns      :  OS_RET_OK, OS_ERR_PARAM
 */
extern OS_tRet OS_Init (CPU_tSTK* pStackBaseIdleTask, CPU_tSTK stackSizeIdleTask);

/*
 * Function:  OS_Run
 * --------------------
 * Start running and transfer the control to the PrettyOS to run the tasks.
 *
 * Arguments    : cpuClockFreq          is the running CPU frequency in Hertz .
 *
 * Returns      : None.
 */
extern void OS_Run (CPU_t32U cpuClockFreq);

/*
 * Function:  OS_TimerTick
 * --------------------
 * Signal the occurrence of a "system tick" to the prettyOS which reflects
 * to the services depending on this "system tick".
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        : 1) This function must be called from a ticker ISR.
 */
extern void OS_TimerTick (void);

/*
 * Function:  OS_IntEnter
 * --------------------
 * Notify PrettyOS that you are about to service an interrupt service routine (ISR).
 * This allows PrettyOS to keep track of the nested interrupts and thus performing the
 * rescheduling at the last nested ISR.
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function must be called with interrupts disabled.
 *                  2) You MUST invoke OS_IntEnter() and OS_IntExit() in pair.
 *                      For every call of OS_IntEnter() at the ISR beginning, you have to call OS_IntExit()
 *                      at the end of the ISR.
 *                  3) Nested interrupts are allowed up to 255 interrupts.
 */
extern void OS_IntEnter (void);

/*
 * Function:  OS_IntExit
 * --------------------
 * Notify PrettyOS that you have completed servicing an ISR. When the last nested ISR has completed.
 * the PrettyOS Scheduler is called to determine the new, highest-priority task is ready to run.
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        :   1) You MUST invoke OS_IntEnter() and OS_IntExit() in pair.
 *                      For every call of OS_IntEnter() at the ISR beginning, you have to call OS_IntExit()
 *                      at the end of the ISR.
 */
extern void OS_IntExit (void);

/*
 * Function:  OS_SchedLock
 * --------------------
 * Prevent re-scheduling to take place.
 * The task that calls OSSchedLock() keeps control of the CPU
 * even though other higher priority tasks are ready to run.
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        :   1) You MUST invoke OS_SchedLock() and OS_SchedUnlock() in pair.
 *                  2) The system ISRs are still serviced.
 *                  3) Must be used with caution because it affects the normal management of tasks.
 *                     And your application must not make any system calls that suspend execution
 *                     of the current task since this may lead to system lock-up.
 *                  4) Nested lock are up to 255 locks.
 */
extern void OS_SchedLock (void);

/*
 * Function:  OS_SchedUnlock
 * --------------------
 * Re-allow re-scheduling.
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        :   1) You MUST invoke OS_SchedLock() and OS_SchedUnlock() in pair.
 *                  2) It calls the OS scheduler when all nesting locks are unlocked
 *                     because the current task could have made higher priority tasks ready to run
 *                     while scheduling was locked.
 */
extern void OS_SchedUnlock (void);

/*
 * Function:  OS_StrError
 * --------------------
 * Return a constant string describing the error code.
 *
 * Arguments    : errno         is an error code defined in the enum list of OS_ERR
 *
 * Returns      :               A const pointer to a const char array values describing the error code.
 */
extern char const* const OS_StrError (OS_ERR errno);

/*
 * Function:  OS_StrLastErrIfFail
 * --------------------
 * Return a constant string describing the last error occured.
 *
 * Arguments    :    None.
 *
 * Returns      :    A const pointer to a const char array values describing the error code.
 *                   "Success" string if OS_ERR_NONE was the last error.
 */
extern char const* const OS_StrLastErrIfFail (void);
/*
*******************************************************************************
*                       PrettyOS Time functions                               *
*******************************************************************************
*/

/*
 * Function:  OS_DelayTicks
 * --------------------
 * Block the current task execution for number of system ticks.
 *
 * Arguments    :   ticks   is the number of ticks for the task to be blocked.
 *
 * Returns      :   None.
 *
 * Note(s)      :   1) This function is called only from task level code.
 */
extern void OS_DelayTicks (OS_TICK ticks);

/*
 * Function:  OS_DelayTime
 * --------------------
 * Block the current task execution for a time specified in the OS_TIME structure.
 *
 * Arguments    :   ptime   is a pointer to an OS_TIME structure where time is specified ( Hours, Minutes, seconds and milliseconds.)
 *
 * Returns      :   None.
 *
 * Note(s)      :   1) This function is called only from task level code.
 *                  2) A non valid value of any member of the internal structure of the OS_TIME object results in an immediate return.
 *                  3) This call can be expensive for some MCUs.
 */
extern void OS_DelayTime(OS_TIME* ptime);

/*
 * Function:  OS_TickTimeGet
 * --------------------------
 * Obtain the current value of the time counter which keeps track of the number of clock ticks
 * occurred since the first system tick of system ISR ticker.
 *
 * Arguments    :   None.
 *
 * Returns      :   The current value of OS_TickTime
 */
extern OS_TICK OS_TickTimeGet (void);

/*
 * Function:  OS_TickTimeSet
 * --------------------------
 * Set the OS_TickTime to a new value.
 *
 * Arguments    :   tick	is the new value of OS_TickTime to be set.
 *
 * Returns      :   None.
 */
extern void OS_TickTimeSet (OS_TICK tick);

/*
 * Function:  OS_TimeGet
 * --------------------------
 * Obtain the current value of system time in OS_TIME structure.
 *
 * Arguments    :   ptime	is a pointer to a valid OS_TIME structure which will contain the current system time.
 *
 * Returns      :   None.
 */
extern void OS_TimeGet (OS_TIME* ptime);

/*
*******************************************************************************
*                   OS Semaphore function Prototypes                          *
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
OS_SEM* OS_SemCreate (OS_SEM_COUNT cnt);

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
 * Return       :   OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL,OS_ERR_EVENT_TYPE, OS_ERR_EVENT_PEND_ISR
 * 								 OS_ERR_EVENT_PEND_LOCKED, OS_ERR_EVENT_PEND_ABORT, OS_ERR_EVENT_TIMEOUT }
 *
 * Notes        :   1) This function must used only from Task code level and not an ISR.
 */
void OS_SemPend (OS_SEM* pevent, OS_TICK timeout);

/*
 * Function:  OS_SemPost
 * --------------------
 * Signal a semaphore.
 *
 * Arguments    :   pevent      is a pointer to the OS_EVENT object associated with the semaphore.
 *
 * Returns      :   OS_ERRNO = { OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_NONE }
 *
 * Notes        :   1) This function can be called from a task code or an ISR.
 */
void OS_SemPost (OS_SEM* pevent);

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
OS_SEM_COUNT OS_SemPendNonBlocking (OS_SEM* pevent);

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
 * Returns      :   OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_EVENT_PEND_ABORT }
 *
 * Note(s)      :   1) This function can be called from a task code or an ISR.
 */
void OS_SemPendAbort (OS_SEM* pevent, CPU_t08U opt, OS_TASK_COUNT* abortedTasksCount);

/*
 * Function:  OS_SemGetCount
 * -------------------------
 * Return the number of semaphore resources.
 *
 * Arguments    :   pevent      is a pointer to the OS_EVENT object associated with the semaphore.
 *
 * 					pCount		is a pointer to a semaphore count type to hold the number of the semaphore resources[Counts]
 *
 * Return(s)	:	OS_ERRNO = { OS_ERR_NONE, OS_ERR_PARAM, OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE }
 *
 * Note(s)      :   1) This function can be called from a task code or an ISR.
 */
void OS_SemGetCount (OS_SEM* pevent, OS_SEM_COUNT* pCount);

/*
*******************************************************************************
*                       OS Mutex function Prototypes                          *
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
OS_MUTEX* OS_MutexCreate (OS_PRIO prio, OS_OPT opt);

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
void OS_MutexPend (OS_MUTEX* pevent, OS_TICK timeout);

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
void OS_MutexPost (OS_MUTEX* pevent);

/*
*******************************************************************************
*                       OS Mailbox function Prototypes                        *
*******************************************************************************
*/

/*
 * Function:  OS_MutexCreate
 * --------------------
 * Creates a message mailbox container.
 *
 * Arguments    :   p_message    is a pointer sized variable which points to the message you desire to deposit at the creation
 * 								 of the mailbox.
 *
 * 								 If you set p_message to ((void*)0) (i.e NULL pointer) then the mailbox will be considered empty.
 * 								 otherwise, it is Full.
 *
 * Returns      :  != (OS_MAILBOX*)0U  is a pointer to OS_EVENT object of type OS_EVENT_TYPE_MAILBOX associated with the created mailbox.
 *                 == (OS_MAILBOX*)0U  if no events object were available.
 *
 *                 OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_POOL_EMPTY, OS_ERR_EVENT_CREATE_ISR }
 *
 * Note(s)      :   1) This function is used only from a Task code level.
 */
OS_MAILBOX* OS_MailBoxCreate (void* p_message);

/*
 * Function:  OS_MailBoxPend
 * --------------------
 * Waits for a message arrival or within a finite time if 'timeout' is set.
 *
 * Arguments    :   pevent    	is a pointer to an OS_EVENT object associated with a mailbox object.
 *
 *                  timeout     is an optional timeout period (in clock ticks).  If non-zero, your task will
 *                              wait for message arrival up to the amount of time specified by this argument.
 *                              If you specify 0, however, your task will wait forever at the specified
 *                              mailbox or, until a messages arrives.
 *
 * Returns      :  	!= (void*)0 is a pointer to the message which is received.
 * 					== (void*)0 If no message is received or 'pevent' is a NULL pointer.
 *
 * 					OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL,OS_ERR_EVENT_TYPE, OS_ERR_EVENT_PEND_ISR
 * 								 OS_ERR_EVENT_PEND_LOCKED, OS_ERR_EVENT_PEND_ABORT, OS_ERR_EVENT_TIMEOUT }
 *
 * Note(s)      :   1) This function is used only from a Task code level.
 */
void* OS_MailBoxPend (OS_MAILBOX* pevent, OS_TICK timeout);

/*
 * Function:  OS_MailBoxPost
 * --------------------
 * Sends a message to a mailbox.
 *
 * Arguments    :   pevent    	is a pointer to an OS_EVENT object associated with a mailbox object.
 *
 * 					p_message	is a pointer to a message to send.
 * 								If it's NULL, then you're posting nothing. This will return with an error.
 *
 * Returns      :  	OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_MAILBOX_POST_NULL, OS_ERR_MAILBOX_FULL }
 *
 * Note(s)      :   1) This function can be used from a Task code level or an ISR.
 */
void OS_MailBoxPost (OS_MAILBOX* pevent, void* p_message);

/*
 * Function:  OS_MailBoxRead
 * --------------------
 * Read the message in the Mailbox without waiting/pending.
 *
 * Arguments    :   pevent    	is a pointer to an OS_EVENT object associated with a mailbox object.
 *
 * Returns      :  	!= (void*)0 is a pointer to the message which is received.
 * 					== (void*)0 If no message is received or 'pevent' is a NULL pointer or invalid type of OSEventType.
 *
 * 					OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL,OS_ERR_EVENT_TYPE }
 *
 * Note(s)      :   1) This function can be used from task level code or an ISR.
 */
void* OS_MailBoxRead(OS_MAILBOX* pevent);

/*
*******************************************************************************
*                         PrettyOS Task functions                             *
*******************************************************************************
*/

/*
 * Function:  OS_TaskCreate
 * --------------------
 * Normal Task Creation.
 *
 *
 * Arguments    :   TASK_Handler            is a function pointer to the task code.
 *                  params                  is a pointer to the user supplied data which is passed to the task.
 *                  pStackBase              is a pointer to the bottom of the task stack.
 *                  stackSize               is the task stack size.
 *                  priority                is the task priority. ( A unique priority must be assigned to each task )
 *                                              - A greater number means a higher priority
 *                                              - 0 => is reserved for the OS'Idle Task.
 *                                              - 1 => is reserved for OS use.
 *                                              - OS_LOWEST_PRIO_LEVEL(0) < Allowed value <= OS_HIGHEST_PRIO_LEVEL
 *
 * Returns      :   OS_RET_OK, OS_ERR_PARAM, OS_RET_ERROR_TASK_CREATE_ISR
 */
OS_tRet OS_TaskCreate (void (*TASK_Handler)(void* params),
                             void *params,
                             CPU_tSTK* pStackBase,
                             CPU_tSTK_SIZE  stackSize,
                             OS_PRIO    priority);
/*
 * Function:  OS_TaskDelete
 * -------------------------
 * Delete a task given its priority. It can delete the calling task itself.
 * The deleted task is moved to a dormant state and can be re-activated again by creating the deleted task.
 *
 * Arguments    :   prio    is the task priority.
 *
 * Returns      :   OS_RET_OK, OS_ERR_TASK_DELETE_ISR, OS_ERR_TASK_DELETE_IDLE, OS_ERR_PRIO_INVALID, OS_ERR_TASK_NOT_EXIST.
 */
OS_tRet OS_TaskDelete (OS_PRIO prio);

/*
 * Function:  OS_TaskChangePriority
 * --------------------
 * Change the priority of a task dynamically.
 *
 * Arguments    :   oldPrio     is the old priority
 *                  newPrio     is the new priority
 *
 * Returns      :   OS_RET_OK, OS_ERR_PRIO_INVALID, OS_ERR_PRIO_EXIST, OS_ERR_TASK_NOT_EXIST
 */
OS_tRet OS_TaskChangePriority (OS_PRIO oldPrio, OS_PRIO newPrio);

/*
 * Function:  OS_TaskSuspend
 * -------------------------
 * Suspend a task given its priority.
 * This function can suspend the calling task itself.
 *
 * Arguments    :   prio    is the task priority.
 *
 * Returns      :   OS_RET_OK, OS_RET_TASK_SUSPENDED, OS_ERR_TASK_SUSPEND_IDEL, OS_ERR_PRIO_INVALID, OS_ERR_TASK_SUSPEND_PRIO
 */
OS_tRet OS_TaskSuspend (OS_PRIO prio);

/*
 * Function:  OS_TaskResume
 * ------------------------
 * Resume a suspended task given its priority.
 *
 * Arguments    :   prio  is the task priority.
 *
 * Returns      :   OS_RET_OK, OS_ERR_TASK_RESUME_PRIO, OS_ERR_PRIO_INVALID.
 */
OS_tRet OS_TaskResume (OS_PRIO prio);

/*
 * Function:  OS_TaskStatus
 * --------------------
 * Return Task Status.
 *
 * Arguments    :   prio  is the task priority.
 *
 * Returns      :   OS_STATUS
 */
OS_STATUS OS_TaskStatus (OS_PRIO prio);

/*
*******************************************************************************
*                 PrettyOS Memory Management functions                        *
*******************************************************************************
*/

/*
 * Function:  OS_MemoryPartitionCreate
 * ----------------------------------
 * Create a fixed-sized memory partition which will be managed by prettyOS.
 *
 * Arguments    :  partitionBaseAddr	is the starting address of the memory partition.
 *
 *				   blockCount			is the number of the memory blocks of the created memory partition.
 *
 *				   blockSizeInBytes		is the number of bytes per memory block.
 *
 * Returns      :  == ((OS_MEMORY*)0U)  if memory partition creation fails in case of invalid of arguments or no free partition.
 * 				   != ((OS_MEMORY*)0U)  is the created memory partition.
 *
 * 				   OS_ERRNO = { OS_ERR_NONE, OS_ERR_MEM_INVALID_ADDR, OS_ERR_MEM_INVALID_BLOCK_SIZE }
 */
OS_MEMORY* OS_MemoryPartitionCreate (void* partitionBaseAddr, OS_MEMORY_BLOCK blockCount, OS_MEMORY_BLOCK blockSizeInBytes);

/*
 * Function:  OS_MemoryAllocateBlock
 * ---------------------------------
 * Allocate a free block from a valid memory partition.
 *
 * Arguments    :  pMemoryPart		is a pointer to a valid memory partition structure.
 *
 * Returns      :  == ((void*)0U)  if no free memory block is available or invalid pointer of memory partition structure.
 * 				   != ((void*)0U)  is the requested block of memory.
 *
 * 				   OS_ERRNO = { OS_ERR_NONE, OS_ERR_MEM_INVALID_ADDR, OS_ERR_MEM_NO_FREE_BLOCKS }
 */
void* OS_MemoryAllocateBlock (OS_MEMORY* pMemoryPart);

/*
 * Function:  OS_MemoryRestoreBlock
 * --------------------------------
 * Free/Restore a block to a valid memory partition.
 *
 * Arguments    :  	pMemoryPart		is a pointer to a valid memory partition structure.
 *
 * 					pBlock			is a pointer to the released block of the partition pointed by 'pMemoryPart'
 *
 * Returns      :	None.
 *
 * 				   	OS_ERRNO = { OS_ERR_NONE, OS_ERR_MEM_INVALID_ADDR, OS_ERR_MEM_FULL_PARTITION }
 *
 * Note(s)		:	This function is not aware if the returned block is the actually block which is allocated
 * 					from the given partition 'pMemoryPart'.
 * 					So, Caution must be considered. Otherwise, the software can be crashed.
 */
void OS_MemoryRestoreBlock (OS_MEMORY* pMemoryPart, void* pBlock);

/*
*******************************************************************************
*                                                                             *
*                      Application Hook Specific Functions                 	  *
*                [Preferred to do little work as much as possible]		      *
*                                                                             *
*******************************************************************************
*/


#if (OS_CONFIG_APP_TASK_IDLE == OS_CONFIG_ENABLE)
	void App_Hook_TaskIdle		(void);								/* Calls Application specific code in the idle state of prettyOS. 									*/
#endif

#if (OS_CONFIG_APP_TASK_SWITCH == OS_CONFIG_ENABLE)
	void App_Hook_TaskSwitch	(void);								/* Calls Application specific code when task context switch occurs. 								*/
#endif

#if (OS_CONFIG_APP_TASK_CREATED == OS_CONFIG_ENABLE)
	void App_Hook_TaskCreated 	(OS_TASK_TCB* ptcb);				/* Calls Application specific code when a task is created. 											*/
#endif

#if (OS_CONFIG_APP_TASK_DELETED == OS_CONFIG_ENABLE)
	void App_Hook_TaskDeleted 	(OS_TASK_TCB* ptcb);				/* Calls Application specific code when a task is deleted. 											*/
#endif

#if (OS_CONFIG_APP_TASK_RETURNED == OS_CONFIG_ENABLE)
	void App_Hook_TaskReturned	(OS_TASK_TCB* ptcb); 				/* Calls Application specific code when a task returns intentionally. 								*/
#endif

#if (OS_CONFIG_APP_TIME_TICK == OS_CONFIG_ENABLE)
	void App_Hook_TimeTick 		(void);  							/* Calls Application specific code when an OS system tick occurs. (i.e the single tick ! )			*/
#endif



/*
*******************************************************************************
*                      														  *
*                      Target Hook Specific Functions                 		  *
*                      	   [Required in CPU port]							  *
*             [Preferred to do little work as much as possible]				  *
*                      														  *
*******************************************************************************
*/

#if(OS_CONFIG_CPU_INIT == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_Init 			(void);					/* Hooked with OS_Init() and is called once before OS_Init() does a thing.						*/
#endif

#if(OS_CONFIG_CPU_IDLE == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_Idle		(void);						/* A low level CPU idle routine in the idle state of prettyOS.									*/
#endif

#if(OS_CONFIG_CPU_TASK_CREATED == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_TaskCreated 	(OS_TASK_TCB*	ptcb);	/* A low level CPU routine when a task is created.												*/
#endif

#if(OS_CONFIG_CPU_TASK_DELETED == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_TaskDeleted		(OS_TASK_TCB*	ptcb);	/* A low level CPU routine when a task is deleted.												*/
#endif

#if(OS_CONFIG_CPU_CONTEXT_SWITCH == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_ContextSwitch 	(void);					/* A low level CPU routine when a context switch occurs.										*/
#endif

#if(OS_CONFIG_CPU_TIME_TICK == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_TimeTick     	(void);					/* A low level CPU routine when an OS system tick occurs. (i.e the single tick ! )				*/
#endif


/*
*******************************************************************************
*																			  *
*																			  *
*                       OS Miscellaneous Configurations                       *
*                       			Check									  *
*                                     										  *
*******************************************************************************
*/

#ifndef OS_CONFIG_MEMORY_EN
	#error "Missing  OS_CONFIG_MEMORY_EN "
#endif

#ifndef OS_CONFIG_MUTEX_EN
	#error "Missing  OS_CONFIG_MUTEX_EN "
#endif

#ifndef OS_CONFIG_SEMAPHORE_EN
	#error "Missing  OS_CONFIG_SEMAPHORE_EN "
#endif

#ifndef OS_CONFIG_MAILBOX_EN
	#error "Missing  OS_CONFIG_MAILBOX_EN "
#endif

#ifndef OS_CONFIG_ERRNO_EN
	#error "Missing  OS_CONFIG_ERRNO_EN "
#endif

#ifndef OS_CONFIG_TCB_TASK_ENTRY_STORE_EN
	#error "Missing  OS_CONFIG_TCB_TASK_ENTRY_STORE_EN "
#endif

#ifndef OS_CONFIG_TCB_EXTENSION_EN
	#error "Missing  OS_CONFIG_TCB_EXTENSION_EN "
#endif

#ifndef OS_CONFIG_SYSTEM_TIME_SET_GET_EN
	#error "Missing OS_CONFIG_SYSTEM_TIME_SET_GET_EN"
#endif

#ifndef OS_CONFIG_TICKS_PER_SEC
    #error  "Missing OS_CONFIG_TICKS_PER_SEC"
#endif

#ifndef OS_CONFIG_TASK_COUNT
    #error  "Missing OS_CONFIG_TASK_COUNT"
#endif

#ifndef OS_CONFIG_MAX_EVENTS
    #error  "Missing OS_CONFIG_MAX_EVENTS"
#endif

#ifndef	OS_CONFIG_MEMORY_PARTITION_COUNT
	#error  "Missing OS_CONFIG_MEMORY_PARTITION_COUNT"
#endif

#ifndef OS_AUTO_CONFIG_INCLUDE_EVENTS
    #error  "Missing OS_AUTO_CONFIG_INCLUDE_EVENTS"
#endif



#ifdef __cplusplus
}
#endif
#endif /* __PRETTY_OS_H_ */
