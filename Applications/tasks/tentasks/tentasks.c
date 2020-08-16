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
 * Purpose  : 10 tasks, each one prints a message.
 *
 * Language:  C
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
#define PRIO_BASE	 (3U)

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/

OS_tSTACK stkTask_1 	  [STACK_SIZE];
OS_tSTACK stkTask_2 	  [STACK_SIZE];
OS_tSTACK stkTask_3 	  [STACK_SIZE];
OS_tSTACK stkTask_4 	  [STACK_SIZE];
OS_tSTACK stkTask_5 	  [STACK_SIZE];
OS_tSTACK stkTask_6 	  [STACK_SIZE];
OS_tSTACK stkTask_7 	  [STACK_SIZE];
OS_tSTACK stkTask_8 	  [STACK_SIZE];
OS_tSTACK stkTask_9 	  [STACK_SIZE];
OS_tSTACK stkTask_10 	  [STACK_SIZE];

OS_tSTACK stkTask_Idle    [STACK_SIZE];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/
volatile unsigned long  second_count = 0;
/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/

void App_Hook_TaskIdle(void)
{
    /*  Application idle routine.    */
}

/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void
task_1(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        if(second_count > 2)
        {
        	if(!OS_TaskDelete(PRIO_BASE + 1))
        	{
        		printf("----> Task#1 ---> [Deleted]\n");
        	}
        	else
        	{
        		printf("----> Task#1 ---> [Failed to deleted]\n");
        		return;
        	}
        }
        OS_DelayTime(&period);
    }
}

void
task_2(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}

void
task_3(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}


void
task_4(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}


void
task_5(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}


void
task_6(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}


void
task_7(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}


void
task_8(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}


void
task_9(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}


void
task_10(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",second_count,(char*)args);
        ++second_count;
        OS_DelayTime(&period);
    }
}

int main() {

    /* Setup low level connected devices.   */
    BSP_HardwareSetup();

    /* Clear console terminal.              */
    BSP_UART_ClearVirtualTerminal();

    /* Initialize the Idle Task stack.      */
    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));

    /* Create the welcome tasks.
     * task_1 is lowest priority.
     * ... task_10 is the highest priority.
     */
    OS_TaskCreate(&task_1,
                  "1",
                  &stkTask_1,
                  sizeof(stkTask_1),
				  PRIO_BASE + 1);

    OS_TaskCreate(&task_2,
                  "2",
                  &stkTask_2,
                  sizeof(stkTask_2),
				  PRIO_BASE + 2);

    OS_TaskCreate(&task_3,
                  "3",
                  &stkTask_3,
                  sizeof(stkTask_3),
				  PRIO_BASE + 3);

    OS_TaskCreate(&task_4,
                  "4",
                  &stkTask_4,
                  sizeof(stkTask_4),
				  PRIO_BASE + 4);

    OS_TaskCreate(&task_5,
                  "5",
                  &stkTask_5,
                  sizeof(stkTask_5),
				  PRIO_BASE + 5);

    OS_TaskCreate(&task_6,
                  "6",
                  &stkTask_6,
                  sizeof(stkTask_6),
				  PRIO_BASE + 6);

    OS_TaskCreate(&task_7,
                  "7",
                  &stkTask_7,
                  sizeof(stkTask_7),
				  PRIO_BASE + 7);

    OS_TaskCreate(&task_8,
                  "8",
                  &stkTask_8,
                  sizeof(stkTask_8),
				  PRIO_BASE + 8);

    OS_TaskCreate(&task_9,
                  "9",
                  &stkTask_9,
                  sizeof(stkTask_9),
				  PRIO_BASE + 9);

    OS_TaskCreate(&task_10,
                  "10",
                  &stkTask_10,
                  sizeof(stkTask_10),
				  PRIO_BASE + 10);

    /* Some applications logs.              */
    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);

    for(int s = 3; s > 0; s--)
    {
    	printf("\r.. %d ...\r",s);
        BSP_DelayMilliseconds(1000);
    }

    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
}
