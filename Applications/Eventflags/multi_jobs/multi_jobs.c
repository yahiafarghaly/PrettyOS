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
 * Purpose  :	This example demonstrates the usage of event flags for waiting of multiple bits (events) to occur from different tasks.
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
#define STACK_SIZE   		(40U)
#define PRIO_BASE			(8U)


#define BIT_0	(1 << 0)
#define BIT_1	(1 << 1)
#define BIT_2	(1 << 2)
#define BIT_3	(1 << 3)
#define BIT_4	(1 << 4)
#define BIT_5	(1 << 5)
#define BIT_6	(1 << 6)
#define BIT_7	(1 << 7)


/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stkTask 		 [5][STACK_SIZE];
OS_tSTACK stkTask_Idle		[20];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/

OS_EVENT_FLAG_GRP* event_bits;

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
*                              Helpful Functions                              *
*******************************************************************************
*/


/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void Task_1(void* args)
{
    OS_TIME period = { 0, 0, 1U, 0};
    OS_FLAG flags;
    printf("Task_1 Started !\n");

    OS_DelayTime(&period);
    flags = OS_EVENT_FlagPost(event_bits,BIT_0,OS_FLAG_SET);
    printf("Task_1 sets 0x%x\n",flags);
}

void Task_2(void* args)
{
    OS_TIME period = { 0, 0, 1U, 0};
    OS_FLAG flags;
    printf("Task_2 Started !\n");

    OS_DelayTime(&period);
    flags = OS_EVENT_FlagPost(event_bits,BIT_1,OS_FLAG_SET);
    printf("Task_2 sets 0x%x\n",flags);
}


void Task_3(void* args)
{
    OS_TIME period = { 0, 0, 3U, 0};
    OS_FLAG flags;
    printf("Task_3 Started !\n");

    OS_DelayTime(&period);

	flags = OS_EVENT_FlagPost(event_bits,BIT_2,OS_FLAG_SET);
	printf("Task_3 sets 0x%x\n",flags);
}


void Task_4(void* args)
{
    OS_TIME period = { 0, 0, 4U, 0};
    OS_FLAG flags;
    printf("Task_4 Started !\n");

	OS_DelayTime(&period);

	flags = OS_EVENT_FlagPost(event_bits,BIT_3,OS_FLAG_SET);
	printf("Task_4 sets 0x%x\n",flags);
}

void Task_sum(void* args)
{
    OS_TIME period = { 0, 0, 1U, 0};
    OS_FLAG waited_bits;

    printf("%s Started !\n\n",(char*)args);

    event_bits = OS_EVENT_FlagCreate(0);

    if(event_bits == OS_NULL(OS_EVENT_FLAG_GRP))
    {
        printf("\nError Creating `event_bits` \n");
        printf("Error message: %s\n",OS_StrError(OS_ERRNO));
        return;
    }

    void (*task[4])(void*) = { Task_1, Task_2, Task_3, Task_4};

    for(int i = 0; i < 4; i++)
    {
        OS_TaskCreate(task[i],
        			   (void*)0,
                      stkTask[i+1],
                      sizeof(stkTask[i+1]),
                      PRIO_BASE + i + 5);
    }

    while(1)
    {
    	waited_bits = OS_EVENT_FlagPend(event_bits,BIT_0 | BIT_1 | BIT_2 | BIT_3,OS_FLAG_WAIT_SET_ALL,OS_TRUE,0);
    	printf("[Task_sum]: 0x%x has been occurred\n",waited_bits);
    	printf("----------------------------------\n");

        OS_DelayTime(&period);
    }
}

int main (void)
{

    /* Setup low level connected devices.   */
    BSP_HardwareSetup();

    /* Clear console terminal.              */
    BSP_UART_ClearVirtualTerminal();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);


    /* Initialize the Idle Task stack.      */
    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));

    /* Create the tasks.                    */
    OS_TaskCreate(Task_sum,
                  "Task_sum",
                  stkTask[0],
                  sizeof(stkTask[0]),
                  PRIO_BASE);

    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
    return 0;
}
