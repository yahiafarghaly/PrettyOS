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
 * Purpose  : Contains the implementation of various of OS_Task*() APIs.
 *
 * Language:  C
 * 
 * Set 1 tab = 4 spaces for better comments readability.
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"
#include "pretty_shared.h"

/*
*******************************************************************************
*                               static variables                              *
*******************************************************************************
*/

/* Array of TCBs, Each TCB Containing the task internal data.
 * This Table can be accessed by task priority. However, this is not the
 * right way. Instead you should you OS_tblTCBPrio[] pointers which will
 * point to the right TCB for the current execution.                         */
static OS_TASK_TCB OS_TblTask[OS_CONFIG_TASK_COUNT]      = { 0U };



/*
*******************************************************************************
*                               static functions                              *
*******************************************************************************
*/

/*
*******************************************************************************
*                             external functions                              *
*******************************************************************************
*/

/*
 * Function:  OS_Event_FreeListInit
 * --------------------
 * Initialize the memory pool of TCB entries to their default values
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Notes        :   1) This function for internal use.
 */
extern void
OS_TCB_ListInit (void)
{
    CPU_t32U idx;

    for(idx = 0; idx < OS_CONFIG_TASK_COUNT; ++idx)
    {
        OS_TblTask[idx].TASK_Stat   = OS_TASK_STAT_DELETED;
        OS_TblTask[idx].TASK_Ticks  = 0U;

#if (OS_AUTO_CONFIG_INCLUDE_EVENTS == OS_CONFIG_ENABLE)

        OS_TblTask[idx].TASK_Event    = OS_NULL(OS_EVENT);
        OS_TblTask[idx].OSTCB_NextPtr = OS_NULL(OS_TASK_TCB);

#endif

        OS_tblTCBPrio[idx]          = OS_NULL(OS_TASK_TCB);
    }
}

