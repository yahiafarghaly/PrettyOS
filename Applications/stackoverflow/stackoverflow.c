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
 * Purpose  : This examples shows how a task can be overflowed and how it's detected by the prettyOS.
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
#define PRIO_BASE    (3U)

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/

unsigned long stack_size_factor = 1;

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/

OS_tSTACK stkTask_1       [STACK_SIZE];
OS_tSTACK stkTask_2       [STACK_SIZE];
OS_tSTACK stkTask_3       [STACK_SIZE];
OS_tSTACK stkTask_Idle    [STACK_SIZE];

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/

void App_Hook_TaskIdle(void)
{
    /*  Application idle routine.    */
    BSP_CPU_WFI();
}

/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void
task_1(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    unsigned long volatile sum = 0;
    unsigned long volatile arr[100] = { 0xDeadBeef };

    for(int i = 0; i < 100; i++)
        arr[i] = 0xdeadbeef;

    while (1) {
        printf("[+%05d]: task %s !\n",OS_TickTimeGet(),(char*)args);
        for(int i = 0; i < 100; i++)
            sum += arr[i];
        printf("Sum = %d\n",sum);
        OS_DelayTime(&period);
    }
}

void
task_2(void* args) {
    OS_TIME period = { 0U, 0U, 2U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",OS_TickTimeGet(),(char*)args);
        OS_DelayTime(&period);
    }
}

void
task_3(void* args) {
    OS_TIME period = { 0U, 0U, 3U, 0U};
    while (1) {
        printf("[+%05d]: task %s !\n",OS_TickTimeGet(),(char*)args);
        OS_DelayTime(&period);
    }
}


#if (OS_CONFIG_APP_STACK_OVERFLOW == OS_CONFIG_ENABLE)
extern CPU_tWORD    volatile        OS_Running;
void App_Hook_StackOverflow_Detected (OS_TASK_TCB* ptcb)
{
    BSP_LED_RedOn();

    printf("TASK with priority %d has been overflowed !\n",ptcb->TASK_priority);

    unsigned long* pstack_size_factor = &stack_size_factor;

    *pstack_size_factor += 1;   /* Assume Flash memory is writable, then the const variable will change.            */

    printf("Increase the stack of task_1 to %d\n",STACK_SIZE*(*pstack_size_factor));

    printf("Reset The CPU in\n");

    for(int s = 3; s > 0; s--)
    {
        printf("\r.. %d ...\r",s);
        BSP_DelayMilliseconds(1000);
    }

    BSP_CPU_Reset();
}

#endif

int main() {

    /* Setup low level connected devices.   */
    BSP_HardwareSetup();

    /* Clear console terminal.              */
    BSP_UART_ClearVirtualTerminal();

    /* Initialize the Idle Task stack.      */
    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));

    OS_TaskCreate(&task_1,
                  "1",
                  &stkTask_1,
                  sizeof(stkTask_1)*stack_size_factor,
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

    /* Some applications logs.              */
    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);
    printf("[Info]: Task_1 Stack Size Factor is %d\n",stack_size_factor);
    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
}
