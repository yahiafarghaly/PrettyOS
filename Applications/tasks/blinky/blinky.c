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
 * Purpose  : Demonstrate the correct timely executed tasks according to RMS (Rate Monotonic Scheduling) design
 *            for fixed priority tasks.
 *            A Green LED blinks every 100 system ticks, while a Blue LED blinks every 500 system ticks.
 *            According to RMS, Green task should be higher priority than blue task.
 *            With every blink, a global count is counting for each of task occurring.
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
#define STACK_SIZE  (40U)
#define PRIO_GREEN  (90U)
#define PRIO_BLUE   (20U)

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/

OS_tSTACK stkTask_GreenBlinky [STACK_SIZE];
OS_tSTACK stkTask_BlueBlinky  [STACK_SIZE];
OS_tSTACK stkTask_Idle        [STACK_SIZE];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/

static volatile CPU_t32U GreenBlink_count = 0U;
static volatile CPU_t32U BlueBlink_count  = 0U;

/* This function prints the current values of both GreenBlink_count and BlueBlink_count */
static inline void App_printStatus()
{
    printf("[Green]: %i \t\t [Blue]: %i\r",GreenBlink_count,BlueBlink_count);
}

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/

void App_Hook_TaskIdle(void)
{
    App_printStatus();
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
    GreenBlink_count = 0;
    volatile CPU_t16U i;

    while (1) {
        /*            Simulate a workload          */
        for (i = 1500U; i != 0U; --i) {
            BSP_LED_GreenOn();
            BSP_LED_BlueOff();
            BSP_LED_RedOff();
        }

        App_printStatus();
        OS_DelayTicks(100U);
        GreenBlink_count = ((GreenBlink_count + 1) > 5) ? 1 : (GreenBlink_count+1);
    }
}

void
main_BlueBlinky(void* args) {
    BlueBlink_count = 0;
    volatile CPU_t16U i;

    while (1) {
        /*            Simulate a workload          */
        for (i = 3*1500U; i != 0U; --i) {
            BSP_LED_BlueOn();
            BSP_LED_GreenOff();
            BSP_LED_RedOff();
        }

        App_printStatus();
        OS_DelayTicks(500U);
        ++BlueBlink_count;
    }
}

int main() {

    BSP_HardwareSetup();

    BSP_UART_ClearVirtualTerminal();


    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));

    OS_TaskCreate(&main_GreenBlinky,
                  OS_NULL(void),
                  stkTask_GreenBlinky,
                  sizeof(stkTask_GreenBlinky),
                  PRIO_GREEN);

    OS_TaskCreate(&main_BlueBlinky,
                  OS_NULL(void),
                  stkTask_BlueBlinky,
                  sizeof(stkTask_BlueBlinky),
                  PRIO_BLUE);



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
