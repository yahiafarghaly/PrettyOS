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
 * Purpose  : This example demonstrate a single producer/consumer problem which is solved
 *              using semaphore service.
 *            The example is inspired from a wikipedia article:
 *            ( https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem )
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
#define DUMMY_TASK_2_PRIO      (17U)
#define DUMMY_TASK_1_PRIO      (16U)
#define CONSUMER_TASK_PRIO     (14U)
#define PRODUCER_TASK_PRIO     (13U)
#define BUFFER_SIZE            (3U)

/*
*******************************************************************************
*                               Globals                                       *
*******************************************************************************
*/
OS_tSTACK task_stacks [5][40];

OS_SEM *fill_cnt;
OS_SEM *remaining_cnt;

unsigned char                   buffer[BUFFER_SIZE];
static volatile unsigned char    buff_Idx = 0;

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/
void OS_Hook_onIdle(void)
{
    BSP_CPU_WFI();
}
/*
*******************************************************************************
*                            Functions Prototypes                             *
*******************************************************************************
*/

void consumer(void* args);
void producer(void* args);
void dummy1(void*args);
void dummy2(void*args);

unsigned char produceItem(void);
void consumeItem(unsigned char item);

void putItemIntoBuffer(unsigned char item);
unsigned char removeItemFromBuffer();
void printBuffer(void);

void fakeWorkload(void);
/*
*******************************************************************************
*                            Functions Definitions                            *
*******************************************************************************
*/

int main() {

    BSP_HardwareSetup();

    BSP_UART_ClearVirtualTerminal();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_TICKS_PER_SEC);

    OS_Init(task_stacks[0], sizeof(task_stacks[0]));

    OS_TaskCreate(&consumer,
                        OS_NULL(void),
                        task_stacks[1],
                        sizeof(task_stacks[1]),
                        CONSUMER_TASK_PRIO);

    OS_TaskCreate(&producer,
                        OS_NULL(void),
                        task_stacks[2],
                        sizeof(task_stacks[2]),
                        PRODUCER_TASK_PRIO);

    OS_TaskCreate(&dummy1,
                        OS_NULL(void),
                        task_stacks[3],
                        sizeof(task_stacks[3]),
                        DUMMY_TASK_1_PRIO);
    OS_TaskCreate(&dummy2,
                        OS_NULL(void),
                        task_stacks[4],
                        sizeof(task_stacks[4]),
                        DUMMY_TASK_2_PRIO);

    printf("[Info]: Starts !\n\n");

    remaining_cnt = OS_SemCreate(BUFFER_SIZE);      /* Remaining space. */
    fill_cnt  = OS_SemCreate(0);                    /* Items produced.  */

    /* PrettyOS takes control from here. */
    OS_Run(BSP_CPU_FrequencyGet());

    /* Never executed. */
    return 0;
}

void dummy1(void*args)
{
    while(1)
    {
        BSP_LED_GreenOn();
        BSP_LED_BlueOff();
        OS_DelayTicks(50);
    }
}

void dummy2(void*args)
{
    while(1)
    {
        BSP_LED_GreenOff();
        BSP_LED_BlueOn();
        OS_DelayTicks(10);
    }
}

void consumer(void* args) {


    unsigned char item;
    while(1)
    {
        OS_SemPend(fill_cnt, 0);        /* Stop when buffer is empty */
        item = removeItemFromBuffer();
        OS_SemPost(remaining_cnt);      /* increase the available buffer size */

        consumeItem(item);
        fakeWorkload();
    }
}

void producer(void* args) {

    unsigned char item;
    while(1)
    {
        item = produceItem();

        OS_SemPend(remaining_cnt, 0);   /* Stop when buffer is full */
        putItemIntoBuffer(item);
        OS_SemPost(fill_cnt);           /* decrease buffer size (i.e increase write buffer index counter) */
    }
}


unsigned char produceItem(void)
{
    static unsigned char cnt = 0;

    if(cnt == 255)
    {
        cnt = 0;
    }

    cnt = cnt + 1;

    return cnt;
}

void consumeItem(unsigned char item)
{
    item = item;
}

void
putItemIntoBuffer(unsigned char item)
{
    buffer[buff_Idx] = item;
    buff_Idx = buff_Idx + 1;
    printf(" W R I T E => %d\n",item);
    printBuffer();
}

unsigned char
removeItemFromBuffer()
{
    unsigned char item;
    buff_Idx = buff_Idx - 1;
    item = buffer[buff_Idx];
    buffer[buff_Idx] = 0;
    printf(" R E A D <= %d\n",item);
    printBuffer();
    return item;
}

void printBuffer(void)
{
    unsigned char i = 0;
    for(i = 0; i < BUFFER_SIZE;++i)
    {
        printf("B[%d]=>[%d]\n",i,buffer[i]);
    }
    printf("---------\n");
}

void fakeWorkload(void)
{
    unsigned long volatile i;
    unsigned long volatile j;
    for(j = 0; j < 200U;j++)
    {
        for (i = 1500U; i != 0U; --i) {
        }
    }
}
