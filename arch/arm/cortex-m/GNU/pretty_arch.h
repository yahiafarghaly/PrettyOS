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
typedef unsigned int   OS_tCPU_DATA;              /* In ARM Case 32-bit wide */

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

void OS_CPU_ContexSwitch(void);
void OS_CPU_InterruptContexSwitch(void);
void OS_CPU_FirstStart(void);
OS_tCPU_DATA* OS_CPU_TaskInit(void (*TASK_Handler)(void* params),
                             void *params,
                             OS_tCPU_DATA* pStackBase,
                             OS_tCPU_DATA  stackSize);

#endif /* __PRETTY_ARCH_H_ */
