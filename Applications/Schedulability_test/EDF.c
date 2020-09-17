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
 * Author   : 	Yahia Farghaly Ashour
 *
 * Purpose  : This example demonstrate the work of EDF scheduling. Make sure you set OS_CONFIG_EDF_EN to OS_CONFIG_ENABLE
 * 				such that the prettyOS is configured to work with EDF scheduling.
 *
 * 				In this example, We have 3 tasks with the following properties.
 *
				 +----------------------
				 | tsk|  T  |  C  |	 D 	|		T is the period of the task.
				 +----------------+-----		C is the computation time for execution.
				 | T1 |     |     |	 	|		D is the relative deadline for a task job.
				 |    | 20  | 3   |	7	|
				 +----------------+-----|		The unit of time here is second.
				 | T2 |     |     |	4	|		The hyper period here of {20,5,10} is 20 which is calculated as LCM (Least Common Multip)
				 |    |  5  | 2   |		|
				 +----------------+-----|
				 | T3 |     |     |		|
				 |    |  10 | 2   |	8	|
				 +----------------------+

				 The expected behavior:
							t[+00000] | Starts T_2
							t[+00002] | Ends   T_2
							===============================

							t[+00002] | Starts T_1
							t[+00004] | Ends   T_1
							===============================

							t[+00004] | Starts T_3
							t[+00006] | Ends   T_3
							===============================

							t[+00006] | Starts T_2
							t[+00008] | Ends   T_2
							===============================
							Idle
							t[+00010] | Starts T_3
							t[+00011] | Ends   T_3
							===============================

							t[+00011] | Starts T_2
							t[+00014] | Ends   T_2
							===============================
							Idle
							t[+00015] | Starts T_2
							t[+00017] | Ends   T_2
							===============================
							Idle
							t[+00020] | Starts T_1
							t[+00023] | Ends   T_1

 *
 * Language	:  	C
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include <bsp.h>
#include <pretty_os.h>
#include <uartstdio.h>

/*
*******************************************************************************
*                                   Macros                                    *
*******************************************************************************
*/
#define STACK_SIZE   (40U)

/* Task's Parameters In Seconds.			*/
#define Task_1_P	20U
#define Task_2_P	5U
#define Task_3_P	10U

#define Task_1_C	3U
#define Task_2_C	2U
#define Task_3_C 	2U

#define Task_1_D	7U
#define Task_2_D 	4U
#define Task_3_D 	8U

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/

OS_tSTACK stkTask_1 	  [STACK_SIZE];
OS_tSTACK stkTask_2 	  [STACK_SIZE];
OS_tSTACK stkTask_3 	  [STACK_SIZE];

OS_tSTACK stkTask_Idle    [STACK_SIZE];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/
typedef struct Task_Data
{
	char* name;
	OS_TICK T;		/* Period.				*/
	OS_TICK	C;		/* Computation Time.	*/
}tskdata;

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/

#if (OS_CONFIG_APP_STACK_OVERFLOW == OS_CONFIG_ENABLE)

void App_Hook_StackOverflow_Detected (OS_TASK_TCB* ptcb)
{

}

#endif

void App_Hook_TaskIdle(void)
{
    printf("Idle\r");
}

/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void
task_1(void* args) {

	tskdata* t = (tskdata*)args;

    while (1) {

    	printf("\n");
    	printf("t[+%05d] | Starts %s \n",OS_TickTimeGet()/OS_CONFIG_TICKS_PER_SEC,t->name);

    	BSP_DelayMilliseconds(t->C*1000);

    	printf("t[+%05d] | Ends   %s \n",OS_TickTimeGet()/OS_CONFIG_TICKS_PER_SEC,t->name);

    	if(OS_Is_CurrentTaskMissedDeadline())
    	{
    		printf("%s Missed its deadline ! \n",t->name);
    	}
    	printf("===============================\n");

    	OS_TaskYield();
    }
}

void
task_2(void* args) {

	tskdata* t = (tskdata*)args;

    while (1) {

    	printf("\n");
    	printf("t[+%05d] | Starts %s \n",OS_TickTimeGet()/OS_CONFIG_TICKS_PER_SEC,t->name);

    	BSP_DelayMilliseconds(t->C*1000);

    	printf("t[+%05d] | Ends   %s \n",OS_TickTimeGet()/OS_CONFIG_TICKS_PER_SEC,t->name);

    	if(OS_Is_CurrentTaskMissedDeadline())
    	{
    		printf("%s Missed its deadline ! \n",t->name);
    	}
    	printf("===============================\n");

    	OS_TaskYield();
    }
}

void
task_3(void* args) {

	tskdata* t = (tskdata*)args;

    while (1) {

    	printf("\n");
    	printf("t[+%05d] | Starts %s \n",OS_TickTimeGet()/OS_CONFIG_TICKS_PER_SEC,t->name);

    	BSP_DelayMilliseconds(t->C*1000);

    	printf("t[+%05d] | Ends   %s \n",OS_TickTimeGet()/OS_CONFIG_TICKS_PER_SEC,t->name);

    	if(OS_Is_CurrentTaskMissedDeadline())
    	{
    		printf("%s Missed its deadline ! \n",t->name);
    	}
    	printf("===============================\n");

    	OS_TaskYield();
    }
}

int main() {

    /* Setup low level connected devices.   */
    BSP_HardwareSetup();

    /* Clear console terminal.              */
    BSP_UART_ClearVirtualTerminal();

    /* Initialize the Idle Task stack.      */
    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));

    tskdata t1 = { "T_1" ,Task_1_P, Task_1_C};
    tskdata t2 = { "T_2" ,Task_2_P, Task_2_C};
    tskdata t3 = { "T_3" ,Task_3_P, Task_3_C};

    OS_TaskCreate(&task_1,
                  (void*)&t1,
                  &stkTask_1[0],
                  sizeof(stkTask_1),
				  OS_TASK_PERIODIC,
				  Task_1_D*OS_CONFIG_TICKS_PER_SEC,Task_1_P*OS_CONFIG_TICKS_PER_SEC);

    OS_TaskCreate(&task_2,
    			(void*)&t2,
                  &stkTask_2[0],
                  sizeof(stkTask_2),
				  OS_TASK_PERIODIC,
				  Task_2_D*OS_CONFIG_TICKS_PER_SEC,Task_2_P*OS_CONFIG_TICKS_PER_SEC);

    OS_TaskCreate(&task_3,
    			(void*)&t3,
                  &stkTask_3[0],
                  sizeof(stkTask_3),
				  OS_TASK_PERIODIC,
				  Task_3_D*OS_CONFIG_TICKS_PER_SEC,Task_3_P*OS_CONFIG_TICKS_PER_SEC);

    /* Some applications logs.              */
    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);
    printf("[Schedulability Test]:\n");

    float U = ((float)Task_1_C/Task_1_P) + ((float)Task_2_C/Task_2_P) + ((float)Task_3_C/Task_3_P);

    printf("	U = %f\n", U);
    if(U > 1.0F)
    {
    	printf("	Task set is not guarantee to be schedulable\n");
    }
    else
    {
    	printf("	Task set is schedulable\n");
    }

    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
}


