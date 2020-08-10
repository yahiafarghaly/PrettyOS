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
 * Purpose  :	Semaphores Service Implementation.
 *
 * 				Semaphores are one of the oldest mechanisms introduced by multitasking systems,
 * 				 being used both for managing common resources and synchronization.
 *
 * 				 For Task Synchronization, there are two types:
 * 				 	1- Unilateral Rendezvous
 *						This is one way synchronization which uses a semaphore as a flag to signal another task.
 *					2- Bilateral Rendezvous
 *						This is two way synchronization performed using two semaphores. A bilateral rendezvous is similar
 *						to a unilateral rendezvous, except both tasks must synchronize with one another before proceeding.
 *
 *					This type of synchronization can be performed using a binary semaphores which its count is equal to 0 or 1.
 *
 *				The kernel supports counting semaphores which the semaphore would accumulate events that have not yet been processed.
 *				this means that more than one task can be waiting for an event to occur.
 *				In this case, the kernel could signal the occurrence of the event to the highest priority task waiting for the event to occur.
 *
 *
 *				[ Rule ]:	A task can create/post/delete or pend/acquire the semaphore.
 *							An ISR can only post a semaphore value.
 *
 * 				Your application can have any number of semaphores. The limit is set by OS_CONFIG_MAX_EVENTS .
 *
 * Language:  C
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"
#include "pretty_shared.h"

#if (OS_CONFIG_SEMAPHORE_EN == OS_CONFIG_ENABLE)

/*
 * Return the Max Semaphore Count depending on the predefined data type (OS_SEM_COUNT).
 * With The Compiler Optimization, This can be replaced with a constant value without the function call.
 */
static const unsigned long OS_SemMaxCount (void)
{
	switch(sizeof(OS_SEM_COUNT))
	{
	case 1:
		return	0xFF;
	case 2:
		return 0x00FF;
	case 4:
		return 0x0000FFFFFFFF;
	case 8:
		return 0xFFFFFFFFFFFFFFFF;
	default:
		break;
	}
	return 0x0000FFFFFFFF;
}

/*
*******************************************************************************
*                                Semaphore functions                          *
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
 * Returns      :  != (OS_SEM*)0U  is a pointer to OS_EVENT object of type OS_EVENT_TYPE_SEM associated with the created semaphore.
 *                 == (OS_SEM*)0U  if no events object were available.
 *
 *                 OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_POOL_EMPTY, OS_ERR_EVENT_CREATE_ISR }
 *
 * Notes        :   1) This function must used only from Task code level and not an ISR.
 */
