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
 * Author   :   Yahia Farghaly Ashour
 *
 * Purpose  :   PrettyOS Hooks APIs.
 *
 * Language :   C
 *
 * Set 1 tab = 4 spaces for better comments readability.
 */

#ifndef __PRETTYOS_HOOKS_H_
#define __PRETTYOS_HOOKS_H_

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
#include "pretty_types.h"

/*
*******************************************************************************
*                                                                             *
*                      Application Hook Specific Functions                 	  *
*                [Preferred to do little work as much as possible]		      *
*                                                                             *
*******************************************************************************
*/


#if (OS_CONFIG_APP_TASK_IDLE == OS_CONFIG_ENABLE)
	void App_Hook_TaskIdle		(void);								/* Calls Application specific code in the idle state of prettyOS. 								*/
#endif

#if (OS_CONFIG_APP_TASK_SWITCH == OS_CONFIG_ENABLE)
	void App_Hook_TaskSwitch	(void);								/* Calls Application specific code when task context switch occurs. 							*/
#endif

#if (OS_CONFIG_APP_TASK_CREATED == OS_CONFIG_ENABLE)
	void App_Hook_TaskCreated 	(OS_TASK_TCB* ptcb);				/* Calls Application specific code when a task is created. 										*/
#endif

#if (OS_CONFIG_APP_TASK_DELETED == OS_CONFIG_ENABLE)
	void App_Hook_TaskDeleted 	(OS_TASK_TCB* ptcb);				/* Calls Application specific code when a task is deleted. 										*/
#endif

#if (OS_CONFIG_APP_TASK_RETURNED == OS_CONFIG_ENABLE)
	void App_Hook_TaskReturned	(OS_TASK_TCB* ptcb); 				/* Calls Application specific code when a task returns intentionally. 							*/
#endif

#if (OS_CONFIG_APP_TIME_TICK == OS_CONFIG_ENABLE)
	void App_Hook_TimeTick 		(void);  							/* Calls Application specific code when an OS system tick occurs. (i.e the single tick ! )		*/
#endif

#if (OS_CONFIG_APP_STACK_OVERFLOW == OS_CONFIG_ENABLE)
	void App_Hook_StackOverflow_Detected (OS_TASK_TCB* ptcb);       /* Calls Application specific code for a possible event of a task's stack overflow is detected. */
#endif


/*
*******************************************************************************
*                      														  *
*                      Target Hook Specific Functions                 		  *
*                      	   [Required in CPU port]							  *
*             [Preferred to do little work as much as possible]				  *
*                      														  *
*******************************************************************************
*/

#if(OS_CONFIG_CPU_INIT == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_Init 			(void);					/* Hooked with OS_Init() and is called once before OS_Init() does a thing.						*/
#endif

#if(OS_CONFIG_CPU_IDLE == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_Idle		(void);						/* A low level CPU idle routine in the idle state of prettyOS.									*/
#endif

#if(OS_CONFIG_CPU_TASK_CREATED == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_TaskCreated 	(OS_TASK_TCB*	ptcb);	/* A low level CPU routine when a task is created.												*/
#endif

#if(OS_CONFIG_CPU_TASK_DELETED == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_TaskDeleted		(OS_TASK_TCB*	ptcb);	/* A low level CPU routine when a task is deleted.												*/
#endif

#if(OS_CONFIG_CPU_CONTEXT_SWITCH == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_ContextSwitch 	(void);					/* A low level CPU routine when a context switch occurs.										*/
#endif

#if(OS_CONFIG_CPU_TIME_TICK == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_TimeTick     	(void);					/* A low level CPU routine when an OS system tick occurs. (i.e the single tick ! )				*/
#endif

#if(OS_CONFIG_CPU_SOFT_STK_OVERFLOW_DETECTION == OS_CONFIG_ENABLE)
	extern void OS_CPU_Hook_StackOverflow_Detected (void);          /* A low level CPU routine called when a task stack overflow is detected.                       */
#endif

#ifdef __cplusplus
}
#endif
#endif /* __PRETTYOS_HOOKS_H_ */
