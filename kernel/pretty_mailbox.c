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
 * Purpose  :	Mail box service implementation.
 *
 * 				A message mailbox, a mailbox, single queue entry or exchanges are common names for one
 * 				of the simplest methods of inter-task-communication.
 * 				A mailbox allows passing messages between tasks via a pointer sized variable which is
 * 				typically initialized to point to some application specific data structure containing the message.
 *
 * 				[A Rule of Thumb]: A task can send or receive a message. But ISR can only send.
 *
 * 				Your application can have any number of mailboxes. The limit is set by OS_CONFIG_MAX_EVENTS .
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

#if (OS_CONFIG_MAILBOX_EN == OS_CONFIG_ENABLE)


/*
*******************************************************************************
*                              Mail box functions                             *
*******************************************************************************
*/

/*
 * Function:  OS_MutexCreate
 * --------------------
 * Creates a message mailbox container.
 *
 * Arguments    :   p_message    is a pointer sized variable which points to the message you desire to deposit at the creation
 * 								 of the mailbox.
 *
 * 								 If you set p_message to ((void*)0) (i.e NULL pointer) then the mailbox will be considered empty.
 * 								 otherwise, it is Full.
 *
 * Returns      :  != (OS_MAILBOX*)0U  is a pointer to OS_EVENT object of type OS_EVENT_TYPE_MAILBOX associated with the created mailbox.
 *                 == (OS_MAILBOX*)0U  if no events object were available.
 *
 *                 OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_POOL_EMPTY, OS_ERR_EVENT_CREATE_ISR }
 *
 * Note(s)      :   1) This function is used only from a Task code level.
 */
OS_MAILBOX*
OS_MailBoxCreate (void* p_message)
{
	OS_EVENT* pevent;
	CPU_SR_ALLOC();

    if (OS_IntNestingLvl > 0U) {                       	 	/* Don't Create from an ISR.                                	*/
        OS_ERR_SET(OS_ERR_EVENT_CREATE_ISR);
        return OS_NULL(OS_EVENT);
    }

    OS_CRTICAL_BEGIN();

    OS_EVENT_allocate(&pevent);                         	/* Allocate an event object.                         			*/

    OS_CRTICAL_END();

    if(pevent == OS_NULL(OS_EVENT))
    {
        OS_ERR_SET(OS_ERR_EVENT_POOL_EMPTY);
        return OS_NULL(OS_EVENT);
    }

    pevent->OSEventType      = OS_EVENT_TYPE_MAILBOX;    	/* Store the Event type.                                     	*/
    pevent->OSEventPtr       = (OS_EVENT*)p_message;     	/* Store the initial message.									*/
    pevent->OSEventsTCBHead  = OS_NULL(OS_TASK_TCB);    	/* Initial, No tasks are pended on this event.               	*/
    pevent->OSEventCount	 = 0U;             				/* Clear the rest of event structure.	                     	*/

    OS_ERR_SET(OS_ERR_NONE);
    return (pevent);										/* Return the mailbox event object.								*/
}


/*
 * Function:  OS_MailBoxPend
 * --------------------
 * Waits for a message arrival or within a finite time if 'timeout' is set.
 *
 * Arguments    :   pevent    	is a pointer to an OS_EVENT object associated with a mailbox object.
 *
 *                  timeout     is an optional timeout period (in clock ticks).  If non-zero, your task will
 *                              wait for message arrival up to the amount of time specified by this argument.
 *                              If you specify 0, however, your task will wait forever at the specified
 *                              mailbox or, until a messages arrives.
 *
 * Returns      :  	!= (void*)0 is a pointer to the message which is received.
 * 					== (void*)0 If no message is received or 'pevent' is a NULL pointer.
 *
 * 					OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL,OS_ERR_EVENT_TYPE, OS_ERR_EVENT_PEND_ISR
 * 								 OS_ERR_EVENT_PEND_LOCKED, OS_ERR_EVENT_PEND_ABORT, OS_ERR_EVENT_TIMEOUT }
 *
 * Note(s)      :   1) This function is used only from a Task code level.
 */
