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
 * Purpose  : This example demonstrate the the effect of changing priority of a task while maintaining
 *            Getting to the right time.
 *            This is demonstrated by reducing/increasing the priority of the 'main_GreenBlinky' against the other fixed priority tasks.
 *            To see that the effect is taking properly and effectively, the period of each of 'main_GreenBlinky' and 'main_RedBlinky'
 *            is set to be the same to see which one will be scheduled first when changing the priority.
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
#define PRIO_GREEN          (6U)
#define PRIO_RED            (5U)
#define PRIO_TASK_CHANGER   (4U)
#define PRIO_VALUE          (3U)

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stkTask_Idle    [STACK_SIZE];
OS_tSTACK stkTask_Green   [STACK_SIZE];
OS_tSTACK stkTask_Red     [STACK_SIZE];
OS_tSTACK stkTask_changer [STACK_SIZE];

/*
*******************************************************************************
*                               Globals                                       *
*******************************************************************************
*/

static volatile CPU_t32U GreenBlink_count = 0U;
static volatile CPU_t32U RedBlink_count   = 0U;

/*
*******************************************************************************
*                            Functions Prototypes                             *
*******************************************************************************
*/

static inline void App_printStat (void)
{
    printf("Blinky[G]: %i \t\t Blinky[R]: %i\r",GreenBlink_count,RedBlink_count);
}

static inline void App_fakeLoad (void)
{
    CPU_t16U i,j;
    for(j = 0; j < 10U;j++)
    {
        for (i = 150U; i != 0U; --i) {
            BSP_CPU_NOP();
        }
    }
}

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/
extern void main_ChangerTask(void* args);

void OS_Hook_onIdle(void)
{
    BSP_LED_GreenOff();
    BSP_LED_BlueOff();
    BSP_LED_RedOff();
    BSP_CPU_WFI();
}

/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void
main_ChangerTask(void* args) {

    static unsigned char switchFlag = 0U;
    OS_TIME period = { 0, 0, 4U, 500U};    /*  Every 4.5 seconds.  */

    while (1) {

        OS_DelayTime(&period);

        BSP_LED_GreenOn();
        BSP_LED_BlueOn();
        BSP_LED_RedOn();

        switch(switchFlag)
        {
        case 0x00:
            if(OS_ERR_NONE != OS_TaskChangePriority(PRIO_GREEN, PRIO_VALUE))
            {
                printf("\n[ F a i l ] ==> Reduce: PRIO_GREEN = [%d]->[%d]\n",PRIO_GREEN,PRIO_VALUE);
            }
            else
            {
                printf("\n Reduce: PRIO_GREEN = [%d]->[%d]\n",PRIO_GREEN,PRIO_VALUE);
            }
            break;
        case 0xFF:
        default:
            if(OS_ERR_NONE != OS_TaskChangePriority(PRIO_VALUE,PRIO_GREEN))
            {
                printf("\n[ F a i l ] ==> Increase: PRIO_GREEN = [%d]->[%d]\n",PRIO_VALUE,PRIO_GREEN);
            }
            else
            {
                printf("\n Increase: PRIO_GREEN = [%d]->[%d]\n",PRIO_VALUE,PRIO_GREEN);
            }
            break;
        }

        switchFlag = ~(switchFlag);
    }
}

void
main_GreenBlinky(void* args) {

    GreenBlink_count = 0;
    OS_TIME period = { 0, 0, 1, 0};     /*  Every 1 seconds.  */

    while (1) {

        BSP_UART_SendByte('G');
        BSP_UART_SendByte(' ');

        ++GreenBlink_count;

        BSP_LED_GreenOn();
        BSP_LED_BlueOff();
        BSP_LED_RedOff();

        //App_fakeLoad();

        OS_DelayTime(&period);
    }
}

void
main_RedBlinky(void* args) {

    RedBlink_count = 0;
    OS_TIME period = { 0, 0, 1, 0};     /*  Every 1 seconds.  */

    while (1) {

        BSP_UART_SendByte('R');
        BSP_UART_SendByte(' ');

        ++RedBlink_count;

        BSP_LED_RedOn();
        BSP_LED_BlueOff();
        BSP_LED_GreenOff();

        //App_fakeLoad();

        OS_DelayTime(&period);
    }
}

int main() {

    BSP_HardwareSetup();

    BSP_UART_ClearVirtualTerminal();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);
    printf("[Info]: Green, Red, Changer PRIOs respectively = %d, %d, %d\n",PRIO_GREEN,PRIO_RED,PRIO_TASK_CHANGER);

    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));


    OS_TaskCreate(&main_GreenBlinky,
                        OS_NULL(void),
                        stkTask_Green,
                        sizeof(stkTask_Green),
                        PRIO_GREEN);

    OS_TaskCreate(&main_RedBlinky,
                        OS_NULL(void),
                        stkTask_Red,
                        sizeof(stkTask_Red),
                        PRIO_RED);

    OS_TaskCreate(&main_ChangerTask,
                   OS_NULL(void),
                   stkTask_changer,
                   sizeof(stkTask_changer),
                   PRIO_TASK_CHANGER);

    printf("[Info]: OS Starts !\n\n");

    /* PrettyOS takes control from here. */
    OS_Run(BSP_CPU_FrequencyGet());

    /* Never executed. */
    return 0;
}
