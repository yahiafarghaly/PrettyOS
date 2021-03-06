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
 * Purpose  : This example shows the priority inversion bug which can happens if priority ceiling protocol is not used.
 *            Also, shows how the bug is avoided when OCPP (Original Ceiling Priority Protocol) is used when passing the argument
 *            'OS_MUTEX_PRIO_CEIL_ENABLE' to the created mutex.
 *
 *            In this example, we have 3 tasks ( L,M,H ) where L refers to a low priority task, M is a middle priority task, H is the highest priority task.
 *            Suppose we have a virtual line that a message is sent on.
 *
 *            H task: For every 1    second, it sends a critical message "SOS" on a virtual line. It's not acceptable that the task is not meet its periodic timing.
 *            M task: For every 0.25 second, it does some jobs which is roughly takes +1 second. the job is not related to H or L tasks.
 *            L task: For every 0.5  second, it sends a test message " T E S T " on a virtual line (used by H task).
 *            Additionally to add more complex scenario,
 *            VH task:For every 5    second, it prints --- VH ---, it is the highest task in the system which doing a job unrelated to L,M or H tasks.
 *
 *            There is a single virtual line to send the message, Hence a mutex service is used in this example to give a special single access for one task at a time.
 *
 *            While L is executed, it's preempted by M task which is strictly fine due to the priority order.
 *            When H is executed, it must send immediately "SOS" message on the virtual line. But as L owns virtual line and it's get preempted
 *            before releasing it. The H task is delayed and it misses its deadline.
 *            And as a result, M continues its job till the end, then L continues to send "T E S T" message, the H is executed.
 *            This is the priority inversion bug. i.e the higher priority is being inverted to be a low priority task in a certain scenario.
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
#define STACK_SIZE   (60U)
#define PRIO_L_TASK  (5U)
#define PRIO_M_TASK  (6U)
#define PRIO_H_TASK  (7U)
#define PRIO_PCP     (8U)
#define PRIO_VH_TASK  (10U)
/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stkTask_L     [STACK_SIZE];
OS_tSTACK stkTask_M     [STACK_SIZE];
OS_tSTACK stkTask_H     [STACK_SIZE];
OS_tSTACK stkTask_VH    [STACK_SIZE];
OS_tSTACK stkTask_Idle  [STACK_SIZE];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/
OS_MUTEX* message_lock;

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

void task_L(void* args)
{
    OS_TIME period = { 0, 0, 0, 500};
    while(1)
    {
        printf("\nSending '%s' message\n",(char*)args);
        /*  Sending the message takes roughly +2 seconds. */
        OS_MutexPend(message_lock, 0U);

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(' ');
        BSP_UART_SendByte(((char*)args)[0]);
        BSP_UART_SendByte(' ');

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(' ');
        BSP_UART_SendByte(((char*)args)[1]);
        BSP_UART_SendByte(' ');

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(' ');
        BSP_UART_SendByte(((char*)args)[2]);
        BSP_UART_SendByte(' ');

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(' ');
        BSP_UART_SendByte(((char*)args)[3]);
        BSP_UART_SendByte(' ');

        OS_MutexPost(message_lock);


        OS_DelayTime(&period);
    }
}

void task_M(void* args)
{
    OS_TIME period = { 0, 0, 0, 250};
    int i = 0U;
    while(1)
    {
        /* The execution takes roughly +1 second.  */
        BSP_UART_SendByte('[');         /* Starts with close bracket.   */
        for(i = 0; i < 10U; i++)
        {
            BSP_UART_SendByte(48 + i);  /* Print digits [0 --> 9]'      */
            BSP_DelayMilliseconds(100);
        }
        BSP_UART_SendByte(']');         /* Ends with a bracket.         */
        OS_DelayTime(&period);
    }
}

void task_H(void* args)
{
    OS_TIME period = { 0, 0, 1U, 0};
    while(1)
    {
        printf("\nSending '%s' message\n",(char*)args);
        /*  Sending the message takes roughly +1.5 seconds. */
        OS_MutexPend(message_lock, 0U);

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(((char*)args)[0]);
        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(((char*)args)[1]);
        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(((char*)args)[2]);
        BSP_UART_SendByte('\n');
        BSP_UART_SendByte('\r');

        OS_MutexPost(message_lock);

        OS_DelayTime(&period);
    }
}

void task_VH(void* args)
{
    OS_TIME period = { 0, 0, 5U, 0};
    while(1)
    {
        printf("\n--- VH ---\n");
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
    OS_TaskCreate(&task_L,
                  "TEST",
                  stkTask_L,
                  sizeof(stkTask_L),
                  PRIO_L_TASK);

    OS_TaskCreate(&task_M,
                  "The Middle Task",
                  stkTask_M,
                  sizeof(stkTask_M),
                  PRIO_M_TASK);

    OS_TaskCreate(&task_H,
                  "SOS",
                  stkTask_H,
                  sizeof(stkTask_H),
                  PRIO_H_TASK);

    OS_TaskCreate(&task_VH,
                  "SOS",
                  stkTask_VH,
                  sizeof(stkTask_VH),
                  PRIO_VH_TASK);

    message_lock = OS_MutexCreate(PRIO_PCP, OS_MUTEX_PRIO_CEIL_ENABLE);
    if(message_lock == (OS_MUTEX*)0U)
    {
        printf("\nError Creating `message_lock` Mutex\n");
        printf("Error message: %s\n",OS_StrError(OS_ERRNO));
    }

    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
    return 0;
}