/*
*******************************************************************************
*                                                                             *
*                         PrettyOS Task functions                             *
*                                                                             *
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
OS_tRet
OS_TaskCreate (void (*TASK_Handler)(void* params),
                             void *params,
                             CPU_tSTK* pStackBase,
                             CPU_tSTK_SIZE  stackSize,
                             OS_PRIO priority)

{
    CPU_tWORD* stack_top;
    CPU_SR_ALLOC();

    if(TASK_Handler == OS_NULL(void) || pStackBase == OS_NULL(CPU_tWORD) ||
            stackSize == 0U )
    {
        return (OS_ERR_PARAM);
    }

    OS_CRTICAL_BEGIN();

    if(OS_IntNestingLvl > 0U)                                                     /* Don't Create a task from an ISR.                                                        */
    {
        OS_CRTICAL_END();
        return (OS_ERR_TASK_CREATE_ISR);
    }

    stack_top = OS_CPU_TaskStackInit(TASK_Handler, params, pStackBase, stackSize);     /* Call low level function to Initialize the stack frame of the task.                      */

    if(OS_IS_VALID_PRIO(priority))
    {
        OS_tblTCBPrio[priority] = &OS_TblTask[priority];

        if(OS_tblTCBPrio[priority]->TASK_Stat != OS_TASK_STAT_DELETED)            /* Check that the task is not in use.                                                      */
        {
            OS_CRTICAL_END();
            return (OS_ERR_TASK_CREATE_EXIST);
        }

        OS_tblTCBPrio[priority]->TASK_SP       = stack_top;
        OS_tblTCBPrio[priority]->TASK_priority = priority;
        OS_tblTCBPrio[priority]->TASK_Stat     = OS_TASK_STAT_READY;

#if (OS_AUTO_CONFIG_INCLUDE_EVENTS == OS_CONFIG_ENABLE)

        OS_tblTCBPrio[priority]->TASK_PendStat = OS_STAT_PEND_OK;
        OS_tblTCBPrio[priority]->OSTCB_NextPtr = OS_NULL(OS_TASK_TCB);
        OS_tblTCBPrio[priority]->TASK_Event    = OS_NULL(OS_EVENT);

#endif

#if (OS_CONFIG_TCB_TASK_ENTRY_STORE_EN == OS_CONFIG_ENABLE)
        OS_tblTCBPrio[priority]->TASK_EntryAddr = TASK_Handler;
        OS_tblTCBPrio[priority]->TASK_EntryArg  = params;
#endif

#if(OS_CONFIG_CPU_TASK_CREATED == OS_CONFIG_ENABLE)
        OS_CPU_Hook_TaskCreated (OS_tblTCBPrio[priority]);						 /* Call port specific task creation code.													  */
#endif

#if (OS_CONFIG_APP_TASK_CREATED == OS_CONFIG_ENABLE)
        App_Hook_TaskCreated (OS_tblTCBPrio[priority]);							 /* Calls Application specific code for a successfully created task.						  */
#endif

        OS_SetReady(priority);                                                   /* Put in ready state.                                                                       */
    }
    else
    {
        OS_CRTICAL_END();
        return (OS_ERR_PRIO_INVALID);
    }

    if(OS_TRUE == OS_Running)
    {
        OS_Sched();                                                             /* A higher priority task can be created inside another task. So, Schedule it immediately.    */
    }

    OS_CRTICAL_END();

    return (OS_ERR_NONE);
}

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
OS_tRet
OS_TaskDelete (OS_PRIO prio)
{
    OS_TASK_TCB* ptcb;
    CPU_SR_ALLOC();

    if(OS_IntNestingLvl > 0U)                                                      /* Don't delete from an ISR.                 */
    {
        return (OS_ERR_TASK_DELETE_ISR);
    }
    if(prio == OS_IDLE_TASK_PRIO_LEVEL)                                            /* Don't delete the Idle task.               */
    {
        return (OS_ERR_TASK_DELETE_IDLE);
    }
    if(!OS_IS_VALID_PRIO(prio))                                                    /* Valid priority ?                          */
    {
        return (OS_ERR_PRIO_INVALID);
    }

    OS_CRTICAL_BEGIN();

    ptcb = OS_tblTCBPrio[prio];

    if(ptcb == OS_NULL(OS_TASK_TCB) || ptcb->TASK_Stat == OS_TASK_STAT_DELETED)   /* Task must exist.                           */
    {
        OS_CRTICAL_END();
        return (OS_ERR_TASK_NOT_EXIST);
    }

    OS_RemoveReady(prio);                                                         /* Remove the task from ready state.          */

#if (OS_AUTO_CONFIG_INCLUDE_EVENTS == OS_CONFIG_ENABLE)

    if(ptcb->TASK_Event != ((OS_EVENT*)0U))                                       /* If it is waiting for any event...          */
    {
        OS_Event_TaskRemove(ptcb, ptcb->TASK_Event);                              /* ... unlink it.                             */
    }

#endif

    if(ptcb->TASK_Stat & OS_TASK_STAT_DELAY)                                      /* If it's waiting due to a delay             */
    {
        OS_UnBlockTime(prio);                                                     /* Remove from the time blocked state.        */
    }

    ptcb->TASK_Ticks    = 0U;                                                     /* Remove any remaining ticks.                */

#if (OS_AUTO_CONFIG_INCLUDE_EVENTS == OS_CONFIG_ENABLE)

    ptcb->TASK_PendStat = OS_STAT_PEND_OK;                                        /* Remove any pend state.                     */

#endif

    ptcb->TASK_Stat     = OS_TASK_STAT_DELETED;                                   /* Make the task be Dormant.                  */

#if(OS_CONFIG_CPU_TASK_DELETED == OS_CONFIG_ENABLE)
    OS_CPU_Hook_TaskDeleted (ptcb);												  /* Call port specific task deletion code.		*/
#endif

#if (OS_CONFIG_APP_TASK_DELETED == OS_CONFIG_ENABLE)
	App_Hook_TaskDeleted 	(ptcb);												  /* Calls Application specific code.			*/
#endif

    OS_tblTCBPrio[prio] = OS_NULL(OS_TASK_TCB);                                   /* The task is no longer exist. 				*/

    /* At this point, the task is prevented from resuming or made ready from another higher task or an ISR.                     */
    if(OS_TRUE == OS_Running)
    {
        OS_Sched();                                                               /* Schedule a new higher priority task.       */
    }

    OS_CRTICAL_END();

    return (OS_ERR_NONE);
}

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
OS_tRet
OS_TaskChangePriority (OS_PRIO oldPrio, OS_PRIO newPrio)
{
    OS_TASK_TCB* ptcb;
    OS_EVENT*    pevent;
    CPU_SR_ALLOC();

    if(oldPrio == newPrio)                                                    /* Don't waste more cycles.                     */
    {
        return (OS_ERR_PRIO_EXIST);
    }

    if(OS_IS_RESERVED_PRIO(oldPrio))                                          /* Don't Change an OS reserved priority.        */
    {
        return (OS_ERR_PRIO_EXIST);
    }

    if(OS_IS_RESERVED_PRIO(newPrio))                                         /* Don't Change to an OS reserved priority.      */
    {
        return (OS_ERR_PRIO_EXIST);
    }

    if(!OS_IS_VALID_PRIO(oldPrio) && !OS_IS_VALID_PRIO(newPrio))             /* Priority within our acceptable range.         */
    {
        return (OS_ERR_PRIO_INVALID);
    }

    OS_CRTICAL_BEGIN();

    if(OS_tblTCBPrio[oldPrio] == OS_NULL(OS_TASK_TCB))                       /* Check that the old task is Created.          */
    {
        OS_CRTICAL_END();
        return OS_ERR_TASK_NOT_EXIST;
    }

    if(OS_tblTCBPrio[oldPrio] == OS_TCB_MUTEX_RESERVED)
    {
        OS_CRTICAL_END();                                                  /* old prio should not be reserved for a mutex.   */
        return OS_ERR_TASK_NOT_EXIST;
    }

    if(OS_tblTCBPrio[oldPrio]->TASK_Stat == OS_TASK_STAT_DELETED)          /* Be dummy in the checks !                       */
    {
        OS_CRTICAL_END();
        return OS_ERR_TASK_NOT_EXIST;
    }

    if(OS_tblTCBPrio[newPrio] != OS_NULL(OS_TASK_TCB))                      /* The new priority must be available.           */
    {
        OS_CRTICAL_END();
        return OS_ERR_TASK_CREATE_EXIST;
    }

    if(OS_tblTCBPrio[newPrio] == OS_TCB_MUTEX_RESERVED)
    {
        OS_CRTICAL_END();                                                  /* new prio should not be reserved for a mutex.   */
        return OS_ERR_TASK_NOT_EXIST;
    }

    ptcb   = OS_tblTCBPrio[oldPrio];                                       /* Store a pointer to TCB entry at old priority.  */

#if (OS_AUTO_CONFIG_INCLUDE_EVENTS == OS_CONFIG_ENABLE)

    pevent = ptcb->TASK_Event;

#endif

    if(ptcb->TASK_Stat == OS_TASK_STAT_READY)
    {
        OS_RemoveReady(oldPrio);
        OS_SetReady(newPrio);
    }
    else
    {
        if(ptcb->TASK_Stat & OS_TASK_STAT_DELAY)
        {
            OS_UnBlockTime(oldPrio);
            OS_BlockTime(newPrio);
        }

#if (OS_AUTO_CONFIG_INCLUDE_EVENTS == OS_CONFIG_ENABLE)

        if(ptcb->TASK_Event != OS_NULL(OS_EVENT))                          /* If old priority is waiting for an event.        */
        {
            OS_Event_TaskRemove(ptcb, pevent);                             /* ... Remove at the old priority.                 */

            ptcb->TASK_priority    = newPrio;
            OS_Event_TaskInsert(OS_tblTCBPrio[newPrio], pevent);           /* ... Place event at the new priority.            */
        }

#endif

    }

    ptcb->TASK_priority    = newPrio;                                      /* Store new priority in TCB entry.                */
    OS_tblTCBPrio[oldPrio] = OS_NULL(OS_TASK_TCB);                         /* Unlink old priority pointer to TCB entry...     */
    OS_tblTCBPrio[newPrio] = ptcb;                                         /* ... Link to the new priority.                   */

    OS_CRTICAL_END();

    if(OS_TRUE == OS_Running)
    {
        OS_Sched();                                                      /* Call the scheduler, it may be a higher priority task.   */
    }

    return (OS_ERR_NONE);
}

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
OS_tRet
OS_TaskSuspend (OS_PRIO prio)
{
    CPU_tWORD       selfTask;
    OS_TASK_TCB*    thisTask;
    CPU_SR_ALLOC();

    if(OS_IDLE_TASK_PRIO_LEVEL == prio)                     /* Don't suspend idle task                                                 */
    {
        return (OS_ERR_TASK_SUSPEND_IDLE);
    }

    if(OS_IS_VALID_PRIO(prio))
    {
        OS_CRTICAL_BEGIN();

        if(prio == OS_currentTask->TASK_priority)           /* Is the caller task will be the suspended ?                              */
        {
            selfTask = OS_TRUE;
        }
        else
        {
            selfTask = OS_FAlSE;
        }

        thisTask = OS_tblTCBPrio[prio];

        if(thisTask == OS_NULL(OS_TASK_TCB) ||
          thisTask->TASK_Stat == OS_TASK_STAT_DELETED)     /* Check that the suspended task is actually exist.                         */
        {
            OS_CRTICAL_END();
            return (OS_ERR_TASK_SUSPEND_PRIO);
        }

        if(thisTask->TASK_Stat & OS_TASK_STAT_SUSPENDED)   /* If it's in a suspend state, why do extra work !                           */
        {
            OS_CRTICAL_END();
            return (OS_ERR_TASK_SUSPENDED);
        }

        thisTask->TASK_Stat |= OS_TASK_STAT_SUSPENDED;

        OS_RemoveReady(prio);

        OS_CRTICAL_END();

        if(selfTask == OS_TRUE)                            /* Calls the scheduler only if the task being suspended is the calling task. */
        {
            OS_Sched();
        }

        return OS_ERR_NONE;
    }

    return (OS_ERR_PRIO_INVALID);
}

