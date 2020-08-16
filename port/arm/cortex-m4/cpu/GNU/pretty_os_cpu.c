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
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_arch.h"

/*
*******************************************************************************
*                               Global/External Data                          *
*******************************************************************************
*/
extern void OS_TaskReturn (void);
extern void OS_TimerTick  (void);
extern void OS_IntEnter   (void);
extern void OS_IntExit    (void);

/*
*******************************************************************************
*                 Structures/Macros to access SysTick                         *
*******************************************************************************
*/

/*  Structure type to access the System Timer (SysTick). */
typedef struct
{
    volatile CPU_t32U CTRL;                   /* Offset: 0x000 (R/W)  SysTick Control and Status Register   */
    volatile CPU_t32U LOAD;                   /* Offset: 0x004 (R/W)  SysTick Reload Value Register         */
    volatile CPU_t32U VAL;                    /* Offset: 0x008 (R/W)  SysTick Current Value Register        */
} SysTick_Type;

#define SYSTICK_BASE                    (0xE000E010U)
#define NVIC_SYSPRI3                    (0xE000ED20U)
#define NVIC_SYSTICK_PRIO         (CPU_CONFIG_SYSTICK_PRIO)

#define SysTick                   ((SysTick_Type*)(SYSTICK_BASE))

/*
 * Function:  OS_CPU_TaskInit
 * --------------------
 * Initialize the stack frame of the task being created.
 * This function is a processor specific.
 *
 * Called By: OS_CreateTask()
 *
 * Arguments:
 *          TASK_Handler            is a function pointer to the task code.
 *          params                  is a pointer to the user supplied data which is passed to the task.
 *          pStackBase              is a pointer to the bottom of the task stack.
 *          stackSize               is the task stack size.
 *
 * Returns: The new location of the top of stack after the processor insert the registers in the stack
 *          in the proper of order as specified in the Technical Manual of the CPU.
 *
 * Notes:   ARM Cortex-M stack grows from high to low memory address.
 */
CPU_tSTK*
OS_CPU_TaskStackInit(void (*TASK_Handler)(void* params),
                             void *params,
                             CPU_tSTK* pStackBase,
                             CPU_tSTK_SIZE  stackSize)
{
    CPU_tSTK* sp;
    /*
     * Move to the top of the task stack and round down it 8-bytes to
     * be Aligned.
     * */
    sp = (CPU_tSTK*)(((CPU_tSTK)pStackBase + stackSize) & 0xFFFFFFF8U);
    /* Stacking the registers as auto saved at exception entrance. */
    *(--sp) = (1U << 24);  /* xPSR ( Thumb State ) */
    *(--sp) = (CPU_tWORD)TASK_Handler;      /* PC (Task Entry Point) */
    *(--sp) = (CPU_tWORD)OS_TaskReturn;     /* LR (Task End Point)   */
    *(--sp) = 0x0000000CU;                  /* R12 */
    *(--sp) = 0x00000003U;                  /* R3  */
    *(--sp) = 0x00000002U;                  /* R2  */
    *(--sp) = 0x00000001U;                  /* R1  */
    *(--sp) = (CPU_tWORD)params;            /* R0: Argument  */
    /* Additionally, fake registers R4-R11 */
    *(--sp) = 0x0000000BU;                  /* R11 */
    *(--sp) = 0x0000000AU;                  /* R10 */
    *(--sp) = 0x00000009U;                  /* R9 */
    *(--sp) = 0x00000008U;                  /* R8 */
    *(--sp) = 0x00000007U;                  /* R7 */
    *(--sp) = 0x00000006U;                  /* R6 */
    *(--sp) = 0x00000005U;                  /* R5 */
    *(--sp) = 0x00000004U;                  /* R4 */

    return (sp);
}

/*
 * Function:  OS_CPU_SystemTimerHandler
 * --------------------
 * Handle the system tick interrupt which is used for signaling the system tick
 * to OS_TimerTick().
 *
 * Arguments    : None.
 *
 * Returns      : None.
 *
 * Note(s)      :   1) this function must be placed on entry 15 of the ARM Cortex-M4 vector table.
 */
void OS_CPU_SystemTimerHandler  (void)
{
    CPU_SR_ALLOC();

    OS_CRTICAL_BEGIN();

    OS_IntEnter();          /* Notify that we are entering an ISR.          */

    OS_CRTICAL_END();

    OS_TimerTick();         /* Signal the tick to the OS_timerTick().       */

    OS_IntExit();           /* Notify that we are leaving the ISR.          */
}

/*
 * Function:  OS_CPU_SystemTimerSetup
 * --------------------
 * Initialize the timer which will be used as a system ticker for the OS.
 *
 * Arguments    :   ticks   is the number of ticks count between two OS tick interrupts.
 *
 * Returns      :   None.
 */
void  OS_CPU_SystemTimerSetup (CPU_t32U ticks)
{
    SysTick->LOAD  = (ticks - 1U);      /* Load the value which will reach to zero, Max is 0xFFFFFF      */
    SysTick->VAL   = 0U;                /* Clear any value in the STCURRENT register.                    */
    SysTick->CTRL |= (0x04U);           /* Enable Clock source to be from the system clock.              */
    SysTick->CTRL |= (0x01U);           /* Enable SysTick to operate in multi-shot way.                  */

    (*((volatile CPU_t32U*)NVIC_SYSPRI3)) |= (NVIC_SYSTICK_PRIO << 29U); /* Set the priority of SysTick interrupt */

    SysTick->CTRL |= (0x02U);           /* Finally, Enable Interrupt generation when count reaches 0     */
}

/*
*******************************************************************************
*                           CPU Hook Functions                                *
*******************************************************************************
*/

struct OS_TASK_TCB; /* Forward declaration instead of including the pretty_os.h file as i don't use it here. */

void OS_CPU_Hook_Init (void)
{
    /* Init certain CPU module.                                 */
}

void OS_CPU_Hook_TaskCreated (struct OS_TASK_TCB*   ptcb)
{
    /* ...                                                      */
}

void OS_CPU_Hook_TaskDeleted (struct OS_TASK_TCB*   ptcb)
{
    /* ...                                                      */
}

void OS_CPU_Hook_Idle (void)
{
    /* You may reduce the CPU utilization.                      */
}

void OS_CPU_Hook_ContextSwitch (void)
{
    /* This function should be called inside the OS_CPU_PendSVHandler() if you desire to do something when context switch is occurred.  */
    /* You may want to count the number of context switches.    */
}

void OS_CPU_Hook_TimeTick (void)
{
    /* You may want to record ticks elapsed here.               */
}

