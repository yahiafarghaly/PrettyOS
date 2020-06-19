/*
 * pretty_core.c
 *
 *  Created on: Jun 8, 2020
 *      Author: yf
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"

/*
*******************************************************************************
*                               OS Macros                                     *
*******************************************************************************
*/


/*
*******************************************************************************
*                               Global Variables                              *
*******************************************************************************
*/
/* State of the OS. */
OS_tCPU_DATA volatile  OS_Running;
/* pointer to the current task. */
OS_TASK_TCB * volatile OS_currentTask;
/* pointer to the next task to run. */
OS_TASK_TCB * volatile OS_nextTask;

/* Array of TCBs, Each Containing the task internal data. */
static OS_TASK_TCB OS_TblTask[OS_MAX_NUMBER_TASKS] = { 0U };
/* Array of bit-mask of tasks that are ready to run. */
static OS_tCPU_DATA OS_TblReady[OS_CONFIG_PRIORTY_ENTRY_COUNT] = { 0U };
/* Array of bit-mask of tasks that blocked. */
static OS_tCPU_DATA OS_TblBlocked[OS_CONFIG_PRIORTY_ENTRY_COUNT] = { 0U };

/*
*******************************************************************************
*                     Internal Functions Prototypes                           *
*******************************************************************************
*/
static void OS_Sched(void);
static OS_tRet OS_TCB_RegisterTask(OS_tptr* stackTop,OS_tCPU_DATA priority);
static OS_tCPU_DATA OS_PriorityHighestGet(void);
static void OS_PrioritySet(OS_t32U prio);
static void OS_PriorityClear(OS_t32U prio);
static void OS_BlockTask(OS_t32U prio);
static void OS_UnBlockTask(OS_t32U prio);


/*
*******************************************************************************
*                                                                             *
*                           Pretty OS Functions                               *
*                                                                             *
*******************************************************************************
*/


/*
 * Function:  OS_IdleTask
 * --------------------
 * The task which runs when no other tasks are running.
 *
 * Arguments:
 *           args   is the function arguments.
 *
 * Returns: None.
 */
void
OS_IdleTask(void* args)
{
    /* Prevent compiler warning. */
    args = args;
    while(1)
    {
        /* Call user's idle function. */
        OS_onIdle();
    }
}

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
    OS_currentTask = OS_nextTask = (OS_TASK_TCB*)OS_NULL;
    OS_Running = OS_FAlSE;
    return (ret);
}

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
 *                                        ( Allowed range is 0 to (OS_IDLE_TASK_PRIO_LEVEL - 1) )
 *
 * Returns: OS_RET_OK               if successful operation is done.
 *          OS_RET_ERROR_PARAM      Invalid supplied parameter.
 */
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

    /* Call the low level function to initialize the stack frame of the task. */
    stack_top = OS_CPU_TaskInit(TASK_Handler, params, pStackBase, stackSize);
    ret = OS_TCB_RegisterTask((OS_tptr*)stack_top,priority);

    OS_CRTICAL_END();

    return (ret);
}

/*
 * Function:  OS_Sched
 * --------------------
 * Schedules the next highest priority task.
 *
 * Arguments: None.
 *
 * Returns: None.
 * Notes: Must be called with interrupt disabled.
 */
void OS_Sched(void)
{
    OS_tCPU_DATA OS_HighPrio =  OS_PriorityHighestGet();

    if(OS_IDLE_TASK_PRIO_LEVEL == OS_HighPrio)
    {
        OS_nextTask = &OS_TblTask[OS_IDLE_TASK_PRIO_LEVEL];
    }
    else
    {
        OS_nextTask = &OS_TblTask[OS_HighPrio];
    }

    if(OS_CPU_likely(OS_Running))
    {
        if(OS_nextTask != OS_currentTask &&
                OS_nextTask != (OS_TASK_TCB*)OS_NULL)
        {
            OS_CPU_ContexSwitch();
        }
    }
}

/*
 * Function:  OS_Run
 * --------------------
 * Start running and transfer the control to the Pretty OS to run the tasks.
 *
 * Arguments: None.
 *
 * Returns: None.
 */
void
OS_Run(void)
{
    if(OS_TRUE == OS_Running)
    {
        return;
    }
    else
    {
        OS_CRTICAL_BEGIN();
        /* Find the highest priority task to be scheduled. */
        OS_Sched();
        /* Start the first task. */
        OS_CPU_FirstStart();
        /* Enable the interrupt in case accidentally it is not enabled. */
        OS_CRTICAL_END();
    }

    /* This should never be executed. */
    for(;;);
}

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
void
OS_TimerTick (void)
{
    OS_tCPU_DATA i = 0;
    OS_tCPU_DATA workingSet;

    if(OS_Running == OS_FAlSE)
    {
        return;
    }

    OS_CRTICAL_BEGIN();

    for(i = 0; i < OS_CONFIG_PRIORTY_ENTRY_COUNT; i++)
    {
        if(OS_TblBlocked[i] != 0U)
        {
            workingSet = OS_TblBlocked[i];
            while(workingSet != 0U)
            {
                OS_tCPU_DATA task_pos = ((OS_CPU_WORD_SIZE_IN_BITS - (OS_tCPU_DATA)OS_CPU_CountLeadZeros(workingSet)) - 1U);
                OS_TASK_TCB* t = &OS_TblTask[task_pos];
                --(t->TASK_Ticks);
                if(0U == t->TASK_Ticks)
                {
                    /* Remove the task from the unblock table. */
                    OS_UnBlockTask(t->TASK_priority);
                    /*Add the current task to the ready table to be scheduled. */
                    OS_PrioritySet(t->TASK_priority);

                }
                /* Remove this processed bit and go to the next priority
                 * task in the same entry level. */
                workingSet &= ~(1U << task_pos);
            }
        }
    }
    /* Preempt Another Task ? */
    OS_Sched();

    OS_CRTICAL_END();
}