/*
 * Function:  OS_TaskResume
 * ------------------------
 * Resume a suspended task given its priority.
 *
 * Arguments    :   prio  is the task priority.
 *
 * Returns      :   OS_RET_OK, OS_ERR_TASK_RESUME_PRIO, OS_ERR_PRIO_INVALID.
 */
OS_tRet
OS_TaskResume (OS_PRIO prio)
{
    OS_TASK_TCB*    thisTask;
    CPU_SR_ALLOC();

    if(OS_IDLE_TASK_PRIO_LEVEL == prio)                                             /* Resume an suspended task !                                                 */
    {
        return (OS_ERR_PRIO_INVALID);
    }

    if(OS_IS_VALID_PRIO(prio))
    {
        OS_CRTICAL_BEGIN();

        if(prio == OS_currentTask->TASK_priority)                                   /* Resume self !                                                              */
        {
            OS_CRTICAL_END();
            return (OS_ERR_TASK_RESUME_PRIO);
        }

        thisTask = OS_tblTCBPrio[prio];

        if(thisTask == OS_NULL(OS_TASK_TCB) ||
          thisTask->TASK_Stat == OS_TASK_STAT_DELETED)                              /* Check that the resumed task is actually exist.                             */
        {
            OS_CRTICAL_END();
            return (OS_ERR_TASK_RESUME_PRIO);
        }

        if((thisTask->TASK_Stat & OS_TASK_STAT_SUSPENDED) != OS_TASK_STAT_READY)    /* Check it's already in suspend state and not in ready state.                */
        {
            thisTask->TASK_Stat &= ~(OS_TASK_STAT_SUSPENDED);                       /* Clear the suspend state.                                                   */
           if((thisTask->TASK_Stat & OS_TASK_STATE_PEND_ANY) == OS_TASK_STAT_READY) /* If it's not pending on any events ... */
           {
               if(thisTask->TASK_Ticks == 0U)                                       /* If it's not waiting a delay ...                                            */
               {
                   OS_SetReady(prio);
                   OS_CRTICAL_END();
                   if(OS_TRUE == OS_Running)
                   {
                       OS_Sched();                                                  /* Call the scheduler, it may be a higher priority task.                      */
                   }
               }
           }
        }

        OS_CRTICAL_END();

        return (OS_ERR_NONE);
    }

    return (OS_ERR_PRIO_INVALID);
}

