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
 * Purpose  : POSIX port header file.
 *
 * Language:  C
 */

#ifndef __PRETTY_ARCH_H_
#define __PRETTY_ARCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
*******************************************************************************
*                             CPU Predefined Macros                           *
*******************************************************************************
*/


/*-------------------------------- CPU Word Sizes ----------------------------*/
#define  CPU_WORD_SIZE_08                          (8U)   /*  8-bit word size (1 Bytes).                                                    */
#define  CPU_WORD_SIZE_16                          (16U)  /* 16-bit word size (2 Bytes).                                                    */
#define  CPU_WORD_SIZE_32                          (32U)  /* 32-bit word size (4 Bytes).                                                    */
#define  CPU_WORD_SIZE_64                          (64U)  /* 64-bit word size (8 Bytes).                                                    */

/*----------------------- CPU Critical Sections Methods ----------------------*/
#define  CPU_CRITICAL_METHOD_NONE                  (0U)   /* Disable critical section protection.                                            */
#define  CPU_CRITICAL_METHOD_TRIVIAL               (1U)   /* Enable/Disable CPU interrupts without saving interrupt status register.         */
#define  CPU_CRITICAL_METHOD_STACK                 (2U)   /* Enable/Disable interrupts with saving Interrupt status on stack.                */
#define  CPU_CRITICAL_METHOD_LOCAL                 (3U)   /* Enable/Disable interrupts with saving Interrupt status on a local variable.     */

/*----------------------- CPU Stack Growth Direction ------------------------*/
#define  CPU_STACK_GROWTH_NONE                      (0U)
#define  CPU_STACK_GROWTH_HIGH_TO_LOW               (1U)
#define  CPU_STACK_GROWTH_LOW_TO_HIGH               (2U)

/*---------------------------- CPU Endianness -------------------------------*/
#define  CPU_ENDIAN_TYPE_BIG                        (1U)   /* Big-endian order, Store most significant byte in the lowest memory address.     */
#define  CPU_ENDIAN_TYPE_LITTLE                     (2U)   /* Little-endian order, Store most significant byte in the highest memory address. */


/*
*******************************************************************************
*                           Compiler Data Types                               *
*******************************************************************************
*/


typedef unsigned char       CPU_t08U;                    /* Unsigned  8-bit quantity    */
typedef signed   char       CPU_t08S;                    /* Signed    8-bit quantity    */
typedef unsigned short      CPU_t16U;                    /* Unsigned 16-bit quantity    */
typedef signed   short      CPU_t16S;                    /* Signed   16-bit quantity    */
typedef unsigned int        CPU_t32U;                    /* Unsigned 32-bit quantity    */
typedef signed   int        CPU_t32S;                    /* Signed   32-bit quantity    */
typedef unsigned long long  CPU_t64U;                    /* Unsigned 64-bit quantity    */
typedef signed   long long  CPU_t64S;                    /* Signed   64-bit quantity    */

typedef float               CPU_tFP32;                   /* 32-bit floating point       */
typedef double              CPU_tFP64;                   /* 64-bit floating point       */

typedef  volatile  CPU_t08U CPU_tReg08;                  /*  8-bit register             */
typedef  volatile  CPU_t16U CPU_tReg16;                  /* 16-bit register             */
typedef  volatile  CPU_t32U CPU_tReg32;                  /* 32-bit register             */
typedef  volatile  CPU_t64U CPU_tReg64;                  /* 64-bit register             */

typedef void*               CPU_tPtr;                    /* Pointer Type                */


/*
*******************************************************************************
*                             CPU Configurations                              *
*******************************************************************************
*/


/*----------------------- OS Tick Interrupt Priority -------------------------*/
/*
 * If the system doesn't require any real time interrupts, then set the tick
 * interrupt to the highest. It will not affect adversely other system operations.
 * Except that, then set it to the highest from any other non critical interrupts.
 * */
#define CPU_CONFIG_SYSTICK_PRIO                     (0U)

/*--------------------- Count Lead Zeros Implementation ----------------------*/

/*
 * (0U) If no assembly instruction is not supported. This will use pretty_CLZ.c implementation of CPU_CountLeadZeros() .
 * (1U) This will use the assembly implementation which is provided with the port.
 * */
#define CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT     (0U)

