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
 * Purpose  : This example shows is similar to the priority inversion example. listed in mutex/priority_inversion/priority_inversion.c
 *
 *            The only difference is we have two tasks (VH and M ) which are exchanging message data in the same mailbox.
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
#define STACK_SIZE   (60U)
#define PRIO_L_TASK  (5U)
#define PRIO_M_TASK  (6U)
#define PRIO_H_TASK  (7U)
#define PRIO_PCP     (8U)
#define PRIO_VH_TASK  (10U)
/*
*******************************************************************************
*                              Tasks Stacks                                   *
*******************************************************************************
*/
OS_tSTACK stkTask_L     [STACK_SIZE];
OS_tSTACK stkTask_M     [STACK_SIZE];
OS_tSTACK stkTask_H     [STACK_SIZE];
OS_tSTACK stkTask_VH    [STACK_SIZE];
OS_tSTACK stkTask_Idle  [STACK_SIZE];

/*
*******************************************************************************
*                                 Globals                                     *
*******************************************************************************
*/
OS_MUTEX* 	virtual_line_lock;
OS_MAILBOX*	mailbox;

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

void task_L(void* args)
{
    OS_TIME period = { 0, 0, 0, 500};
    while(1)
    {
        printf("\nSending '%s' message\n",(char*)args);
        /*  Sending the message takes roughly +2 seconds. */
        OS_MutexPend(virtual_line_lock, 0U);

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(' ');
        BSP_UART_SendByte(((char*)args)[0]);
        BSP_UART_SendByte(' ');

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(' ');
        BSP_UART_SendByte(((char*)args)[1]);
        BSP_UART_SendByte(' ');

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(' ');
        BSP_UART_SendByte(((char*)args)[2]);
        BSP_UART_SendByte(' ');

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(' ');
        BSP_UART_SendByte(((char*)args)[3]);
        BSP_UART_SendByte(' ');

        OS_MutexPost(virtual_line_lock);


        OS_DelayTime(&period);
    }
}

void task_M(void* args)
{
    OS_TIME period = { 0, 0, 0, 250};
    int i = 0U;
    int message = 0U;
    int recieved_message;

    while(1)
    {
    	printf("M: Waiting for message arrival ...\n");
    	recieved_message = (int)OS_MailBoxPend(mailbox,0);

        switch(OS_ERRNO)
        {
        case OS_ERR_EVENT_TIMEOUT:
        	printf("M: Message arrival timeout.\n");
        	break;
        case OS_ERR_NONE:
        	printf("M: Received a message ( [ %d ] )\n",recieved_message);
        	break;
        default:
        	printf("M: Receive Error [ %s ] .\n",OS_StrError(OS_ERRNO));
        	break;
        }

        message += 3U;
        OS_MailBoxPost(mailbox,(void*)message);

        switch(OS_ERRNO)
        {
        case OS_ERR_MAILBOX_POST_NULL:
        	printf("M: Cannot Posting NULL.\n");
        	break;
        case OS_ERR_MAILBOX_FULL:
        	printf("M: Cannot Post, MailBox is Full.\n");
        	break;
        case OS_ERR_NONE:
            printf("M: Posting [ %d ] to the mailbox\n",message);
            break;
        default:
        	printf("M: Post Error [ %s ] .\n",OS_StrError(OS_ERRNO));
        	break;
        }

        /* The execution takes roughly +1 second.  */
        BSP_UART_SendByte('[');         /* Starts with close bracket.   */
        for(i = 0; i < 10U; i++)
        {
            BSP_UART_SendByte(48 + i);  /* Print digits [0 --> 9]'      */
            BSP_DelayMilliseconds(100);
        }
        BSP_UART_SendByte(']');         /* Ends with a bracket.         */
        OS_DelayTime(&period);
    }
}

void task_H(void* args)
{
    OS_TIME period = { 0, 0, 1U, 0};
    while(1)
    {
        printf("\nSending '%s' message\n",(char*)args);
        /*  Sending the message takes roughly +1.5 seconds. */
        OS_MutexPend(virtual_line_lock, 0U);

        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(((char*)args)[0]);
        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(((char*)args)[1]);
        BSP_DelayMilliseconds(500);
        BSP_UART_SendByte(((char*)args)[2]);
        BSP_UART_SendByte('\n');
        BSP_UART_SendByte('\r');

        OS_MutexPost(virtual_line_lock);

        OS_DelayTime(&period);
    }
}

void task_VH(void* args)
{
    OS_TIME period = { 0, 0, 5U, 0};

    int message = 0U;
    int received_message;

    while(1)
    {
        printf("\n--- VH ---\n");

        printf("VH: Waiting for message arrival for 1 tick ...\n");

        received_message = (int)OS_MailBoxPend(mailbox,1);

        switch(OS_ERRNO)
        {
        case OS_ERR_EVENT_TIMEOUT:
        	printf("VH: No available message.\n");
        	break;
        case OS_ERR_NONE:
        	printf("VH: Received a message ( [ %d ] )\n",received_message);
        	break;
        default:
        	printf("VH: Receive Error [ %s ] .\n",OS_StrError(OS_ERRNO));
        	break;
        }


        OS_MailBoxPost(mailbox,(void*)message);

        switch(OS_ERRNO)
        {
        case OS_ERR_MAILBOX_POST_NULL:
        	printf("VH: Cannot Posting NULL.\n");
        	break;
        case OS_ERR_MAILBOX_FULL:
        	printf("VH: Cannot Post, MailBox is Full.\n");
        	break;
        case OS_ERR_NONE:
            printf("VH: Posting [ %d ] to the mailbox\n",message);
            break;
        default:
        	printf("VH: Post Error [ %s ] .\n",OS_StrError(OS_ERRNO));
        	break;
        }

        message += 5U;

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
    OS_TaskCreate(&task_L,
                  "TEST",
                  stkTask_L,
                  sizeof(stkTask_L),
                  PRIO_L_TASK);

    OS_TaskCreate(&task_M,
                  "The Middle Task",
                  stkTask_M,
                  sizeof(stkTask_M),
                  PRIO_M_TASK);

    OS_TaskCreate(&task_H,
                  "SOS",
                  stkTask_H,
                  sizeof(stkTask_H),
                  PRIO_H_TASK);

    OS_TaskCreate(&task_VH,
                  "SOS",
                  stkTask_VH,
                  sizeof(stkTask_VH),
                  PRIO_VH_TASK);

    virtual_line_lock = OS_MutexCreate(PRIO_PCP, OS_MUTEX_PRIO_CEIL_ENABLE);
    if(virtual_line_lock == OS_NULL(OS_MUTEX))
    {
        printf("\nError Creating `virtual_line_lock` Mutex\n");
        printf("Error message: %s\n",OS_StrError(OS_ERRNO));
    }

    mailbox = OS_MailBoxCreate(OS_NULL(void));
    if(mailbox == OS_NULL(OS_MAILBOX))
    {
        printf("\nError Creating `mailbox`\n");
        printf("Error message: %s\n",OS_StrError(OS_ERRNO));
    }

    printf("[Info]: OS Starts !\n\n");

    /*  Transfer control to the RTOS to run the tasks.   */
    OS_Run(BSP_CPU_FrequencyGet());

    /*       Should never reach here.   */
    for(;;);
    return 0;
}
