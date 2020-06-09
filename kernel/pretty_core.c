/*
 * pretty_core.c
 *
 *  Created on: Jun 8, 2020
 *      Author: yf
 */

#include "pretty_os.h"

typedef struct os_task_tcb OS_TASK_TCB;



extern OS_tRet OS_TCB_RegisterTask(OS_tptr* stackTop,OS_tCPU_DATA priority);


void
OS_IdleTask(void* args)
{
    args = args;
    while(1)
    {
        OS_onIdle();
    }
}


OS_tRet
OS_Init(OS_tCPU_DATA* pStackBaseIdleTask,
                       OS_tCPU_DATA  stackSizeIdleTask)
{

    OS_tRet ret;
    ret = OS_CreateTask(OS_IdleTask,
                        OS_NULL,
                        pStackBaseIdleTask,
                        stackSizeIdleTask,
                        OS_IDLE_TASK_PRIO_LEVEL);
    return (ret);
}

OS_tRet
OS_CreateTask(void (*TASK_Handler)(void* params),
                             void *params,
                             OS_tCPU_DATA* pStackBase,
                             OS_tCPU_DATA  stackSize,
                             OS_tCPU_DATA priority)

{
    OS_tCPU_DATA* stack_top;
    OS_tRet ret;

    if(TASK_Handler == OS_NULL || pStackBase == OS_NULL ||
            stackSize == 0U )
    {
        return (OS_RET_ERROR_PARAM);
    }

    OS_CRTICAL_BEGIN();

    stack_top = OS_CPU_TaskInit(TASK_Handler, params, pStackBase, stackSize);
    ret = OS_TCB_RegisterTask((OS_tptr*)stack_top,priority);

    OS_CRTICAL_END();

    return (ret);
}


