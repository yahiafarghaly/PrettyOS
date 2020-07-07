
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

#define GREEN_TASK_PRIO     (70U)
#define RED_TASK_PRIO       (50U)

CPU_tWORD stack_GreenBlink      [40];
CPU_tWORD stack_REDBlink        [40];
CPU_tWORD stack_idleTask        [40];

static unsigned long green_count,red_count;

OS_EVENT* sem;

/*
*******************************************************************************
*                            Functions Prototypes                             *
*******************************************************************************
*/
static inline void App_printStat();
static inline void App_minicom_SendClearScreen(void);

/*
*******************************************************************************
*                            Functions Definitions                            *
*******************************************************************************
*/

void loopFail(void)
{
    printf("[Info]: S T O P P E D \n");
    volatile unsigned char i = 1;
    while(i)
    {
        i = i;
    }
}

void
main_GreenBlinky(void* args) {
    green_count = 0;
    uint32_t volatile i;
    OS_tRet ret;
    while (1) {
        if(green_count == 5U)
        {
            printf("\nPend on sem\n");
            ret = OS_SemPend(sem, 500*3);
            switch(ret)
            {
            case OS_RET_OK:
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

        BSP_ledGreenOn();
        BSP_ledBlueOff();
        BSP_ledRedOff();

        for (i = 5*1500U; i != 0U; --i) {
        }

        OS_DelayTicks(100U);
    }
}

void
main_RedBlinky(void* args) {
    red_count = 0;
    uint32_t volatile i;

    while (1) {

        if(red_count == 10)
        {
            if(OS_SemPost(sem) != OS_RET_OK)
            {
                printf("Cannot post semaphore value\n");
            }
            else
            {
                printf("\nPost sem \n");
            }
        }

        ++red_count;

        BSP_ledRedOn();
        BSP_ledBlueOff();
        BSP_ledGreenOff();

        for (i = 5*1500U; i != 0U; --i) {
        }

        OS_DelayTicks(500U);
    }
}

void OS_Hook_onIdle(void)
{
    BSP_ledGreenOff();
    BSP_ledBlueOff();
    BSP_ledRedOff();
    App_printStat();
    BSP_WaitForInterrupt();
}

int main() {

    OS_tRet ret;

    BSP_HardwareSetup();

    App_minicom_SendClearScreen();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_SystemClockGet()/1000000);
    printf("[Info]: BSP ticks per second: %d \n",BSP_TICKS_PER_SEC);

    if(OS_RET_OK == OS_Init(stack_idleTask, sizeof(stack_idleTask)))
        printf("[Info]: Initialization ... Good\n");
    else
    {
        printf("[Info]: Initialization ... BAD \n");
        loopFail();
    }

    ret = OS_TaskCreate(&main_GreenBlinky,
                        OS_NULL,
                        stack_GreenBlink,
                        sizeof(stack_GreenBlink),
                        GREEN_TASK_PRIO);

    if(ret == OS_RET_OK)
        printf("[Info]: Green Task creation[prio = %d] ... Good\n",GREEN_TASK_PRIO);
    else
    {
        printf("[Info]: Green Task creation[prio = %d] ... BAD\n",GREEN_TASK_PRIO);
        loopFail();
    }

    ret = OS_TaskCreate(&main_RedBlinky,
                        OS_NULL,
                        stack_REDBlink,
                        sizeof(stack_REDBlink),
                        RED_TASK_PRIO);

    if(ret == OS_RET_OK)
        printf("[Info]: Red Task creation[prio = %d] ... Good\n",RED_TASK_PRIO);
    else
    {
        printf("[Info]: Red Task creation[prio = %d] ... BAD\n",RED_TASK_PRIO);
        loopFail();
    }

    sem = OS_SemCreate(0);
    if(sem == ((OS_SEM*)0U))
    {
        printf("Cannot Create semaphore\n");
        loopFail();
    }

    printf("[Info]: Starts !\n\n");

    /* PrettyOS takes control from here. */
    OS_Run();

    /* Never executed. */
    return 0;
}


static inline void App_printStat()
{
    printf("Blinky[G]: %i \t\t Blinky[R]: %i\r",green_count,red_count);
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