void*
OS_MailBoxPend (OS_MAILBOX* pevent, OS_TICK timeout)
{

	void* p_message;
    CPU_SR_ALLOC();

    if (pevent == OS_NULL(OS_EVENT)) {                      /* Validate 'pevent'                                         */
        OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
        return OS_NULL(void);
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_MAILBOX) {     /* Validate event type                                       */
        OS_ERR_SET(OS_ERR_EVENT_TYPE);
        return OS_NULL(void);
    }

    if (OS_IntNestingLvl > 0U) {
        OS_ERR_SET(OS_ERR_EVENT_PEND_ISR);                  /* Cannot pend inside an ISR.                 				 */
        return OS_NULL(void);
    }

    if (OS_LockSchedNesting > 0U) {
        OS_ERR_SET(OS_ERR_EVENT_PEND_LOCKED);               /* Cannot pend while scheduler is locked.                 	*/
        return OS_NULL(void);
    }

    OS_CRTICAL_BEGIN();

    p_message = (void*)pevent->OSEventPtr;					/* Read mailbox ...											*/

    if(p_message != OS_NULL(void))							/* Is mailbox full ? 										*/
    {
    	pevent->OSEventPtr = OS_NULL(void);					/* Yes ... Clear the mailbox.								*/
    	OS_CRTICAL_END();
    	OS_ERR_SET(OS_ERR_NONE);
    	return (p_message);									/* Return the received message.								*/
    }

    OS_currentTask->TASK_Stat |= OS_TASK_STATE_PEND_MAILBOX;/* Otherwise, pend on message arrival or timeout expires.	*/
    OS_currentTask->TASK_PendStat = OS_STAT_PEND_OK;
    OS_currentTask->TASK_Ticks = timeout;

    if(timeout > 0U)
    {
        OS_BlockTime(OS_currentTask->TASK_priority);
        OS_currentTask->TASK_Stat |= OS_TASK_STAT_DELAY;
    }

    OS_Event_TaskPend(pevent);								/* Suspend Current task till event occurs or timeout.		*/

    OS_CRTICAL_END();

    OS_Sched();												/* Preempt another task.                                     */

    OS_CRTICAL_BEGIN();                                     /* We're back again ...                                      */

    switch (OS_currentTask->TASK_PendStat) {                /* ... See if it was timed-out or aborted.                   */
        case OS_STAT_PEND_OK:
        	p_message = pevent->OSEventPtr;					/* Read the message.	 									 */
        	pevent->OSEventPtr	= OS_NULL(void);			/* Clear the mailbox.										 */
        	OS_ERR_SET(OS_ERR_NONE);
            break;

        case OS_STAT_PEND_ABORT:
        	p_message = OS_NULL(void);						/* An empty message.										 */
        	OS_ERR_SET(OS_ERR_EVENT_PEND_ABORT);            /* Indicate that we aborted.                                 */
            break;

        case OS_STAT_PEND_TIMEOUT:
        default:
            OS_Event_TaskRemove(OS_currentTask, pevent);
        	p_message = OS_NULL(void);						/* An empty message.										 */
            OS_ERR_SET(OS_ERR_EVENT_TIMEOUT);               /* Indicate that we didn't get the message within timeout.   */
            break;
    }

    OS_currentTask->TASK_Stat     &= ~(OS_TASK_STATE_PEND_MAILBOX);
    OS_currentTask->TASK_PendStat  =  OS_STAT_PEND_OK;
    OS_currentTask->TASK_Event     = OS_NULL(OS_EVENT);     /* Unlink the event from the current TCB.                    */

    OS_CRTICAL_END();

	return (p_message);										/* Return the received message.								*/
}


/*
 * Function:  OS_MailBoxPost
 * --------------------
 * Sends a message to a mailbox.
 *
 * Arguments    :   pevent    	is a pointer to an OS_EVENT object associated with a mailbox object.
 *
 * 					p_message	is a pointer to a message to send.
 * 								If it's NULL, then you're posting nothing. This will return with an error.
 *
 * Returns      :  	OS_ERRNO = { OS_ERR_NONE, OS_ERR_EVENT_PEVENT_NULL, OS_ERR_EVENT_TYPE, OS_ERR_MAILBOX_POST_NULL, OS_ERR_MAILBOX_FULL }
 *
 * Note(s)      :   1) This function can be used from a Task code level or an ISR.
 */
void
OS_MailBoxPost (OS_MAILBOX* pevent, void* p_message)
{
    CPU_SR_ALLOC();

    if (pevent == OS_NULL(OS_EVENT)) {                       /* Validate 'pevent'                                         */
         OS_ERR_SET(OS_ERR_EVENT_PEVENT_NULL);
         return;
    }

    if (pevent->OSEventType != OS_EVENT_TYPE_MAILBOX) {   	 /* Validate event type                                       */
    	OS_ERR_SET(OS_ERR_EVENT_TYPE);
    	return;
    }

    if (p_message == OS_NULL(void))							 /* Don't post a NULL message. 								  */
    {
    	OS_ERR_SET(OS_ERR_MAILBOX_POST_NULL);
    	return;
    }

    OS_CRTICAL_BEGIN();

    if (pevent->OSEventsTCBHead != OS_NULL(OS_TASK_TCB)) {   /* See if any task waiting for a message.                    */
         OS_Event_TaskMakeReady(pevent, p_message,           /* Make Highest priority task waiting on event be ready.     */
                             OS_TASK_STATE_PEND_MAILBOX,
                             OS_STAT_PEND_OK);               /* OS_STAT_PEND_OK indicates a post operation.               */
         OS_CRTICAL_END();

         OS_Sched();                                         /* Call the scheduler, it may be the highest.                */

         OS_ERR_SET(OS_ERR_NONE);
         return;
     }

    if(pevent->OSEventPtr != OS_NULL(void))					  /* Is mailbox full ? 										  */
    {
    	OS_CRTICAL_END();									  /* Yes, ... leave it.								  		  */
    	OS_ERR_SET(OS_ERR_MAILBOX_FULL);
    	return;
    }

     OS_CRTICAL_END();

     pevent->OSEventPtr	= (OS_EVENT*) p_message;			 /* No, .. Put the message in the mailbox.					  */
     OS_CRTICAL_END();
     OS_ERR_SET(OS_ERR_NONE);
}

#endif	/* OS_CONFIG_MAILBOX_EN */
