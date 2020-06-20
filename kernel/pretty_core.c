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
/* Interrupt nesting level. */
OS_t8U  OS_IntNestingLvl;
/* Scheduler nesting lock level. */
OS_t8U  OS_LockSchedNesting;

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
static void OS_ScheduleHighest(void);
static void OS_Sched(void);
static OS_tRet OS_TCB_RegisterTask(OS_tptr* stackTop,OS_tCPU_DATA priority);
static OS_tCPU_DATA OS_PriorityHighestGet(void);
static void OS_SetReady(OS_t32U prio);
static void OS_RemoveReady(OS_t32U prio);
static void OS_BlockTask(OS_t32U prio);
static void OS_UnBlockTask(OS_t32U prio);
static inline OS_tCPU_DATA OS_Log2(const OS_tCPU_DATA x);


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
 *          OS_ERR_PARAM            Invalid supplied parameter.
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
    OS_IntNestingLvl = 0U;
    OS_LockSchedNesting = 0U;
    OS_Running = OS_FAlSE;
    return (ret);
}

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
void
OS_IntEnter(void)
{
    if(OS_TRUE == OS_Running)
    {
        if(OS_IntNestingLvl < 255U)
        {
            ++OS_IntNestingLvl;
        }
    }
}

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
void
OS_IntExit(void)
{
    if(OS_TRUE == OS_Running)                           /* The kernel has already started.                            */
    {
        OS_CRTICAL_BEGIN();
        if(OS_IntNestingLvl > 0U)                       /* Prevent OS_IntNestingLvl from wrapping                     */
        {
            --OS_IntNestingLvl;
        }

        if(0U == OS_IntNestingLvl)                      /* Re-schedule if all ISRs are completed...                    */
        {
            if(0U == OS_LockSchedNesting)               /* ... and not locked                                          */
            {
                OS_ScheduleHighest();                   /* Determine the next high task to run.                        */
                if(OS_nextTask != OS_currentTask)       /* No context switch if the current task is the highest.       */
                {
                    OS_CPU_InterruptContexSwitch();     /* Perform a CPU specific code for interrupt context switch.   */
                }
            }
        }

        OS_CRTICAL_END();
    }
}

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
void
OS_SchedLock(void)
{
    if(OS_TRUE == OS_Running)
    {
        OS_CRTICAL_BEGIN();
        if(0U == OS_IntNestingLvl)                     /* Don't call from an ISR                             */
        {
            if (OS_LockSchedNesting < 255U) {          /* Prevent wrapping back to 0                         */
                ++OS_LockSchedNesting;                 /* Increment lock nesting level                       */
            }
        }
        OS_CRTICAL_END();
    }
}

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
void
OS_SchedUnlock(void)
{
    if(OS_TRUE == OS_Running)
    {
        OS_CRTICAL_BEGIN();
        if(0U == OS_IntNestingLvl)                     /* Don't call from an ISR                             */
        {
            if(OS_LockSchedNesting > 0U)               /* Don't decrement if it's 0                          */
            {
                --OS_LockSchedNesting;                 /* Decrement lock nesting level                       */
                if(0U == OS_LockSchedNesting)          /* Call the scheduler if lock reached to 0            */
                {
                    OS_CRTICAL_END();
                    OS_Sched();
                }
            }
        }
        OS_CRTICAL_END();
    }
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
 *          priority                is the task priority. ( A unique priority must be assigned to each task )
 *                                      - A greater number means a higher priority
 *                                      - 0 => is reserved for the OS' Idle Task.
 *                                      - OS_LOWEST_PRIO_LEVEL(0) < Allowed value <= OS_HIGHEST_PRIO_LEVEL
 *
 * Returns: OS_RET_OK                       if successful operation is done.
 *          OS_ERR_PARAM                    Invalid supplied parameter.
 *          OS_RET_ERROR_TASK_CREATE_ISR    If a task is created inside an ISR.
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
        return (OS_ERR_PARAM);
    }

    OS_CRTICAL_BEGIN();
    if(OS_IntNestingLvl > 0U)       /* Don't Create a task from an ISR.  */
    {
        OS_CRTICAL_END();
        return (OS_ERR_TASK_CREATE_ISR);
    }

    /* Call the low level function to initialize the stack frame of the task. */
    stack_top = OS_CPU_TaskInit(TASK_Handler, params, pStackBase, stackSize);
    ret = OS_TCB_RegisterTask((OS_tptr*)stack_top,priority);
    if(OS_RET_OK == ret)
    {
        if(OS_TRUE == OS_Running)
        {
            OS_Sched(); /* A higher priority task can be created inside another task. So, Schedule it immediately. */
        }
    }

    OS_CRTICAL_END();

    return (ret);
}

OS_tRet
OS_ChangeTaskPriority(OS_tCPU_DATA oldPrio, OS_tCPU_DATA newPrio)
{
    if(oldPrio == newPrio)                          /* Don't waste more cycles.                     */
    {
        return (OS_ERR_PRIO_EXIST);
    }

    if(OS_IDLE_TASK_PRIO_LEVEL == oldPrio)          /* Don't Change IdleTask priority.              */
    {
        return (OS_ERR_PRIO_INVALID);
    }

    if(OS_IDLE_TASK_PRIO_LEVEL == newPrio)          /* Don't Change to the IdleTask priority.       */
    {
        return (OS_ERR_PRIO_EXIST);
    }

    if(OS_IS_VALID_PRIO(newPrio))                   /* Priority within our acceptable range.        */
    {
        OS_CRTICAL_BEGIN();
        if((OS_TASK_TCB)0U != OS_TblTask[newPrio])  /* New priority must not be exist.              */
        {
            OS_CRTICAL_END();
            return OS_ERR_PRIO_EXIST;
        }

        OS_RemoveReady(oldPrio);
        OS_TblTask[newPrio].TASK_SP         = OS_TblTask[oldPrio].TASK_SP;
        OS_TblTask[newPrio].TASK_Ticks      = OS_TblTask[oldPrio].TASK_Ticks;
        OS_TblTask[newPrio].TASK_priority   = newPrio;
        /* NOT COMPLETE IMPLEMENATION   */

        OS_CRTICAL_END();
    }
    else
    {
        return (OS_ERR_PRIO_INVALID);
    }
}

