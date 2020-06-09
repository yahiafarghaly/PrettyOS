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

#include <pretty_arch.h>
#include "pretty_config.h"


/* OS return Values */

#define OS_RET_OK                 (0U)
#define OS_RET_ERROR              (1U)
#define OS_RET_ERROR_PARAM        (2U)

/* OS Macros and Data Types  */

#define OS_NULL                         ((void*)0U)
#define OS_TRUE                         (1U)
#define OS_FAlSE                        (0U)
#define OS_MAX_NUMBER_TASKS             (sizeof(OS_tCPU_DATA)*OS_CONFIG_PRIORTY_ENTRY_COUNT)
#define OS_HIGHEST_PRIO_LEVEL           (0U)
#define OS_LOWEST_PRIO_LEVEL            (OS_MAX_NUMBER_TASKS - 1U)
#define OS_IDLE_TASK_PRIO_LEVEL         (OS_LOWEST_PRIO_LEVEL)

extern OS_tRet OS_Init(OS_tCPU_DATA* pStackBaseIdleTask,
                       OS_tCPU_DATA  stackSizeIdleTask);

extern OS_tRet OS_Start(void);

extern OS_tRet OS_CreateTask(void (*TASK_Handler)(void* params),
                             void *params,
                             OS_tCPU_DATA* pStackBase,
                             OS_tCPU_DATA  stackSize,
                             OS_tCPU_DATA priority);

extern void OS_TimeTick (void);

/* TO Implement by the user to handle the idle condition. */
extern void OS_onIdle(void);


#ifdef __cplusplus
}
#endif
#endif /* __PRETTY_OS_H_ */
