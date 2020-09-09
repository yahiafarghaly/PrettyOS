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
 * Purpose  : 	This example shows how the RMS of 3 tasks are not feasible
 * 				and 1 activation per period NOT guaranteed. In this example, it's task 3
 *
 *
				 +----------------+
				 | tsk|  T  |  C  |		T is the period of the task.
				 +----------------+		C is the computation power.
				 | T1 |     |     |
				 |    |  3  | 1   |
				 +----------------+
				 | T2 |     |     |
				 |    |   4 | 1   |
				 +----------------+
				 | T3 |     |     |
				 |    |   6 | 2.1 |
				 +----------------+
 *
 *					U = C1/T1 + C2/T2 + C3/T3 = 0.93 > 0.78 ( not feasible ).
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
#define PRIO_Task_1	 (5U)
#define PRIO_Task_2	 (4U)
#define PRIO_Task_3	 (3U)


#define Task_1_PeriodSec		3.0F
#define Task_2_PeriodSec		4.0F
#define Task_3_PeriodSec		6.0F

#define Task_1_ComputionSec		1.0F
#define Task_2_ComputionSec 	1.0F
#define Task_3_ComputionSec 	2.1F

#define ExecutionLOAD(C)	do { BSP_DelayMilliseconds(C*1000); }while(0);
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
	OS_TICK T;		/* Periodicity			*/
	OS_TICK	C;		/* Computation Power.	*/
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
    /*printf("[+%05d]: Idle\r",OS_TickTimeGet());*/
}

/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void
task_1(void* args) {

	OS_TICK curr_tick = 0;
	OS_TICK tick_delays = 0;
	OS_TICK	execution_cnt = 1;

	tskdata* t = (tskdata*)args;

    while (1) {

    	curr_tick = OS_TickTimeGet();

        printf("\n[+%05d]: %s --> \n",curr_tick,t->name);

    	ExecutionLOAD(t->C/OS_CONFIG_TICKS_PER_SEC);

    	curr_tick = OS_TickTimeGet();

        printf("\n[+%05d]: %s \n",curr_tick,t->name);

        if(curr_tick > execution_cnt * t->T)
        {
        	printf(" %s missed its deadline !",t->name);
//        	OS_SchedLock();								/* STOP Scheduling if deadline is missed. */
//        	for(;;);
        }

    	tick_delays = (execution_cnt * t->T) - (curr_tick);
    	++execution_cnt;

    	OS_DelayTicks(tick_delays);
    }
}

void
task_2(void* args) {

	OS_TICK curr_tick = 0;
	OS_TICK tick_delays = 0;
	OS_TICK	execution_cnt = 1;

	tskdata* t = (tskdata*)args;

    while (1) {

    	curr_tick = OS_TickTimeGet();

        printf("\n[+%05d]: %s --> \n",curr_tick,t->name);

    	ExecutionLOAD(t->C/OS_CONFIG_TICKS_PER_SEC);

    	curr_tick = OS_TickTimeGet();

        printf("\n[+%05d]: %s \n",curr_tick,t->name);

        if(curr_tick > execution_cnt * t->T)
        {
        	printf(" %s missed its deadline !",t->name);
//        	OS_SchedLock();								/* STOP Scheduling if deadline is missed. */
//        	for(;;);
        }

        tick_delays = (execution_cnt * t->T) - (curr_tick);
    	++execution_cnt;

    	OS_DelayTicks(tick_delays);
    }
}

void
task_3(void* args) {

	OS_TICK curr_tick = 0;
	OS_TICK tick_delays = 0;
	OS_TICK	execution_cnt = 1;

	tskdata* t = (tskdata*)args;

    while (1) {

    	curr_tick = OS_TickTimeGet();

        printf("\n[+%05d]: %s --> \n",curr_tick,t->name);

    	ExecutionLOAD(t->C/OS_CONFIG_TICKS_PER_SEC);

    	curr_tick = OS_TickTimeGet();

        printf("\n[+%05d]: %s \n",curr_tick,t->name);

        if(curr_tick > execution_cnt * t->T)
        {
        	printf(" %s missed its deadline !",t->name);
//       	OS_SchedLock();								/* STOP Scheduling if deadline is missed. */
//        	for(;;);
        }

        tick_delays = (execution_cnt * t->T) - (curr_tick);
    	++execution_cnt;

    	OS_DelayTicks(tick_delays);
    }
}

int main() {

    /* Setup low level connected devices.   */
    BSP_HardwareSetup();

    /* Clear console terminal.              */
    BSP_UART_ClearVirtualTerminal();

    /* Initialize the Idle Task stack.      */
    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));

    tskdata t1 = { "T_1" ,Task_1_PeriodSec*OS_CONFIG_TICKS_PER_SEC, Task_1_ComputionSec*OS_CONFIG_TICKS_PER_SEC};
    tskdata t2 = { "	T_2" ,Task_2_PeriodSec*OS_CONFIG_TICKS_PER_SEC, Task_2_ComputionSec*OS_CONFIG_TICKS_PER_SEC};
    tskdata t3 = { "		T_3" ,Task_3_PeriodSec*OS_CONFIG_TICKS_PER_SEC, Task_3_ComputionSec*OS_CONFIG_TICKS_PER_SEC};

    OS_TaskCreate(&task_1,
                  (void*)&t1,
                  &stkTask_1[0],
                  sizeof(stkTask_1),
				  PRIO_Task_1);

    OS_TaskCreate(&task_2,
    			(void*)&t2,
                  &stkTask_2[0],
                  sizeof(stkTask_2),
				  PRIO_Task_2);

    OS_TaskCreate(&task_3,
    			(void*)&t3,
                  &stkTask_3,
                  sizeof(stkTask_3),
				  PRIO_Task_3);

    /* Some applications logs.              */
    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);
    printf("[Schedulability Test]:\n");

    float U = (Task_1_ComputionSec/Task_1_PeriodSec) + (Task_2_ComputionSec/Task_2_PeriodSec) + (Task_3_ComputionSec/Task_3_PeriodSec);

    printf("	U = %f\n", U);
    if(U > 0.76)
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
