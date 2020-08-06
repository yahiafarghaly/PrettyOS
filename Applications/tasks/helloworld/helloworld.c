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
 * Purpose  : Print a welcome message to the target board console every 1 second.
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
#define PRIO_WELCOME (10U)

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/

OS_tSTACK stkTask_Welcome [STACK_SIZE];
OS_tSTACK stkTask_Idle    [STACK_SIZE];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/

void OS_Hook_onIdle(void)
{
    /*  Application idle routine.    */
}

/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void
main_welcomeTask(void* args) {
    OS_TIME period = { 0U, 0U, 1U, 0U};
    while (1) {
        printf("PrettyOS %s !\n",(char*)args);
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

    /* Create the welcome task.             */
    OS_TaskCreate(&main_welcomeTask,
                  "2020",
                  stkTask_Welcome,
                  sizeof(stkTask_Welcome),
                  PRIO_WELCOME);


    /* Some applications logs.              */
    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);
    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
}