/*
 * Function:  OS_TaskStatus
 * --------------------
 * Return Task Status.
 *
 * Arguments    :   prio  is the task priority.
 *
 * Returns      :   OS_STATUS
 */
OS_STATUS inline
OS_TaskStatus (OS_PRIO prio)
{
    return (OS_TblTask[prio].TASK_Stat);
}

/*
 * Function:  OS_TaskReturn
 * --------------------
 * This function should be called if a task is accidentally returns without deleting itself.
 * The address of the function should be set at the task stack register of the return address.
 *
 * Arguments    :   None.
 *
 * Returns      :   None.
 *
 * Note(s)      :   1) This function is for internal use and the application should never called it.
 */
void
OS_TaskReturn (void)
{

#if (OS_CONFIG_APP_TASK_RETURNED == OS_CONFIG_ENABLE)
	App_Hook_TaskReturned (OS_currentTask); 			/* Calls Application specific code when a task returns intentionally. 								*/
#endif

    OS_TaskDelete(OS_currentTask->TASK_priority);       /* Delete a task.            																		*/
    for(;;)                                             /* If deletion fails, Loop Every OS_CONFIG_TICKS_PER_SEC											*/
    {
        OS_DelayTicks(OS_CONFIG_TICKS_PER_SEC);
    }
}

