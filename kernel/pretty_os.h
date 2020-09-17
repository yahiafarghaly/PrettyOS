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
#include "pretty_types.h"

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
	OS_ERR_TASK_POOL_EMPTY			=(0x50U),	  /* No more available TCB objects.					 */

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
	OS_ERR_MEM_FULL_PARTITION		=(0x30U),	  /* Memory Partition is full of free memory blocks. */

	OS_ERR_FLAG_GRP_POOL_EMPTY		=(0x31U),	  /* No more space for OS_EVENT_FLAG_GRP object.     */
	OS_ERR_FLAG_PGROUP_NULL			=(0x32U),	  /* OS_EVENT_FLAG_GRP is a NULL Pointer.			 */
	OS_ERR_FLAG_WAIT_TYPE			=(0x33U),	  /* Invalid wait type.								 */
    OS_ERR_FLAG_OPT_TYPE            =(0x34U),     /* Invalid flag option type.                       */

	OS_ERR_END						= (-1)
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

#define OS_TASK_STAT_READY          (0x00U)                     /* Ready.                        	*/

#define OS_TASK_STAT_DELAY          (0x01U)                     /* Delayed or Timeout.           	*/

#define OS_TASK_STAT_SUSPENDED      (0x02U)                     /* Suspended.                    	*/

#define OS_TASK_STATE_PEND_SEM      (0x04U)                     /* Pend on semaphore.            	*/

#define OS_TASK_STATE_PEND_MUTEX    (0x08U)                     /* Pend on mutex.                	*/

#define OS_TASK_STATE_PEND_MAILBOX	(0x10U)						/* Pend on message arrival.	  		*/

#define OS_TASK_STATE_PEND_FLAG		(0x20U)						/* Pend on Event Flag.				*/

#define OS_TASK_STAT_DELETED        (0xFFU)                     /* A deleted task or not created.	*/

#define OS_TASK_STATE_PEND_ANY      (OS_TASK_STATE_PEND_SEM | \
									 OS_TASK_STATE_PEND_MUTEX | \
									 OS_TASK_STATE_PEND_MAILBOX | \
										OS_TASK_STATE_PEND_FLAG)

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

#define  OS_EVENT_TYPE_FLAG				(4U)

/*
*******************************************************************************
*                        	OS Event Flag Wait types                          *
*******************************************************************************
*/

#define  OS_FLAG_WAIT_CLEAR_ALL			(0x01U)			/* Waits for ALL bits in an Event flag group to be CLEAR.					*/

#define	 OS_FLAG_WAIT_CLEAR_ANY			(0x02U)			/* Waits for ANY bits in an Event flag group to be CLEAR.					*/

#define	 OS_FLAG_WAIT_SET_ALL			(0x04U)			/* Waits for ALL bits in an Event flag group to be SET.						*/

#define	 OS_FLAG_WAIT_SET_ANY			(0x08U)			/* Waits for ANY bits in an Event flag group to be SET.						*/

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

/*****************************   Event Flag opt *******************************/

#define OS_FLAG_SET                 (1U)                /* Set Flags (i.e bits) to 1 in the desired location.    */

#define OS_FLAG_CLEAR               (2U)                /* Clear Flags (i.e bits) to 1 in the desired location.  */

/******************************* Task Type ************************************/

#define OS_TASK_PERIODIC			(1U)				/* EDF Task Parameter, typical in hard real-time and control applications. 		 					*/

#define OS_TASK_SPORADIC			(2U)				/* EDF Task Parameter, typical in soft real-time and multimedia applications. ( Not Implemented )	*/

#define OS_TASK_APERIODIC			(3U)				/* NOT Implemented yet.																				*/

/*
* =============================================================================
* =============================================================================
*
*
*                         PrettyOS' Core System APIs
*
*
* =============================================================================
* =============================================================================
* */

/*
 * Function:  OS_Init
 * --------------------
 * Initialize the prettyOS services.
 *
 * Arguments    :  pStackBaseIdleTask    is a pointer to the bottom of the Idle task stack.(i.e stack[0] of the task).
 *                 priority              is the task stack size.
 *
 * Return(s)    :  OS_RET_OK, OS_ERR_PARAM
 *
 * Note(s)		: The First API to be called before calling any of prettyOS APIs.
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
 * ============================================================================
 * ============================================================================
 *
 * 						PrettyOS' System Time Get/Set APIs
 *
 * ============================================================================
 * ============================================================================
 * */

#if(OS_CONFIG_SYSTEM_TIME_SET_GET_EN == OS_CONFIG_ENABLE)

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

#endif

/*
 * ============================================================================
 * ============================================================================
 *
 * 							 	PrettyOS' Error APIs
 *
 * ============================================================================
 * ============================================================================
 * */

#if (OS_CONFIG_ERRNO_EN == OS_CONFIG_ENABLE)

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

#endif

/*
 * ============================================================================
 * ============================================================================
 *
 * 							 PrettyOS' Hooks APIs
 *
 * ============================================================================
 * ============================================================================
 * */

#include "pretty_hooks.h"

/*
 * ============================================================================
 * ============================================================================
 *
 * 						PrettyOS' Kernel Services APIs
 *
 * ============================================================================
 * ============================================================================
 * */

#include "pretty_services.h"


#ifdef __cplusplus
}
#endif
#endif /* __PRETTY_OS_H_ */
