/*
 * pretty_os.h
 *
 *  Created on: Jun 8, 2020
 *      Author: yf
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
#define OS_RET_OK                                                         (0U)
#define OS_RET_ERROR                                                      (1U)

#define OS_ERR_PARAM                                                      (2U)

#define OS_ERR_PRIO_EXIST                                                 (3U)
#define OS_ERR_PRIO                                                       (4U)
#define OS_ERR_PRIO_INVALID                                               (5U)

#define OS_ERR_TASK_CREATE_ISR                                            (6U)

/*
*******************************************************************************
*                               OS Macros                                     *
*******************************************************************************
*/
#define OS_NULL                         ((void*)0U)
#define OS_TRUE                         (1U)
#define OS_FAlSE                        (0U)
#define OS_MAX_NUMBER_TASKS             (CPU_NumberOfBitsPerWord*OS_MAX_PRIO_ENTRIES)
#define OS_HIGHEST_PRIO_LEVEL           (OS_MAX_NUMBER_TASKS - 1U)
#define OS_LOWEST_PRIO_LEVEL            (0U)
#define OS_IDLE_TASK_PRIO_LEVEL         (OS_LOWEST_PRIO_LEVEL)
#define OS_IS_VALID_PRIO(_prio)         ((_prio >= OS_LOWEST_PRIO_LEVEL) && (_prio <= OS_HIGHEST_PRIO_LEVEL))

/*
*******************************************************************************
*                             OS Task Status                                  *
*******************************************************************************
*/
#define OS_TASK_STAT_READY          (0x00U) /* Ready.                        */
#define OS_TASK_STAT_DELAY          (0x01U) /* Delayed or Timeout.           */
#define OS_TASK_STAT_SUSPENDED      (0x02U) /* Suspended.                    */

/*
*******************************************************************************
*                           OS Structures Definition                          *
*******************************************************************************
*/

/******************************* OS Task TCB *********************************/
typedef struct os_task_tcb
{

    CPU_tPtr    TASK_SP;        /* Current Thread's Stack Pointer */

    OS_TICK     TASK_Ticks;     /* Current Thread's Time out */

    OS_PRIO     TASK_priority;  /* Task Priority */

    OS_STATUS   TASK_Stat;      /* Task Status */

}OS_TASK_TCB;

/*
*******************************************************************************
*                           OS Functions Prototypes                           *
*******************************************************************************
*/

/*
 * Function:  OS_Init
 * --------------------
 * Initialize the pretty-OS services.
 *
 * Arguments:
 *          pStackBaseIdleTask    is a pointer the bottom of the Idle stack.
 *          priority              is the task stack size.
 *
 * Returns: OS_RET_OK               if successful operation is done.
 *          OS_ERR_PARAM            Invalid supplied parameter.
 */
extern OS_tRet OS_Init(CPU_tWORD* pStackBaseIdleTask,
                       CPU_tWORD  stackSizeIdleTask);

/*
 * Function:  OS_CreateTask
 * --------------------
 * Normal Task Creation.
 *
 * Arguments:
 *          TASK_Handler            is a function pointer to the task code.
 *          params                  is a pointer to the user supplied data which is passed to the task.
 *          pStackBase              is a pointer to the bottom of the task stack.
 *          stackSize               is the task stack size.
 *          priority                is the task priority. ( A unique priority must be assigned to each task )
 *                                      - A greater number means a higher priority
 *                                      - 0 => is reserved for the OS' Idle Task.
 *                                      - OS_LOWEST_PRIO_LEVEL(0) < Allowed value <= OS_HIGHEST_PRIO_LEVEL
 *
 *
 * Returns: OS_RET_OK                       if successful operation is done.
 *          OS_ERR_PARAM                    Invalid supplied parameter.
 *          OS_RET_ERROR_TASK_CREATE_ISR    If a task is created inside an ISR.
 */
extern OS_tRet OS_CreateTask(void (*TASK_Handler)(void* params),
                             void *params,
                             CPU_tWORD* pStackBase,
                             CPU_tWORD  stackSize,
                             OS_PRIO    priority);

/*
 * Function:  OS_ChangeTaskPriority
 * --------------------
 * Change the priority of a task dynamically.
 *
 * Arguments    : oldPrio     is the old priority
 *
 *                newPrio     is the new priority
 *
 * Returns      : OS_RET_OK                 If successful operation is done.
 *                OS_ERR_PRIO_INVALID       The priority number is not in the accepted range.
 *                OS_ERR_PRIO_EXIST         The new priority is already exist.
 */
extern OS_tRet OS_ChangeTaskPriority(OS_PRIO oldPrio,
                                     OS_PRIO newPrio);

/*
 * Function:  OS_Run
 * --------------------
 * Start running and transfer the control to the Pretty OS to run the tasks.
 *
 * Arguments: None.
 *
 * Returns: None.
 */
extern void OS_Run(void);

/*
 * Function:  OS_DelayTicks
 * --------------------
 * Block the current task execution for number of system ticks.
 *
 * Arguments    :
 *                ticks : is the number of ticks for the task to be blocked.
 *
 * Returns      : None.
 *
 * Note(s)      : 1) This function is called only from task level code.
 */
extern void OS_DelayTicks (OS_TICK ticks);

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
extern void OS_IntEnter(void);

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
extern void OS_IntExit(void);

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
extern void OS_SchedLock(void);

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
extern void OS_SchedUnlock(void);

/*
*******************************************************************************
*                           OS Functions Prototypes                           *
*                 REQUIRED: User Implement these functions.                   *
*******************************************************************************
*/

/*
 * Function:  OS_onIdle
 * --------------------
 * This function runs in the Idle state of OS.
 *
 * Arguments: None.
 *
 * Returns: None.
 */
extern void OS_onIdle(void);

/*
*******************************************************************************
*                               Miscellaneous                                 *
*******************************************************************************
*/

#ifndef OS_MAX_PRIO_ENTRIES
#error  "pretty_config.h, Missing OS_MAX_PRIO_ENTRIES: Max number of entries of priority level count."
#else
    #if     OS_MAX_PRIO_ENTRIES < 1U
    #error  "pretty_config.h, OS_MAX_PRIO_ENTRIES must be >= 1"
    #endif
#endif

#ifdef __cplusplus
}
#endif
#endif /* __PRETTY_OS_H_ */