/*----------------------- CPU Data word sizes in bits ------------------------*/
#define CPU_CONFIG_DATA_SIZE_BITS                   (CPU_WORD_SIZE_32)              /*  Assume that a system with POSIX runs on a 32-bit processor.         */

/*--------------------- CPU Address word sizes in bits -----------------------*/
#define CPU_CONFIG_ADDR_SIZE_BITS                   (CPU_WORD_SIZE_32)              /*  Assume that a system with POSIX runs on a 32-bit processor.         */

/*----------------------- CPU Stack Growth Direction -------------------------*/
#define CPU_CONFIG_STACK_GROWTH                     (CPU_STACK_GROWTH_NONE)  		/*  Doesn't make a difference.					                        */

/*----------------------- CPU Data word memory order -------------------------*/
#define CPU_CONFIG_ENDIAN_TYPE                      (CPU_ENDIAN_TYPE_LITTLE)        /*  Doesn't make a difference.					                        */

/*----------------------- CPU Stack Alignment in Bytes -----------------------*/
#define CPU_CONFIG_STACK_ALIGN_BYTES                  (8U)                          /*  Doesn't make a difference.					                        */

/*----------------------- CPU Critical Section Method ------------------------*/
#define CPU_CONFIG_CRITICAL_METHOD                  (CPU_CRITICAL_METHOD_TRIVIAL)



/*
*******************************************************************************
*                        CPU Typedefs Configurations                          *
*******************************************************************************
*/

/*-------------------------------- CPU_tWORD ---------------------------------*/

#if (CPU_CONFIG_DATA_SIZE_BITS   == CPU_WORD_SIZE_08)
    typedef CPU_t08U        CPU_tWORD;
#elif (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_16)
    typedef CPU_t16U        CPU_tWORD;
#elif (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_32)
    typedef CPU_t32U        CPU_tWORD;
#elif (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_64)
    typedef CPU_t64U        CPU_tWORD;
#else
    #error "Undefined CPU_tWord type."
#endif

/*-------------------------------- CPU_tADDR ---------------------------------*/

#if (CPU_CONFIG_ADDR_SIZE_BITS  == CPU_WORD_SIZE_08)
    typedef CPU_t08U        CPU_tADDR;
#elif (CPU_CONFIG_ADDR_SIZE_BITS == CPU_WORD_SIZE_16)
    typedef CPU_t16U        CPU_tADDR;
#elif (CPU_CONFIG_ADDR_SIZE_BITS == CPU_WORD_SIZE_32)
    typedef CPU_t32U        CPU_tADDR;
#elif (CPU_CONFIG_ADDR_SIZE_BITS == CPU_WORD_SIZE_64)
    typedef CPU_t64U        CPU_tADDR;
#else
    #error "Undefined CPU_tADDR type."
#endif

/*-------------------------------- CPU_t* ------------------------------------*/
typedef CPU_tWORD   CPU_tALIGN; 	/* CPU Data word alignment size.          */
typedef CPU_t32U    CPU_tSR;    	/* Define size of CPU status register.    */
typedef CPU_t32U	CPU_tSTK;		/* Define CPU stack data type.			  */
typedef CPU_t32U	CPU_tSTK_SIZE; 	/* Define CPU stack size data type.		  */

/*
*******************************************************************************
*                             Critical Section Management                     *
*******************************************************************************
*/