/*
 * Function:  OS_Sched
 * --------------------
 * Determine the next highest priority task that is ready to run,
 * And perform a task context switch if needed.
 * This function is invoked by a TASK level code and is not used to re-schedule tasks from ISRs.
 * (see OS_IntExit() for ISR rescheduling).
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        : 1) This function is internal to PrettyOS functions.
 */
void
OS_Sched(void)
{
    OS_CRTICAL_BEGIN();

    if(0U == OS_IntNestingLvl)                      /* Re-schedule if all ISRs are completed.                      */
    {
        if(0U == OS_LockSchedNesting)               /* Re-schedule if it's not locked.                             */
        {
            OS_ScheduleHighest();                   /* Determine the next high task to run.                        */
            if(OS_nextTask != OS_currentTask)       /* No context switch if the current task is the highest.       */
            {
                OS_CPU_ContexSwitch();              /* Perform a CPU specific code for task context switch.        */
            }
        }
    }

    OS_CRTICAL_END();
}

/*
 * Function:  OS_ScheduleHighest
 * --------------------
 * Determine the next highest priority task that is ready to run.
 * The global variable `OS_nextTask` is changed accordingly.
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        : 1) Interrupts are assumed to be disabled.
 *                2) This function is internal to PrettyOS functions.
 */
void
OS_ScheduleHighest(void)
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
        OS_ScheduleHighest();
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
 * Signal the occurrence of a "system tick" to the prettyOS which reflects
 * to the services depending on this "system tick".
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        : 1) This function must be called from a ticker ISR.
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
                OS_TASK_TCB* t = &OS_TblTask[task_pos + (i * OS_CPU_WORD_SIZE_IN_BITS) ];
                --(t->TASK_Ticks);
                if(0U == t->TASK_Ticks)
                {
                    /* Remove the task from the unblock table. */
                    OS_UnBlockTask(t->TASK_priority);
                    /*Add the current task to the ready table to be scheduled. */
                    OS_SetReady(t->TASK_priority);

                }
                /* Remove this processed bit and go to the next priority
                 * task in the same entry level. */
                workingSet &= ~(1U << task_pos);
            }
        }
    }

    OS_CRTICAL_END();
}

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
        OS_RemoveReady(OS_currentTask->TASK_priority);
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

    prio  = (OS_CPU_WORD_SIZE_IN_BITS*OS_CONFIG_PRIORTY_ENTRY_COUNT);
    r_tbl = &OS_TblReady[OS_CONFIG_PRIORTY_ENTRY_COUNT - 1U];
    /* Loop Through Entries until find a non empty entry */
    while (*r_tbl == (OS_tCPU_DATA)0) {
        /* Go Back by a Complete Entry */
        prio -= OS_CPU_WORD_SIZE_IN_BITS;
        r_tbl = r_tbl - 1;
    }
    prio -= OS_CPU_WORD_SIZE_IN_BITS;
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
OS_SetReady(OS_t32U prio)
{
    OS_tCPU_DATA bit_pos = prio & (OS_CPU_WORD_SIZE_IN_BITS - 1);
    OS_tCPU_DATA entry_pos = prio >> OS_Log2(OS_CPU_WORD_SIZE_IN_BITS);
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
OS_RemoveReady(OS_t32U prio)
{
    OS_tCPU_DATA bit_pos = prio & (OS_CPU_WORD_SIZE_IN_BITS - 1);
    OS_tCPU_DATA entry_pos = prio >> OS_Log2(OS_CPU_WORD_SIZE_IN_BITS);
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
    OS_tCPU_DATA entry_pos = prio >> OS_Log2(OS_CPU_WORD_SIZE_IN_BITS);
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
    OS_tCPU_DATA entry_pos = prio >> OS_Log2(OS_CPU_WORD_SIZE_IN_BITS);
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
        return (OS_ERR_PARAM);
    }

    if(OS_CPU_likely(OS_IS_VALID_PRIO(priority)))
    {
        thisTask = &OS_TblTask[priority];
        thisTask->TASK_SP = stackTop;
        thisTask->TASK_priority = priority;
        OS_SetReady(priority);
        ret = OS_RET_OK;
    }
    else
    {
        ret = OS_ERR_PRIO_INVALID;
    }

    return (ret);
}

/*
 * Function:  OS_Log2
 * --------------------
 * Compute the logarithmic number of base 2.
 * Log(x) = 2^k, This function returns `k` for few numbers of x ( 128, 64, 32, 16, 8, 4, 2 and 1 ).
 *
 * Arguments:
 *          x   is the number raised by the number the function returns.
 *
 * Returns: The raised number. (i.e: k)
 */
static inline OS_tCPU_DATA
OS_Log2(const OS_tCPU_DATA x)
{
    switch(x)
    {
    case 128U:
        return 7;
    case 64U:
        return 6;
    case 32U:
        return 5;
    case 16U:
        return 4;
    case 8U:
        return 3;
    case 4U:
        return 2;
    case 2U:
        return 1;
    default:
        break;
    }
    return 0;
}

