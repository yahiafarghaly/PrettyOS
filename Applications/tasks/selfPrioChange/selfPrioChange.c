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
 * Purpose  : This example shows that the task can change its own priority by itself in the runtime.
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
#define PRIO_ONE            (20U)
#define PRIO_TWO            (30U)
#define PRIO_THREE          (10U)

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stkTask_Idle    [STACK_SIZE];
OS_tSTACK stkTask_one     [STACK_SIZE];
OS_tSTACK stkTask_two     [STACK_SIZE];
OS_tSTACK stkTask_three   [STACK_SIZE];

/*
*******************************************************************************
*                               Globals                                       *
*******************************************************************************
*/

/*
*******************************************************************************
*                            Functions Prototypes                             *
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
three (void* args) {

    CPU_t08U three_flag = 1U;
    CPU_t08U count = 0;
    OS_TIME period = { 0, 0, 1, 0};     /*  Every 1 seconds.  */
    OS_PRIO prio = PRIO_THREE;

    while (1) {

        printf("\n -- Number three -- \n");

        if(count == 4U && three_flag)
        {
            if(OS_ERR_NONE != OS_TaskChangePriority(PRIO_THREE, prio))
            {
                printf("\n[ F a i l ] ==> Change: PRIO_GREEN = [%d]->[%d]\n",prio, prio + 10U);
            }
            else
            {
                printf("\nChange: PRIO_GREEN = [%d]->[%d]\n",prio, prio + 10U);
                three_flag = 0;
            }
        }
        else
        {
            prio = prio + PRIO_THREE;
        }

        ++count;

        OS_DelayTime(&period);
    }
}

void one (void* args)
{
    OS_TIME period = { 0, 0, 1, 0};
    while(1)
    {
        printf("\n -- Number one -- \n");
        OS_DelayTime(&period);
    }
}

void two (void* args)
{
    OS_TIME period = { 0, 0, 1, 0};
    while(1)
    {
        printf("\n -- Number two -- \n");
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
    printf("[Info]: OS ticks per second: %d \n",OS_TICKS_PER_SEC);

    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));


    OS_TaskCreate(&three,
                        OS_NULL(void),
                        stkTask_three,
                        sizeof(stkTask_three),
                        PRIO_THREE);

    OS_TaskCreate(&one,
                        OS_NULL(void),
                        stkTask_one,
                        sizeof(stkTask_one),
                        PRIO_ONE);

    OS_TaskCreate(&two,
                        OS_NULL(void),
                        stkTask_two,
                        sizeof(stkTask_two),
                        PRIO_TWO);

    printf("[Info]: OS Starts !\n\n");

    /* PrettyOS takes control from here. */
    OS_Run(BSP_CPU_FrequencyGet());

    /* Never executed. */
    return 0;
}
