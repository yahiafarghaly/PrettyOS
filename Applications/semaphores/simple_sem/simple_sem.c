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
 * Purpose  : This example demonstrate the use of basic operations of pend/post of
 *              a prettyOS semaphore.
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
#define GREEN_TASK_PRIO     (9U)
#define RED_TASK_PRIO       (5U)

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stack_GreenBlink [STACK_SIZE];
OS_tSTACK stack_REDBlink   [STACK_SIZE];
OS_tSTACK stack_idleTask   [STACK_SIZE];

/*
*******************************************************************************
*                               Globals                                       *
*******************************************************************************
*/
static volatile CPU_t32U green_count    = 0U;
static volatile CPU_t32U red_count      = 0U;

OS_EVENT* sem;

/*
*******************************************************************************
*                            Functions Definitions                            *
*******************************************************************************
*/
static inline void App_printStat()
{
    printf("Blinky[G]: %i \t\t Blinky[R]: %i\r",green_count,red_count);
}

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/
void OS_Hook_onIdle(void)
{
    BSP_LED_GreenOff();
    BSP_LED_BlueOff();
    BSP_LED_RedOff();
    App_printStat();
    BSP_CPU_WFI();
}


/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

void
main_GreenBlinky(void* args) {
    green_count = 0;
    CPU_t16U i;
    while (1) {
        if(green_count == 5U)
        {
            printf("\nPend on sem\n");
            OS_SemPend(sem, 500*3);
            switch(OS_ERRNO)
            {
            case OS_ERR_NONE:
                printf("\nResume Green Blinky\n");
                break;
            case OS_ERR_EVENT_TIMEOUT:
                printf("\nResume Green Blinky, Timeout\n");
                break;
            default:
                printf("\nResume Green Blinky, Undefined return\n");
                break;
            }
        }

        ++green_count;

        BSP_LED_GreenOn();
        BSP_LED_BlueOff();
        BSP_LED_RedOff();

        for (i = 5*1500U; i != 0U; --i) {
        }

        OS_DelayTicks(100U);
    }
}

void
main_RedBlinky(void* args) {
    red_count = 0;
    CPU_t16U i;

    while (1) {

        if(red_count == 10)
        {
        	OS_SemPost(sem);
            if(OS_ERRNO != OS_ERR_NONE)
            {
                printf("Cannot post semaphore value\n");
            }
            else
            {
                printf("\nPost sem \n");
            }
        }

        ++red_count;

        BSP_LED_RedOn();
        BSP_LED_BlueOff();
        BSP_LED_GreenOff();

        for (i = 5*1500U; i != 0U; --i) {
        }

        OS_DelayTicks(500U);
    }
}


int main() {

    OS_tRet ret;

    BSP_HardwareSetup();

    BSP_UART_ClearVirtualTerminal();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);

    if(OS_ERR_NONE == OS_Init(stack_idleTask, sizeof(stack_idleTask)))
        printf("[Info]: Initialization ... Good\n");
    else
    {
        printf("[Info]: Initialization ... BAD \n");
    }

    ret = OS_TaskCreate(&main_GreenBlinky,
                        OS_NULL(void),
                        stack_GreenBlink,
                        sizeof(stack_GreenBlink),
                        GREEN_TASK_PRIO);

    if(ret == OS_ERR_NONE)
        printf("[Info]: Green Task creation[prio = %d] ... Good\n",GREEN_TASK_PRIO);
    else
    {
        printf("[Info]: Green Task creation[prio = %d] ... BAD\n",GREEN_TASK_PRIO);
    }

    ret = OS_TaskCreate(&main_RedBlinky,
                        OS_NULL(void),
                        stack_REDBlink,
                        sizeof(stack_REDBlink),
                        RED_TASK_PRIO);

    if(ret == OS_ERR_NONE)
        printf("[Info]: Red Task creation[prio = %d] ... Good\n",RED_TASK_PRIO);
    else
    {
        printf("[Info]: Red Task creation[prio = %d] ... BAD\n",RED_TASK_PRIO);
    }

    sem = OS_SemCreate(0);
    if(sem == ((OS_SEM*)0U))
    {
        printf("Cannot Create semaphore\n");
    }

    printf("[Info]: Starts !\n\n");

    /* PrettyOS takes control from here. */
    OS_Run(BSP_CPU_FrequencyGet());

    /* Never executed. */
    return 0;
}

