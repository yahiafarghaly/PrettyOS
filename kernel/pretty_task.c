/*
 * pretty_task.c
 *
 *  Created on: Jun 8, 2020
 *      Author: yf
 */


#include "pretty_os.h"

#define OS_IS_VALID_PRIO(_prio)         ((_prio < OS_IDLE_TASK_PRIO_LEVEL) && (_prio >= OS_HIGHEST_PRIO_LEVEL))

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

/* Array of TCBs, Each Containing the task internal data. */
static OS_TASK_TCB OS_TCB_Table[OS_MAX_NUMBER_TASKS] = { 0 };
/* Array of bit-mask of tasks that are ready to run. */
static OS_tCPU_DATA OS_ReadyTable[OS_CONFIG_PRIORTY_ENTRY_COUNT] = { 0 };

/*
 * Function:  OS_PriorityHighestGet
 * --------------------
 * Get the highest priority of a task which is in a ready state.
 *
 * Arguments: None.
 *
 * Returns: The highest priority number.
 */
OS_tCPU_DATA
OS_PriorityHighestGet(void)
{
    OS_tCPU_DATA   *p_tbl;
    OS_tCPU_DATA    prio;

    prio  = 0U;
    p_tbl = &OS_ReadyTable[0];
    /* Loop Through Entries until find a non empty entry */
    while (*p_tbl == (OS_tCPU_DATA)0) {
        /* Advance by a Complete Entry */
        prio += sizeof(OS_tCPU_DATA);
        p_tbl++;
    }
    prio += (sizeof(OS_tCPU_DATA) - (OS_tCPU_DATA)OS_CPU_CountLeadZeros(*p_tbl));
    return (prio);
}

/*
 * Function:  OS_PrioritySet
 * --------------------
 * Insert a task with a certain priority from the ready state.
 *
 * Arguments:
 *          prio    is the task's priority.
 *
 * Returns: None
 */
void
OS_PrioritySet(OS_t32U prio)
{
    OS_tCPU_DATA bit_pos = prio & (sizeof(OS_tCPU_DATA) - 1);
    OS_tCPU_DATA entry_pos = prio / sizeof(OS_tCPU_DATA); //TODO: Optimize it with bitwise operation.
    OS_ReadyTable[entry_pos] |= (1U << bit_pos);
}

/*
 * Function:  OS_PriorityClear
 * --------------------
 * Remove a task with a certain priority from the ready state.
 *
 * Arguments:
 *          prio    is the task's priority.
 *
 * Returns: None
 */
void
OS_PriorityClear(OS_t32U prio)
{
    OS_tCPU_DATA bit_pos = prio & (sizeof(OS_tCPU_DATA) - 1);
    OS_tCPU_DATA entry_pos = prio / sizeof(OS_tCPU_DATA); //TODO: Optimize it with bitwise operation.
    OS_ReadyTable[entry_pos] &= ~(1U << bit_pos);
}


/*
 * Function:  OS_TCB_RegisterTask
 * --------------------
 * Add a task to the TCB table to be managed by Scheduler.
 *
 *
 * Called By: OS_CreateTask()
 *
 * Arguments:
 *          stackTop    is a pointer to the task's top of stack.
 *                      assuming that the CPU registers have been placed on the stack.
 *          priority    is the task's priority.
 *
 * Returns: OS_RET_OK               if successful operation is done.
 *          OS_RET_ERROR_PARAM      Invalid supplied parameter.
 */
OS_tRet
OS_TCB_RegisterTask(OS_tptr* stackTop,OS_tCPU_DATA priority)
{
    OS_tRet ret;
    OS_TASK_TCB*  thisTask;

    if(OS_NULL == stackTop)
    {
        return (OS_RET_ERROR_PARAM);
    }

    if(OS_CPU_likely(OS_IS_VALID_PRIO(priority)))
    {
        /*
         * TODO: Add the code for handling the same priority (Round-Robin).
         * */
        thisTask = &OS_TCB_Table[priority];
        thisTask->TASK_SP = stackTop;
        thisTask->TASK_priority = priority;
        OS_PrioritySet(priority);
        ret = OS_RET_OK;
    }
    else
    {
        ret = OS_RET_ERROR_PARAM;
    }

    return (ret);
}