OS_SEM*
OS_SemCreate (OS_SEM_COUNT cnt)
{
    OS_EVENT* pevent;
    CPU_SR_ALLOC();

    if(OS_IntNestingLvl > 0U)                           /* Create only from task level.                      */
    {
        OS_ERR_SET(OS_ERR_EVENT_CREATE_ISR);
        return OS_NULL(OS_EVENT);
    }

    OS_CRTICAL_BEGIN();

    OS_EVENT_allocate(&pevent);                         /* Allocate an event object.                         */

    OS_CRTICAL_END();

    if(pevent == OS_NULL(OS_EVENT))
    {
    	OS_ERR_SET(OS_ERR_EVENT_POOL_EMPTY);
        return (pevent);
    }

    pevent->OSEventType     = OS_EVENT_TYPE_SEM;
    pevent->OSEventPtr      = OS_NULL(OS_EVENT);       /* Unlink from the free list of queue events.         */
    pevent->OSEventCount    = cnt;
    pevent->OSEventsTCBHead = OS_NULL(OS_TASK_TCB);    /* Initialize that no tasks waiting on this semaphore.*/

    OS_ERR_SET(OS_ERR_NONE);
    return (pevent);
}

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
void
OS_SemPend (OS_SEM* pevent, OS_TICK timeout)
{
    CPU_SR_ALLOC();

    if (pevent == OS_NULL(OS_EVENT)) {                     /* Validate 'pevent'                                         */
        OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
        return;
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {        /* Validate event type                                       */
        OS_ERR_SET(OS_ERR_EVENT_TYPE);
        return;
    }

    if (OS_IntNestingLvl > 0U) {
        OS_ERR_SET(OS_ERR_EVENT_PEND_ISR);                 /* Doesn't make sense to wait inside an ISR.                 */
        return;
    }

    if (OS_LockSchedNesting > 0U) {
    	OS_ERR_SET(OS_ERR_EVENT_PEND_LOCKED);              /* Should not wait when scheduler is locked.                 */
    	return;
    }

    OS_CRTICAL_BEGIN();

    if(pevent->OSEventCount > 0U)                           /* If semaphore resource is available ...                    */
    {
        (pevent->OSEventCount)--;                           /* ... decrement it ...                                      */
        OS_CRTICAL_END();
        OS_ERR_SET(OS_ERR_NONE);
        return;												/* ... and return.                                           */
    }

    OS_currentTask->TASK_Stat |= OS_TASK_STATE_PEND_SEM;    /* Otherwise, pend on semaphore and wait for event to occur. */
    OS_currentTask->TASK_PendStat = OS_STAT_PEND_OK;
    OS_currentTask->TASK_Ticks = timeout;

    if(timeout > 0U)
    {
        OS_BlockTime(OS_currentTask->TASK_priority);
        OS_currentTask->TASK_Stat |= OS_TASK_STAT_DELAY;
    }

    OS_Event_TaskPend(pevent);                              /* Place the current TCB in the pending list.                */

    OS_CRTICAL_END();

    OS_Sched();                                             /* Preempt another task.                                     */

    OS_CRTICAL_BEGIN();                                     /* We're back again ...                                      */

    switch (OS_currentTask->TASK_PendStat) {                /* ... See if it was timed-out or aborted.                   */
        case OS_STAT_PEND_OK:
        	OS_ERR_SET(OS_ERR_NONE);                        /* Indicate that the task owns the resource.                 */
            break;

        case OS_STAT_PEND_ABORT:
        	OS_ERR_SET(OS_ERR_EVENT_PEND_ABORT);            /* Indicate that we aborted.                                 */
            break;

        case OS_STAT_PEND_TIMEOUT:
        default:
            OS_Event_TaskRemove(OS_currentTask, pevent);
            OS_ERR_SET(OS_ERR_EVENT_TIMEOUT);				/* Indicate that we didn't get event within Time out.        */
            break;
    }

    OS_currentTask->TASK_Stat     &= ~(OS_TASK_STATE_PEND_SEM);
    OS_currentTask->TASK_PendStat  =  OS_STAT_PEND_OK;
    OS_currentTask->TASK_Event     = OS_NULL(OS_EVENT);     /* Unlink the event from the current TCB.                    */

    OS_CRTICAL_END();

    return;
}

/*
 * Function:  OS_SemPost
 * --------------------
 * Signal a semaphore.
 *
 * Arguments    :   pevent      is a pointer to the OS_EVENT object associated with the semaphore.
 *
 * Returns      :   OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_SEM_OVERFLOW }
 *
 * Notes        :   1) This function can be called from a task code or an ISR.
 */
void
OS_SemPost (OS_SEM* pevent)
{
    CPU_SR_ALLOC();

    if (pevent == OS_NULL(OS_EVENT)) {                      /* Validate 'pevent'                                          */
         OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
         return;
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {         /* Validate event type                                        */
    	OS_ERR_SET(OS_ERR_EVENT_TYPE);
    	return;
    }

    OS_CRTICAL_BEGIN();

    if (pevent->OSEventsTCBHead != ((OS_TASK_TCB*)0U)) {    /* See if any task waiting for semaphore.                     */
        OS_Event_TaskMakeReady(pevent, (void *)0,           /* Make Highest priority task waiting on event be ready.      */
                            OS_TASK_STATE_PEND_SEM,
                            OS_STAT_PEND_OK);               /* OS_STAT_PEND_OK indicates a post operation.                */
        OS_CRTICAL_END();
        /* We don't need to increment the semaphore count here, since it's emulated by the design as this task is
         * preempted to the highest priority waiting task which takes the resource (decrement it again).
         * Also, this prevent other tasks (preempted to others than the one who waiting for the event) from
         * owning the resource.
         * On the other side, the pend() function is not performing any decrement operation if it is pended. */
        OS_Sched();                                         /* Call the scheduler, it may be the highest.                 */
        OS_ERR_SET(OS_ERR_NONE);
        return;
    }

    if(pevent->OSEventCount < (OS_SEM_COUNT)OS_SemMaxCount())
    {
        (pevent->OSEventCount)++;
        OS_CRTICAL_END();
        OS_ERR_SET(OS_ERR_NONE);
        return;
    }

    OS_ERR_SET(OS_ERR_SEM_OVERFLOW);						/* The semaphore count has reached its maximum.				  */
    return;
}

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
OS_SEM_COUNT
OS_SemPendNonBlocking(OS_SEM* pevent)
{
    OS_SEM_COUNT count;
    CPU_SR_ALLOC();

    if (pevent == OS_NULL(OS_EVENT)) {                      /* Validate 'pevent'                                            */
    	OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
    	return (0U);
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {         /* Validate event type                                          */
        OS_ERR_SET(OS_ERR_EVENT_TYPE);
    	return (0U);
    }

    OS_CRTICAL_BEGIN();

    count = pevent->OSEventCount;                           /* The available count.                                         */
    if(count > 0U)
    {
        (pevent->OSEventCount)--;                           /* The calling Task/ISR owns a semaphore resource.              */
    }

    OS_CRTICAL_END();

    OS_ERR_SET(OS_ERR_NONE);
    return (count);
}

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
void
OS_SemPendAbort(OS_SEM* pevent, CPU_t08U opt, OS_TASK_COUNT* abortedTasksCount)
{
    OS_TASK_COUNT nAbortedTasks;
    CPU_SR_ALLOC();

    if (pevent == (OS_EVENT*)0U) {                          /* Validate 'pevent'                                         */
         OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
         return;
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {         /* Validate event type                                       */
    	OS_ERR_SET(OS_ERR_EVENT_TYPE);
    	return;
    }

    OS_CRTICAL_BEGIN();

    if (pevent->OSEventsTCBHead != OS_NULL(OS_TASK_TCB)) {  /* See if any task waiting for semaphore.                    */
        nAbortedTasks = 0U;
        switch(opt)
        {
        case OS_SEM_ABORT_ALL:
            while(pevent->OSEventsTCBHead != OS_NULL(OS_TASK_TCB))
            {
                OS_Event_TaskMakeReady(pevent, (void *)0,
                                    OS_TASK_STATE_PEND_SEM,
                                    OS_STAT_PEND_ABORT);    /* OS_STAT_PEND_ABORT indicates abort operation.               */
                ++nAbortedTasks;
            }
            break;
        case OS_OPT_DEFAULT & OS_SEM_ABORT_HPT:
        default:
            OS_Event_TaskMakeReady(pevent, (void *)0,
                                OS_TASK_STATE_PEND_SEM,
                                OS_STAT_PEND_ABORT);
            ++nAbortedTasks;
            break;
        }

        OS_CRTICAL_END();

        OS_Sched();                                        /* Preempt HPT that no longer wait for an event ?              */

        if(abortedTasksCount != OS_NULL(OS_TASK_COUNT))
        {
            *abortedTasksCount = nAbortedTasks;
        }
        OS_ERR_SET(OS_ERR_EVENT_PEND_ABORT);               /* Indicate that we aborted.                                    */
        return;
    }

    OS_CRTICAL_END();

    if(abortedTasksCount != OS_NULL(OS_TASK_COUNT))        /* No waiting tasks to abort.                                    */
    {
        *abortedTasksCount = 0U;
    }

    OS_ERR_SET(OS_ERR_NONE);
    return;
}

#endif 		/* OS_CONFIG_SEMAPHORE_EN */