/*
 * Function:  OS_DelayTicks
 * --------------------
 * Block the current task execution for number of system ticks.
 *
 * Arguments:
 *            ticks     is the number of ticks for the task to be blocked.
 *
 * Returns: None.
 */
void
OS_DelayTicks (OS_t32U ticks)
{
    OS_CRTICAL_BEGIN();

    if(ticks == 0U)
    {
        OS_CRTICAL_END();
        return;
    }
    /* Don't allow blocking the ideal task. */
    if(OS_currentTask != &OS_TblTask[OS_IDLE_TASK_PRIO_LEVEL])
    {
        OS_currentTask->TASK_Ticks = ticks;
        /* Make the current task to be not scheduled. */
        OS_PriorityClear(OS_currentTask->TASK_priority);
        /* Put the current thread into a blocking state. */
        OS_BlockTask(OS_currentTask->TASK_priority);
        /* Preempt Another Task. */
        OS_Sched();
    }

    OS_CRTICAL_END();
}

/*
*******************************************************************************
*                                                                             *
*                           Task Management Functions                         *
*                                                                             *
*******************************************************************************
*/

/*
 * Function:  OS_PriorityHighestGet
 * --------------------
 * Get the highest priority of a task which is in a ready state.
 *
 * Arguments: None.
 *
 * Returns: The highest priority number.
 */
OS_tCPU_DATA inline
OS_PriorityHighestGet(void)
{
    OS_tCPU_DATA   *r_tbl;
    OS_tCPU_DATA    prio;

    prio  = 0U;
    r_tbl = &OS_TblReady[0];
    /* Loop Through Entries until find a non empty entry */
    while (*r_tbl == (OS_tCPU_DATA)0) {
        /* Advance by a Complete Entry */
        prio += OS_CPU_WORD_SIZE_IN_BITS;
        r_tbl++;
    }
    prio += ((OS_CPU_WORD_SIZE_IN_BITS - (OS_tCPU_DATA)OS_CPU_CountLeadZeros(*r_tbl)) - 1U);
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
void inline
OS_PrioritySet(OS_t32U prio)
{
    OS_tCPU_DATA bit_pos = prio & (OS_CPU_WORD_SIZE_IN_BITS - 1);
    OS_tCPU_DATA entry_pos = prio / OS_CPU_WORD_SIZE_IN_BITS; //TODO: Optimize it with bitwise operation.
    OS_TblReady[entry_pos] |= (1U << bit_pos);
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
void inline
OS_PriorityClear(OS_t32U prio)
{
    OS_tCPU_DATA bit_pos = prio & (OS_CPU_WORD_SIZE_IN_BITS - 1);
    OS_tCPU_DATA entry_pos = prio / OS_CPU_WORD_SIZE_IN_BITS; //TODO: Optimize it with bitwise operation.
    OS_TblReady[entry_pos] &= ~(1U << bit_pos);
}

/*
 * Function:  OS_BlockTask
 * --------------------
 * Put a task with a certain priority in the block state.
 *
 * Arguments:
 *          prio    is the task's priority.
 *
 * Returns: None
 */
void inline
OS_BlockTask(OS_t32U prio)
{
    OS_tCPU_DATA bit_pos = prio & (OS_CPU_WORD_SIZE_IN_BITS - 1);
    OS_tCPU_DATA entry_pos = prio / OS_CPU_WORD_SIZE_IN_BITS; //TODO: Optimize it with bitwise operation.
    OS_TblBlocked[entry_pos] |= (1U << bit_pos);
}

/*
 * Function:  OS_UnBlockTask
 * --------------------
 * Remove a task with a certain priority from the block state.
 *
 * Arguments:
 *          prio    is the task's priority.
 *
 * Returns: None
 */
void inline
OS_UnBlockTask(OS_t32U prio)
{
    OS_tCPU_DATA bit_pos = prio & (OS_CPU_WORD_SIZE_IN_BITS - 1);
    OS_tCPU_DATA entry_pos = prio / OS_CPU_WORD_SIZE_IN_BITS; //TODO: Optimize it with bitwise operation.
    OS_TblBlocked[entry_pos] &= ~(1U << bit_pos);
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
        thisTask = &OS_TblTask[priority];
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

