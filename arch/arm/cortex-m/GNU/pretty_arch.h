/*
 * pretty_arch.h
 *
 *  Created on: Jun 8, 2020
 *      Author: yf
 */

#ifndef __PRETTY_ARCH_H_
#define __PRETTY_ARCH_H_

/*
*******************************************************************************
*                               Compiler Specific                             *
*******************************************************************************
*/
#define OS_CPU_CountLeadZeros              __builtin_clz
#define OS_CPU_likely(x)                   __builtin_expect(!!(x), 1)
#define OS_CPU_unlikely(x)                 __builtin_expect(!!(x), 0)
#define OS_CPU_WORD_SIZE_IN_BITS            (32U)

/*
*******************************************************************************
*                               DATA TYPES                                    *
*******************************************************************************
*/

typedef unsigned char  OS_t8U;                    /* Unsigned  8 bit quantity */
typedef signed   char  OS_t8S;                    /* Signed    8 bit quantity */
typedef unsigned short OS_t16U;                   /* Unsigned 16 bit quantity */
typedef signed   short OS_t16S;                   /* Signed   16 bit quantity */
typedef unsigned int   OS_t32U;                   /* Unsigned 32 bit quantity */
typedef signed   int   OS_t32S;                   /* Signed   32 bit quantity */

typedef void*          OS_tptr;                   /* Pointer Type */
typedef unsigned int   OS_tCPU_DATA;              /* Define size of CPU word size (In ARM-CM4 = 32 bits) */
typedef unsigned int   OS_tCPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */

typedef OS_tCPU_DATA   OS_tRet;                   /* Fit to the easiest type of memory for CPU  */

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
 * Function:  OS_CPU_PendSVHandler
 * --------------------
 * This where the real context switch happens.
 *
 * Note: Make the PendSVHandler entry in the vector table points to this function.
 */
void OS_CPU_PendSVHandler(void);

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
OS_tCPU_DATA* OS_CPU_TaskInit(void (*TASK_Handler)(void* params),
                             void *params,
                             OS_tCPU_DATA* pStackBase,
                             OS_tCPU_DATA  stackSize);

#endif /* __PRETTY_ARCH_H_ */
