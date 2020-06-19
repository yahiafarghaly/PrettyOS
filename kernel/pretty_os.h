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
#define OS_RET_OK                 (0U)
#define OS_RET_ERROR              (1U)
#define OS_RET_ERROR_PARAM        (2U)

/*
*******************************************************************************
*                               OS Macros                                     *
*******************************************************************************
*/
#define OS_NULL                         ((void*)0U)
#define OS_TRUE                         (1U)
#define OS_FAlSE                        (0U)
#define OS_MAX_NUMBER_TASKS             (OS_CPU_WORD_SIZE_IN_BITS*OS_CONFIG_PRIORTY_ENTRY_COUNT)
#define OS_HIGHEST_PRIO_LEVEL           (0U)
#define OS_LOWEST_PRIO_LEVEL            (OS_MAX_NUMBER_TASKS - 1U)
#define OS_IDLE_TASK_PRIO_LEVEL         (OS_LOWEST_PRIO_LEVEL)
#define OS_IS_VALID_PRIO(_prio)         ((_prio <= OS_LOWEST_PRIO_LEVEL) && (_prio >= OS_HIGHEST_PRIO_LEVEL))

/*
*******************************************************************************
*                           OS Structures Definition                          *
*******************************************************************************
*/

/******************************* OS Task TCB *********************************/
typedef struct os_task_tcb
{
    /* Current Thread's Stack Pointer */
    OS_tptr TASK_SP;
    /* Current Thread's Time out */
    OS_t32U TASK_Ticks;
    /* Task Priority */
    OS_t32U TASK_priority;

    /* Doubly LinkedList Pointers for tasks with the same priority */
    struct os_task_tcb* TCB_Prev;
    struct os_task_tcb* TCB_Next;

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
 *          OS_RET_ERROR_PARAM      Invalid supplied parameter.
 */
extern OS_tRet OS_Init(OS_tCPU_DATA* pStackBaseIdleTask,
                       OS_tCPU_DATA  stackSizeIdleTask);

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
 *          priority                is the task priority.
 *                                        ( 0 => is the highest priority            )
 *                                        ( OS_IDLE_TASK_PRIO_LEVEL is not allowed  )
 *
 * Returns: OS_RET_OK               if successful operation is done.
 *          OS_RET_ERROR_PARAM      Invalid supplied parameter.
 */
extern OS_tRet OS_CreateTask(void (*TASK_Handler)(void* params),
                             void *params,
                             OS_tCPU_DATA* pStackBase,
                             OS_tCPU_DATA  stackSize,
                             OS_tCPU_DATA priority);
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
 * Arguments:
 *            ticks     is the number of ticks for the task to blocked.
 *
 * Returns: None.
 */
extern void OS_DelayTicks (OS_t32U ticks);
/*
 * Function:  OS_TimerTick
 * --------------------
 * Decrement the ticks for each task.
 *
 * Arguments: None.
 *
 * Returns: None.
 * Notes:   Must be called within the system tick handler.
 */
extern void OS_TimerTick (void);

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


#ifdef __cplusplus
}
#endif
#endif /* __PRETTY_OS_H_ */
