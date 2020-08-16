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
 * Purpose  : A dummy example which demonstrate the allocation and deallocation of memory blocks across two tasks.
 * 				there are two tasks. one which acts as it reads temperature data from sensor across time stamps.
 * 				and the second task which display the read result from the first task.
 *
 * 				For every temperature sensor read, there is an allocation occurs.
 * 				& for every temperature stored value in memory read, there is a deallocation occurs.
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
#define STACK_SIZE   		(60U)
#define PRIO_ANALOG_TASK  	(5U)
#define PRIO_DISPLAY_TASK  	(6U)

#define N_BLOCK		5U
#define	BLOCK_SZ	16U

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stkTask_ANALOG_TASK   [STACK_SIZE];
OS_tSTACK stkTask_DISPLAY_TASK  [STACK_SIZE];
OS_tSTACK stkTask_Idle  		[STACK_SIZE];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/

OS_MEMORY* 	AnalogReadBuffer;
void* 		memoryBlock[N_BLOCK];
CPU_t08U	memoryBuffer [N_BLOCK][BLOCK_SZ];	/* 5 elements, each is 16 bytes.		  */

volatile CPU_t16U UnreadCount = 0;
OS_MUTEX*	count_mux;

/*
*******************************************************************************
*                              OS Hooks functions                             *
*******************************************************************************
*/

void App_Hook_TaskIdle(void)
{
    /*  Application idle routine.    */
}

/*
*******************************************************************************
*                              Helpful Functions                              *
*******************************************************************************
*/


/*
*******************************************************************************
*                              Tasks Definitions                              *
*******************************************************************************
*/

CPU_t08U getTemperature ()
{
	static CPU_t08U current_temp = 0;
	current_temp += 5;
	return current_temp;
}

void task_ANALOG(void* args)
{
	CPU_t08U temperatureVal;
    OS_TIME 	 period 		= { 0, 0, 1,0};
    OS_TIME 	 timeStamp 		= { 0, 0, 0, 0};
    CPU_t16U memoryBlockIdx		= 0;
    void* allocatedMemoryBlock;

    while(1)
    {
        temperatureVal = getTemperature ();

        timeStamp.seconds 		+= (period.seconds + period.milliseconds/1000U + period.minutes*60 + period.hours*60*60);;

        allocatedMemoryBlock = OS_MemoryAllocateBlock(AnalogReadBuffer);

        if(allocatedMemoryBlock == OS_NULL(void))
        {
        	printf("Memory Block Allocation Fails with error [ %s ]\n",OS_StrLastErrIfFail());
        }
        else
        {
        	memoryBlock[memoryBlockIdx % N_BLOCK] = allocatedMemoryBlock;
        	*((CPU_t16U*)memoryBlock[memoryBlockIdx % N_BLOCK]) = ( 0x00FF & temperatureVal);						/* Store @ the right byte.		*/
        	*((CPU_t16U*)memoryBlock[memoryBlockIdx % N_BLOCK]) |= ( 0xFF00 & ((CPU_t08U)timeStamp.seconds << 8));	/* Store @ the left byte.		*/

        	++memoryBlockIdx;

        	OS_MutexPend(count_mux,0);
        	++UnreadCount;
        	OS_MutexPost(count_mux);

        	printf("Store temperature [ %d C ] @ %d\n",( 0x00FF & temperatureVal), (0x00FF & timeStamp.seconds) );
        }

        OS_DelayTime(&period);
    }
}

void task_DISPLAY(void* args)
{
    OS_TIME period = { 0, 0, 0, 500};
	CPU_t08U temperatureVal;
	CPU_t08U timeStampInSeconds;
    CPU_t16U memoryBlockIdx		= 0;

    while(1)
    {
    	if(!(UnreadCount % (N_BLOCK - 1)))
    	{
			while(UnreadCount)
			{
				if(memoryBlock[memoryBlockIdx % N_BLOCK] != OS_NULL(void))
				{
					temperatureVal 		= *((CPU_t16U*)memoryBlock[memoryBlockIdx % N_BLOCK]);
					timeStampInSeconds 	= ((*((CPU_t16U*)memoryBlock[memoryBlockIdx % N_BLOCK]) & 0xFF00 ) >> 8);
					OS_MemoryRestoreBlock(AnalogReadBuffer, memoryBlock[memoryBlockIdx % N_BLOCK]);
					++memoryBlockIdx;
					printf("\t(Display Task): T[ %d ] -> Temperature is %d C\n",timeStampInSeconds,temperatureVal);
					BSP_DelayMilliseconds(10);
				}
				else
				{
					printf("\t(Display Task): Invalid Memory Read for temperature.\n");
				}
				OS_MutexPend(count_mux,0);
				--UnreadCount;
				OS_MutexPost(count_mux);
			}
    	}

    	memoryBlockIdx = 0U;
        OS_DelayTime(&period);
    }
}

int main (void)
{

    /* Setup low level connected devices.   */
    BSP_HardwareSetup();

    /* Clear console terminal.              */
    BSP_UART_ClearVirtualTerminal();

    printf("\n\n");
    printf("                PrettyOS              \n");
    printf("                --------              \n");
    printf("[Info]: System Clock: %d MHz\n", BSP_CPU_FrequencyGet()/1000000);
    printf("[Info]: OS ticks per second: %d \n",OS_CONFIG_TICKS_PER_SEC);


    /* Initialize the Idle Task stack.      */
    OS_Init(stkTask_Idle, sizeof(stkTask_Idle));

    /* Create the tasks.                    */
    OS_TaskCreate(&task_ANALOG,
                  "Analog",
                  stkTask_ANALOG_TASK,
                  sizeof(stkTask_ANALOG_TASK),
                  PRIO_ANALOG_TASK);

    OS_TaskCreate(&task_DISPLAY,
                  "Display",
                  stkTask_DISPLAY_TASK,
                  sizeof(stkTask_DISPLAY_TASK),
                  PRIO_DISPLAY_TASK);

    AnalogReadBuffer = OS_MemoryPartitionCreate(memoryBuffer, N_BLOCK, BLOCK_SZ);

    printf("[Memory]: [ %s ] ---> Create %d bytes per block, Block Count = %d\n",OS_StrError(OS_ERRNO),BLOCK_SZ,N_BLOCK);

    count_mux = OS_MutexCreate(15,OS_MUTEX_PRIO_CEIL_ENABLE);

    printf("[Mutex] -> status: [ %s ]\n",OS_StrError(OS_ERRNO));

    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
    return 0;
}
