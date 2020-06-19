
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

OS_tCPU_DATA stack_GreenBlink [40];
OS_tCPU_DATA stack_BlueBlink  [40];
OS_tCPU_DATA stack_idleTask   [40];

static unsigned long GBlink_count,BBlink_count;

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

void
main_GreenBlinky(void* args) {
    GBlink_count = 0;
    while (1) {
        uint32_t volatile i;
        /* Simulate a workload */
        for (i = 1500U; i != 0U; --i) {
            BSP_ledGreenOn();
            BSP_ledBlueOff();
            BSP_ledRedOff();
        }
        App_printStat();
        OS_DelayTicks(100U);
        GBlink_count = ((GBlink_count + 1) > 5) ? 1 : (GBlink_count+1);
    }
}

void
main_BlueBlinky(void* args) {
    BBlink_count = 0;
    while (1) {
        uint32_t volatile i;
        for (i = 3*1500U; i != 0U; --i) {
            BSP_ledBlueOn();
            BSP_ledGreenOff();
            BSP_ledRedOff();
        }
        App_printStat();
        OS_DelayTicks(500U);
        ++BBlink_count;
    }
}

void OS_onIdle(void)
{
    App_printStat();
    BSP_ledGreenOff();
    BSP_ledBlueOff();
    BSP_WaitForInterrupt();
}

int main() {

    BSP_HardwareSetup();

    OS_Init(stack_idleTask, sizeof(stack_idleTask));

    OS_CreateTask(&main_GreenBlinky, OS_NULL, stack_GreenBlink, sizeof(stack_GreenBlink), 5U);

    OS_CreateTask(&main_BlueBlinky, OS_NULL, stack_BlueBlink, sizeof(stack_BlueBlink), 2U);


    App_minicom_SendClearScreen();
    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_SystemClockGet()/1000000);
    printf("[Info]: BSP ticks per second: %d \n",BSP_TICKS_PER_SEC);
    printf("[Info]: Starts !\n\n");

    /* transfer control to the RTOS to run the tasks */
    OS_Run();

    //return 0;
}


static inline void App_printStat()
{
    printf("Blinky1[G]: %i \t\t Blinky2[B]: %i\r",GBlink_count,BBlink_count);
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
