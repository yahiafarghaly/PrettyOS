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

#ifndef __PRETTY_ARCH_H_
#define __PRETTY_ARCH_H_

#ifdef __cplusplus
extern "C" {
#endif


/*
*******************************************************************************
*                             CPU Configurations                              *
*******************************************************************************
*/

/************************* OS Tick Interrupt Priority *************************/
/*
 * If the system doesn't require any real time interrupts, then set the tick
 * interrupt to the highest. It will not affect adversely other system operations.
 * Except that, then set it to the highest from any other non critical interrupts. */
#define OS_CPU_CONFIG_SYSTICK_PRIO          (0U)

/*
*******************************************************************************
*                               Compiler Specific                             *
*******************************************************************************
*/
#define CPU_CountLeadZeros          __builtin_clz   /* If no CPU clz instruction is not supported, implement it in C */
#define CPU_NumberOfBitsPerWord            (32U)    /* Until I found a magical way to calculate it automatically.    */

/*
*******************************************************************************
*                               DATA TYPES                                    *
*******************************************************************************
*/

typedef unsigned char  CPU_t08U;                    /* Unsigned  8 bit quantity                             */
typedef signed   char  CPU_t08S;                    /* Signed    8 bit quantity                             */
typedef unsigned short CPU_t16U;                    /* Unsigned 16 bit quantity                             */
typedef signed   short CPU_t16S;                    /* Signed   16 bit quantity                             */
typedef unsigned int   CPU_t32U;                    /* Unsigned 32 bit quantity                             */
typedef signed   int   CPU_t32S;                    /* Signed   32 bit quantity                             */

typedef void*          CPU_tPtr;                    /* Pointer Type                                         */
typedef unsigned int   CPU_tWORD;                   /* Define size of CPU word size (In ARM-CM4 = 32 bits)  */
typedef unsigned int   CPU_tPSR;                    /* Define size of CPU status register (PSR = 32 bits)   */



/*
*******************************************************************************
*                             Critical Section Management                     *
*******************************************************************************
*/
#define OS_CRTICAL_BEGIN()      __asm volatile ("cpsid i" : : : "memory");
#define OS_CRTICAL_END()        __asm volatile ("cpsie i" : : : "memory");


/*
*******************************************************************************
*                             Arch Specific Functions Prototypes              *
*******************************************************************************
*/

/*
 * Function:  OS_CPU_ContexSwitch
 * --------------------
 * Perform a context switch from a task level.
 */
void OS_CPU_ContexSwitch(void);

/*
 * Function:  OS_CPU_InterruptContexSwitch
 * --------------------
 * Perform a context switch from an interrupt level.
 */
void OS_CPU_InterruptContexSwitch(void);

/*
 * Function:  OS_CPU_FirstStart
 * --------------------
 * Setup the architecture specific registers to trigger a context switch
 * to the first task to start.
 *
 * It should do the following ( General Speaking ):
 *      - Setup an interrupt handler for performing the context switch.
 *      - Set OS_Running to OS_TRUE.
 *      - Trigger this interrupt handler to start the first task.
 *      - Enable CPU interrupts.
 */
void OS_CPU_FirstStart(void);

/*
 * Function:  OS_CPU_TaskInit
 * --------------------
 * Initialize the stack frame of the task being created.
 * This function is a processor specific.
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
 * Notes:   Be aware of how the CPU stacking happens in the memory space.
 */
CPU_tWORD* OS_CPU_TaskInit(void (*TASK_Handler)(void* params),
                             void *params,
                             CPU_tWORD* pStackBase,
                             CPU_tWORD  stackSize);

/*
 * Function:  OS_CPU_SystemTimerHandler
 * --------------------
 * Handle the system tick interrupt which is used for signaling the system tick
 * to OS_TimerTick().
 *
 * Arguments    :   None.
 *
 * Returns      :   None.
 */
void OS_CPU_SystemTimerHandler (void);

/*
 * Function:  OS_CPU_SystemTimerSetup
 * --------------------
 * Initialize the timer which will be used as a system ticker for the OS.
 *
 * Arguments    :   ticks   is the number of ticks count between two OS tick interrupts.
 *
 * Returns      :   None.
 */
void  OS_CPU_SystemTimerSetup (CPU_t32U ticks);

#ifdef __cplusplus
}
#endif

#endif /* __PRETTY_ARCH_H_ */
