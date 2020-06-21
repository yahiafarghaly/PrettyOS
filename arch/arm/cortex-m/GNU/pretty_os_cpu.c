/*
 * pretty_os_cpu.c
 *
 *  Created on: Jun 8, 2020
 *      Author: yf
 */

#include "pretty_arch.h"

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
CPU_tWORD*
OS_CPU_TaskInit(void (*TASK_Handler)(void* params),
                             void *params,
                             CPU_tWORD* pStackBase,
                             CPU_tWORD  stackSize)
{
    CPU_tWORD* sp;
    /*
     * Move to the top of the task stack and round down it 8-bytes to
     * be Aligned.
     * */
    sp = (CPU_tWORD*)(((CPU_tWORD)pStackBase + stackSize) & 0xFFFFFFF8U);
    /* Stacking the registers as auto saved at exception entrance. */
    *(--sp) = (1U << 24);  /* xPSR ( Thumb State ) */
    *(--sp) = (CPU_tWORD)TASK_Handler;   /* PC (Task Entry Point) */
    *(--sp) = 0x0000000EU;                  /* LR  */
    *(--sp) = 0x0000000CU;                  /* R12 */
    *(--sp) = 0x00000003U;                  /* R3  */
    *(--sp) = 0x00000002U;                  /* R2  */
    *(--sp) = 0x00000001U;                  /* R1  */
    *(--sp) = (CPU_tWORD)params;         /* R0: Argument  */
    /* Additionally, fake registers R4-R11 */
    *(--sp) = 0x0000000BU;                  /* R11 */
    *(--sp) = 0x0000000AU;                  /* R10 */
    *(--sp) = 0x00000009U;                  /* R9 */
    *(--sp) = 0x00000008U;                  /* R8 */
    *(--sp) = 0x00000007U;                  /* R7 */
    *(--sp) = 0x00000006U;                  /* R6 */
    *(--sp) = 0x00000005U;                  /* R5 */
    *(--sp) = 0x00000004U;                  /* R4 */

#ifdef ARM_FP_ENABLED
    *--sp = 0x02000000u;                   /* FPSCR  */
     /* Initialize S0-S31 floating point registers */
    *--sp = 0x41F80000u;                        /* S31 */
    *--sp = 0x41F00000u;                        /* S30 */
    *--sp = 0x41E80000u;                        /* S29 */
    *--sp = 0x41E00000u;                        /* S28 */
    *--sp = 0x41D80000u;                        /* S27 */
    *--sp = 0x41D00000u;                        /* S26 */
    *--sp = 0x41C80000u;                        /* S25 */
    *--sp = 0x41C00000u;                        /* S24 */
    *--sp = 0x41B80000u;                        /* S23 */
    *--sp = 0x41B00000u;                        /* S22 */
    *--sp = 0x41A80000u;                        /* S21 */
    *--sp = 0x41A00000u;                        /* S20 */
    *--sp = 0x41980000u;                        /* S19 */
    *--sp = 0x41900000u;                        /* S18 */
    *--sp = 0x41880000u;                        /* S17 */
    *--sp = 0x41800000u;                        /* S16 */
    *--sp = 0x41700000u;                        /* S15 */
    *--sp = 0x41600000u;                        /* S14 */
    *--sp = 0x41500000u;                        /* S13 */
    *--sp = 0x41400000u;                        /* S12 */
    *--sp = 0x41300000u;                        /* S11 */
    *--sp = 0x41200000u;                        /* S10 */
    *--sp = 0x41100000u;                        /* S9  */
    *--sp = 0x41000000u;                        /* S8  */
    *--sp = 0x40E00000u;                        /* S7  */
    *--sp = 0x40C00000u;                        /* S6  */
    *--sp = 0x40A00000u;                        /* S5  */
    *--sp = 0x40800000u;                        /* S4  */
    *--sp = 0x40400000u;                        /* S3  */
    *--sp = 0x40000000u;                        /* S2  */
    *--sp = 0x3F800000u;                        /* S1  */
    *--sp = 0x00000000u;                        /* S0  */
#endif

    return (sp);
}