/*
 * Configure 'CPU_CONFIG_CRITICAL_METHOD' to one of the following options:
 *          1) CPU_CRITICAL_METHOD_TRIVIAL
 *          2) CPU_CRITICAL_METHOD_STACK
 *          3) CPU_CRITICAL_METHOD_LOCAL
 *
 *          CPU_CRITICAL_METHOD_TRIVIAL
 *          --------------------------
 *          Using this method means you are not support multiple levels of interrupts.
 *
 *
 *          CPU_CRITICAL_METHOD_STACK
 *          -------------------------
 *          Using this method means you are support multiple levels of interrupts by
 *          saving the interrupt status on the memory stack.
 *
 *              > Enter Critical section:
 *                  1) Push/save    interrupt status onto a local stack.
 *                  2) Disable interrupts.
 *              > Exit  Critical section:
 *                  1) Pop/restore interrupt status from a local stack.
 *
 *
 *          CPU_CRITICAL_METHOD_LOCAL
 *          -------------------------
 *          Using this method means you are support multiple levels of interrupts by
 *          saving the interrupt status into a local variable.
 *
 *              > Enter Critical section:
 *                  1) Save    interrupt status into a local variable.
 *                  2) Disable interrupts.
 *              > Exit  Critical section:
 *                  1) Restore interrupt status from a local variable.
 *
 *                  In this method, a local variable named 'cpu_sr' is used to hold the interrupt status inside PrettyOS code.
 *                  This requires that 'cpu_sr' to be declared before invoking OS_CRTICAL_BEGIN() or OS_CRTICAL_END()
 *
 *                  Using CPU_SR_ALLOC() macro, allows to declare 'cpu_sr' with the appropriate CPU data size
 *                  which is enough to store the CPU status word.
 *
 *                      Usage Example:
 *                      void Foo (void)
 *                      {
 *                          CPU_t32U x;
 *                          CPU_t32U y;
 *                          CPU_SR_ALLOC();      Preferred to be declared after all other local variables.
 *                      }
 */

#if(CPU_CONFIG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_TRIVIAL)

    #define CPU_SR_ALLOC()          /* CPU_SR_ALLOC(); To give a completeness for the compiler.                                     */
    #define OS_CRTICAL_BEGIN()      do { CPU_InterruptDisable(); } while (0)
    #define OS_CRTICAL_END()        do { CPU_InterruptEnable (); } while (0)

#endif


/*
*******************************************************************************
*                    CPU Low Level APIs Hooks Configurations                  *
*******************************************************************************
*/

#define 	OS_CONFIG_ENABLE				(1U)		/* TRUE  value for Enabling  a macro.		*/
#define 	OS_CONFIG_DISABLE				(0U)		/* FALSE value for Disabling a macro. 		*/


/*=======  Enable/Disable CPU Hook for doing CPU initialization before OS_Init()   ======*/

#define OS_CONFIG_CPU_INIT					(OS_CONFIG_ENABLE)

/*=======  			Enable/Disable CPU Hook idle routine  ====================*/

#define OS_CONFIG_CPU_IDLE					(OS_CONFIG_ENABLE)

/*=======  		Enable/Disable CPU Hook when Context Switch occurs. ==========*/

#define OS_CONFIG_CPU_CONTEXT_SWITCH		(OS_CONFIG_ENABLE)

/*=======  		Enable/Disable CPU Hook when a task is created ===============*/

#define OS_CONFIG_CPU_TASK_CREATED			(OS_CONFIG_ENABLE)

/*=======  		Enable/Disable CPU Hook when a task is deleted. ==============*/

#define OS_CONFIG_CPU_TASK_DELETED			(OS_CONFIG_ENABLE)

/*=======  		Enable/Disable CPU Hook when a CPU timer tick occurs  ========*/

#define OS_CONFIG_CPU_TIME_TICK				(OS_CONFIG_ENABLE)

/*=======    Enable/Disable CPU Hook when a stack overflow is detected. ======*/

#define OS_CONFIG_CPU_STACK_OVERFLOW        (OS_CONFIG_DISABLE)

/*
*******************************************************************************
*                         Others' CPU Configurations                          *
*******************************************************************************
*/

/*=========  Enable/Disable OS Software Stack Overflow Detection. =========*/

#define OS_CONFIG_CPU_SOFT_STK_OVERFLOW_DETECTION   (OS_CONFIG_DISABLE)

/*
*******************************************************************************
*                      CPU Specific Functions Prototypes                      *
*******************************************************************************
*/

void CPU_InterruptDisable (void);

void CPU_InterruptEnable (void);


/*
 * Function:  CPU_CountLeadZeros
 * --------------------
 * Counts the number of contiguous, most-significant, leading zero bits before the
 * first binary one bit in a data value.
 *
 * Arguments    :   val   Data value to count the leading zero bits.
 *
 * Returns      :   Number of contiguous, most-significant, leading zero bits in 'val'.
 */
CPU_tWORD CPU_CountLeadZeros(CPU_tWORD val);

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
 * Function:  OS_CPU_TaskStackInit
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
CPU_tSTK* OS_CPU_TaskStackInit(void (*TASK_Handler)(void* params),
                             void *params,
							 CPU_tSTK* pStackBase,
							 CPU_tSTK_SIZE  stackSize);

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
