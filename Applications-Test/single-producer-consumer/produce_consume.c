
/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include <stdint.h>
#include <pretty_os.h>
#include <bsp.h>
#include <uartstdio.h>

/*
*******************************************************************************
*                               Globals                                       *
*******************************************************************************
*/

#define DUMMY_TASK_2_PRIO      (4U)
#define DUMMY_TASK_1_PRIO      (3U)
#define CONSUMER_TASK_PRIO     (2U)
#define PRODUCER_TASK_PRIO     (1U)

#define BUFFER_SIZE            (5U)

CPU_tWORD task_stacks [5][40];

unsigned char buffer[BUFFER_SIZE];
static unsigned char buff_Idx = 0;

OS_SEM *fill_cnt;
OS_SEM *remaining_cnt;

/*
*******************************************************************************
*                            Functions Prototypes                             *
*******************************************************************************
*/

static inline void App_minicom_SendClearScreen(void);
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

    App_minicom_SendClearScreen();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_SystemClockGet()/1000000);
    printf("[Info]: BSP ticks per second: %d \n",BSP_TICKS_PER_SEC);

    OS_Init(task_stacks[0], sizeof(task_stacks[0]));

    OS_CreateTask(&consumer,
                        OS_NULL,
                        task_stacks[1],
                        sizeof(task_stacks[1]),
                        CONSUMER_TASK_PRIO);

    OS_CreateTask(&producer,
                        OS_NULL,
                        task_stacks[2],
                        sizeof(task_stacks[2]),
                        PRODUCER_TASK_PRIO);

    OS_CreateTask(&dummy1,
                        OS_NULL,
                        task_stacks[3],
                        sizeof(task_stacks[3]),
                        DUMMY_TASK_1_PRIO);
    OS_CreateTask(&dummy2,
                        OS_NULL,
                        task_stacks[4],
                        sizeof(task_stacks[4]),
                        DUMMY_TASK_2_PRIO);

    printf("[Info]: Starts !\n\n");

    remaining_cnt = OS_SemCreate(BUFFER_SIZE);   /* Remaining space. */
    fill_cnt  = OS_SemCreate(0);             /* Items produced. */

    /* PrettyOS takes control from here. */
    OS_Run();

    /* Never executed. */
    return 0;
}

void dummy1(void*args)
{
    while(1)
    {
        BSP_ledGreenOn();
        BSP_ledBlueOff();
        OS_DelayTicks(50);
    }
}

void dummy2(void*args)
{
    while(1)
    {
        BSP_ledGreenOff();
        BSP_ledBlueOn();
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

void OS_Hook_onIdle(void)
{
    BSP_WaitForInterrupt();
}

static inline void App_minicom_SendClearScreen(void)
{
    /*The command sequence for minicom to clear the screen is Esc[2J
     * which can be interpreted as
     *  Esc     the ASCII Escape character, value 0x1B.
     *  [       the ASCII left square brace character, value 0x5B.
     *  2       the ASCII character for numeral 2, value 0x32.
     *  J       the ASCII character for the letter J, value 0x4A.
     * */
    BSP_UARTSend(0x1B);
    BSP_UARTSend(0x5B);
    BSP_UARTSend(0x32);
    BSP_UARTSend(0x4A);
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
