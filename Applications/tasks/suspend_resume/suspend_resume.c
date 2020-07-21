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
 * Purpose  : This example demonstrate the use of suspend/resume APIs of prettyOS.
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
#define STACK_SIZE          (40U)
#define GREEN_TASK_PRIO     (90U)
#define BLUE_TASK_PRIO      (35U)

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
    printf("Blinky1[G]: %i \t\t Blinky2[B]: %i\r",GBlink_count,BBlink_count);
}

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/

void OS_Hook_onIdle(void)
{
    if(BBlink_count == 10)
    {
        OS_TaskResume(GREEN_TASK_PRIO);
        printf("\nGreen Task is resumed. \n");
        BBlink_count = 0;
    }
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
        GBlink_count = ((GBlink_count + 1) > 5) ? 1 : (GBlink_count+1);
        App_printStat();
        OS_DelayTicks(100U);
    }
}

void
main_BlueBlinky(void* args) {
    BBlink_count = 0;
    OS_tRet ret;
    while (1) {
        CPU_t32U  i;
        for (i = 3*1500U; i != 0U; --i) {
            BSP_LED_BlueOn();
            BSP_LED_GreenOff();
            BSP_LED_RedOff();
        }
        ++BBlink_count;
        if(BBlink_count == 3)
        {
            ret = OS_TaskSuspend(GREEN_TASK_PRIO);
            if(OS_ERR_NONE == ret)
            {
               printf("\nGreen Task is suspended. \n");
            }else if(OS_ERR_TASK_SUSPENDED == ret)
            {
                /* Already suspended. */
            }
            else
            {
                printf("\nTask suspension error:%d\n",ret);
            }
        }
        App_printStat();
        OS_DelayTicks(300U);
    }
}

int main() {

    BSP_HardwareSetup();

    BSP_UART_ClearVirtualTerminal();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: BSP ticks per second: %d \n",BSP_TICKS_PER_SEC_CONFIG);

    OS_Init(stack_idleTask, sizeof(stack_idleTask));

    OS_TaskCreate(&main_GreenBlinky,
                  OS_NULL(void),
                  stack_GreenBlink,
                  sizeof(stack_GreenBlink),
                  GREEN_TASK_PRIO);

    OS_TaskCreate(&main_BlueBlinky,
                  OS_NULL(void),
                  stack_BlueBlink,
                  sizeof(stack_BlueBlink),
                  BLUE_TASK_PRIO);

    printf("[Info]: Starts !\n\n");

    /* transfer control to the RTOS to run the tasks */
    OS_Run(BSP_CPU_FrequencyGet());

    //return 0;
}
