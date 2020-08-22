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
 * Purpose  :
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
#define PRIO_Master_TASK  	(15U)
#define PRIO_Set_TASK  		(6U)
#define PRIO_Clr_TASK  		(7U)
#define PRIO_Response_TASK  (10U)


#define BIT_0	(1 << 0)
#define BIT_1	(1 << 1)
#define BIT_2	(1 << 2)
#define BIT_3	(1 << 3)
#define BIT_4	(1 << 4)
#define BIT_5	(1 << 5)
#define BIT_6	(1 << 6)
#define BIT_7	(1 << 7)

/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stkTask_Master     	[STACK_SIZE];
OS_tSTACK stkTask_Set     		[STACK_SIZE];
OS_tSTACK stkTask_Clr     		[STACK_SIZE];
OS_tSTACK stkTask_Response    	[STACK_SIZE];
OS_tSTACK stkTask_Idle  		[STACK_SIZE];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/\

OS_EVENT_FLAG_GRP* eflagGrp_set;
OS_EVENT_FLAG_GRP* eflagGrp_clr;

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

void ResponseTask(void* args)
{
    OS_TIME period = { 0, 0, 1U, 0};
    OS_FLAG flags;
    printf("%s Started !\n\n",(char*)args);

    while(1)
    {
    	flags = OS_EVENT_FlagPend(eflagGrp_set,(OS_FLAG)(BIT_0 | BIT_2),OS_FLAG_WAIT_SET_ANY,OS_TRUE,10);

    	if((flags & (BIT_0 | BIT_2)) == (BIT_0 | BIT_2))
    	{
    		printf("Both BIT_0 & BIT_2 have been SET.\n");
    	}
    	else if ((flags & BIT_0) != 0U)
    	{
    		printf("BIT_0 has been SET\n");
    	}
    	else if ((flags & BIT_2) != 0U)
    	{
    		printf("BIT_2 has been SET\n");
    	}
    	else
    	{
    		printf("Neither BIT_0 nor BIT_2 has been SET due to a timeout\n");
    		OS_TaskDelete(PRIO_Response_TASK);
    	}

        OS_DelayTime(&period);
    }
}

void SET_Task(void* args)
{
    OS_TIME period = { 0, 0, 1U, 0};
    volatile CPU_t08U test_number = 0;
    volatile OS_FLAG posted_flags;
    printf("%s Started !\n\n",(char*)args);

    while(1)
    {

    	switch(test_number)
    	{
    	/* 						Testing Set BIT_0												*/
    	case 0:
    		posted_flags = OS_EVENT_FlagPost(eflagGrp_set,BIT_0,OS_FLAG_SET);
        	printf("[ %d ] Test#%d : SET Flags [ 0x%x ].\n",OS_TickTimeGet(),test_number,posted_flags);
    		break;

    	/* 						Testing Set BIT_2												*/
    	case 1:
    		posted_flags = OS_EVENT_FlagPost(eflagGrp_set,BIT_2,OS_FLAG_SET);
        	printf("[ %d ] Test#%d : SET Flags [ 0x%x ].\n",OS_TickTimeGet(),test_number,posted_flags);
    		break;

    	/* 						Testing Set both BIT_0 & BIT_2									*/
    	case 2:
    		posted_flags = OS_EVENT_FlagPost(eflagGrp_set,BIT_0 | BIT_2,OS_FLAG_SET);
        	printf("[ %d ] Test#%d : SET Flags [ 0x%x ].\n",OS_TickTimeGet(),test_number,posted_flags);
    		break;

    	/* 						Testing Not setting BIT_0 or BIT_2 timeout						*/
    	case 3:
    		posted_flags = 0;
        	printf("[ %d ] Test#%d : Ignore SET Flags [ 0x%x ].\n",OS_TickTimeGet(),test_number,posted_flags);
            OS_DelayTime(&period);
    		break;

    	/* 						Finally, Ignore other tests the test.							*/
    	default:
    		break;
    	}

    	posted_flags = 0;
        ++test_number;

        OS_DelayTime(&period);
    }
}

void CLR_TASK(void* args)
{
    OS_TIME period = { 0, 0, 1U, 0};

    while(1)
    {
        OS_DelayTime(&period);
    }
}


void MasterTask(void* args)
{
    OS_TIME period = { 0, 0, 4U, 0};
    OS_FLAG set_pattern = 0;
    OS_FLAG clr_pattern = 0;

    printf("%s Started !\n",(char*)args);

    OS_TaskCreate(&ResponseTask,
                  "ResponseTask",
                  stkTask_Response,
                  sizeof(stkTask_Response),
                  PRIO_Response_TASK);

    OS_TaskCreate(&SET_Task,
                  "SET_Task",
                  stkTask_Set,
                  sizeof(stkTask_Set),
                  PRIO_Set_TASK);
//
//    OS_TaskCreate(&CLR_TASK,
//                  "CLR_TASK",
//                  stkTask_Clr,
//                  sizeof(stkTask_Clr),
//                  PRIO_Clr_TASK);

    eflagGrp_set = OS_EVENT_FlagCreate(set_pattern);
    if(eflagGrp_set == OS_NULL(OS_EVENT_FLAG_GRP))
    {
        printf("\nError Creating `eflagGrp_set` \n");
        printf("Error message: %s\n",OS_StrError(OS_ERRNO));
    }

    eflagGrp_clr = OS_EVENT_FlagCreate(set_pattern);
    if(eflagGrp_clr == OS_NULL(OS_EVENT_FLAG_GRP))
    {
        printf("\nError Creating `eflagGrp_clr` \n");
        printf("Error message: %s\n",OS_StrError(OS_ERRNO));
    }

    printf(".................\n");
    while(1)
    {
        printf(" Master Call \n");
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
    OS_TaskCreate(&MasterTask,
                  "MasterTask",
                  stkTask_Master,
                  sizeof(stkTask_Master),
                  PRIO_Master_TASK);

    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
    return 0;
}
