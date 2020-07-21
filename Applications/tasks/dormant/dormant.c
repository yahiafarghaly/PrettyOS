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
 * Purpose  : This example demonstrate how to delete a task, how to re-create it and
 *              and how a returned task is catched as deleted.
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
#define STACK_SIZE     (40U)
#define GREEN_PRIO     (90U)
#define BLUE_PRIO      (35U)

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stack_GreenBlink [STACK_SIZE];
OS_tSTACK stack_BlueBlink  [STACK_SIZE];
OS_tSTACK stack_idleTask   [STACK_SIZE];

/*
*******************************************************************************
*                               Globals                                       *
*******************************************************************************
*/
static volatile CPU_t32U GBlink_count = 0U;
static volatile CPU_t32U BBlink_count = 0U;

/*
*******************************************************************************
*                            Functions Definitions                            *
*******************************************************************************
*/

static inline void App_printStat()
{
    if(BBlink_count == 12 && GBlink_count == 10)
    {
        printf("Idle State: ==> Blinky1[G]: %i \t\t Blinky2[B]: %i\r",GBlink_count,BBlink_count);
    }
    else
    {
        printf("Blinky1[G]: %i \t\t Blinky2[B]: %i\r",GBlink_count,BBlink_count);
    }
}

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/

void OS_Hook_onIdle(void)
{
    App_printStat();
    BSP_LED_GreenOff();
    BSP_LED_BlueOff();
    BSP_CPU_WFI();
}

/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void
main_GreenBlinky(void* args) {
    GBlink_count = 0;
    while (1) {
        CPU_t32U i;
        /* Simulate a workload */
        for (i = 1500U; i != 0U; --i) {
            BSP_LED_GreenOn();
            BSP_LED_BlueOff();
            BSP_LED_RedOff();
        }
        App_printStat();
        OS_DelayTicks(100U);
        ++GBlink_count;
        if(GBlink_count == 10)
        {
            printf("\nDeleting Green task\n");
            OS_TaskDelete(GREEN_PRIO);
        }
    }
}

void
main_BlueBlinky(void* args) {
    BBlink_count = 0;
    while (1) {
        CPU_t32U  i;
        for (i = 3*1500U; i != 0U; --i) {
            BSP_LED_BlueOn();
            BSP_LED_GreenOff();
            BSP_LED_RedOff();
        }
        App_printStat();
        OS_DelayTicks(500U);
        ++BBlink_count;
        if(BBlink_count == 5)
        {
            printf("\nRestoring Green task\n");
            OS_TaskCreate(&main_GreenBlinky, OS_NULL(void), stack_GreenBlink, sizeof(stack_GreenBlink), GREEN_PRIO);
        }
        if(BBlink_count == 12)
        {
            printf("\nExit infinite loop of Blue task\n");
            printf("\nRe-Create the green task\n");
            OS_TaskCreate(&main_GreenBlinky, OS_NULL(void), stack_GreenBlink, sizeof(stack_GreenBlink), GREEN_PRIO);
            break;
        }
    }
}



int main() {

    BSP_HardwareSetup();
    BSP_UART_ClearVirtualTerminal();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_TICKS_PER_SEC);

    OS_Init(stack_idleTask, sizeof(stack_idleTask));

    OS_TaskCreate(&main_GreenBlinky,
                  OS_NULL(void),
                  stack_GreenBlink,
                  sizeof(stack_GreenBlink),
                  GREEN_PRIO);

    OS_TaskCreate(&main_BlueBlinky,
                  OS_NULL(void),
                  stack_BlueBlink,
                  sizeof(stack_BlueBlink),
                  BLUE_PRIO);


    printf("[Info]: Starts !\n\n");

    /* transfer control to the RTOS to run the tasks */
    OS_Run(BSP_CPU_FrequencyGet());

    //return 0;
}
