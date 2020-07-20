/*
 * File:         startup_lm4f_gnu.c
 * Author:       Yahia Farghaly <yahiafarghaly@gmail.com>.
 * Description:  startup file for a Cortex-M4 device.
 */

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

/*
*******************************************************************************
*                   Symbols defined in the linker script                      *
*******************************************************************************
*/

extern unsigned long    __data_start__;
extern unsigned long    __data_end__;
extern unsigned long    __data_load;
extern unsigned long    __bss_start__;
extern unsigned long    __bss_end__;
extern unsigned long    __stack_start__;
extern unsigned long    __stack_end__;

/*
*******************************************************************************
*                   Symbols Defined in other source files                     *
*******************************************************************************
*/

extern int main(void);

/*
*******************************************************************************
*                               Functions Prototypes                          *
*******************************************************************************
*/

extern void Reset_Handler(void);

__attribute__((naked))  void Default_Handler(void);
__attribute__((naked))  void BSP_onFailure(char const *module, int loc);    /* Made ((naked)) to avoid stack changes when it's called on system failure. */

/*
*******************************************************************************
*                  LMF4F Exceptions Handlers Prototypes                       *
*******************************************************************************
*/

                    /* Cortex-M Processor fault exceptions. */
void NMI_Handler           (void) __attribute__ ((weak));
void HardFault_Handler     (void) __attribute__ ((weak));
void MemManage_Handler     (void) __attribute__ ((weak));
void BusFault_Handler      (void) __attribute__ ((weak));
void UsageFault_Handler    (void) __attribute__ ((weak));

                    /* Cortex-M Processor non-fault exceptions. */
void SVC_Handler           (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));

                    /*       No weak, Must be defined.          */
void OS_CPU_SystemTimerHandler  (void); /* Defined in PrettyOS/arch/arm/cortex-m/GNU/pretty_os_cpu.c */
void OS_CPU_PendSVHandler       (void); /* Defined in the PrettyOS/arch/arm/cortex-m/GNU/pretty_os_cpu_a.asm */

                    /* LM4F External interrupts.   */
void GPIOPortA_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortB_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortC_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortD_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortE_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void UART0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SSI0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMFault_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen2_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void QEI0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq0_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq1_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq2_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ADCSeq3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Watchdog_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer0A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer0B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer1A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer1B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer2A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer2B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp0_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp1_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void Comp2_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SysCtrl_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void FlashCtrl_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortF_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortG_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortH_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SSI1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer3A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer3B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void QEI1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void CAN2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Hibernate_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void USB0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PWMGen3_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void uDMAST_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void uDMAError_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC1Seq0_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC1Seq1_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC1Seq2_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC1Seq3_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortJ_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortK_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortL_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void SSI2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void SSI3_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART3_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UART4_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UART5_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UART6_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UART7_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C3_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer4A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer4B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer5A_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Timer5B_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer0A_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer0B_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer1A_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer1B_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer2A_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer2B_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer3A_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer3B_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer4A_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer4B_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer5A_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void WideTimer5B_IRQHandler(void) __attribute__ ((weak, alias("Default_Handler")));
void FPU_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C4_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C5_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortM_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortN_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void QEI2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortP0_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortP1_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortP2_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortP3_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortP4_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortP5_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortP6_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortP7_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortQ0_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortQ1_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortQ2_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortQ3_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortQ4_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortQ5_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortQ6_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortQ7_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortR_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOPortS_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM1Gen0_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM1Gen1_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM1Gen2_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM1Gen3_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM1Fault_IRQHandler  (void) __attribute__ ((weak, alias("Default_Handler")));


/*
*******************************************************************************
*                        LMF4F NVIC Vector Table                              *
*******************************************************************************
*/
__attribute__ ((section(".NVIC_TABLE")))
int const g_pfnVectors[] = {
    (int)&__stack_start__,        /* Top of Stack                    */
    (int)&Reset_Handler,          /* Reset Handler                   */
    (int)&NMI_Handler,            /* NMI Handler                     */
    (int)&HardFault_Handler,      /* Hard Fault Handler              */
    (int)&MemManage_Handler,      /* The MPU fault handler           */
    (int)&BusFault_Handler,       /* The bus fault handler           */
    (int)&UsageFault_Handler,     /* The usage fault handler         */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    (int)&SVC_Handler,            /* SVCall handler                  */
    (int)&DebugMon_Handler,       /* Debug monitor handler           */
    0,                            /* Reserved                        */
    (int)&OS_CPU_PendSVHandler,   /* The PendSV handler              */
    (int)&OS_CPU_SystemTimerHandler,        /* The SysTick handler             */

    /*IRQ handlers... */
    (int)&GPIOPortA_IRQHandler,   /* GPIO Port A                     */
    (int)&GPIOPortB_IRQHandler,   /* GPIO Port B                     */
    (int)&GPIOPortC_IRQHandler,   /* GPIO Port C                     */
    (int)&GPIOPortD_IRQHandler,   /* GPIO Port D                     */
    (int)&GPIOPortE_IRQHandler,   /* GPIO Port E                     */
    (int)&UART0_IRQHandler,       /* UART0 Rx and Tx                 */
    (int)&UART1_IRQHandler,       /* UART1 Rx and Tx                 */
    (int)&SSI0_IRQHandler,        /* SSI0 Rx and Tx                  */
    (int)&I2C0_IRQHandler,        /* I2C0 Master and Slave           */
    (int)&PWMFault_IRQHandler,    /* PWM Fault                       */
    (int)&PWMGen0_IRQHandler,     /* PWM Generator 0                 */
    (int)&PWMGen1_IRQHandler,     /* PWM Generator 1                 */
    (int)&PWMGen2_IRQHandler,     /* PWM Generator 2                 */
    (int)&QEI0_IRQHandler,        /* Quadrature Encoder 0            */
    (int)&ADCSeq0_IRQHandler,     /* ADC Sequence 0                  */
    (int)&ADCSeq1_IRQHandler,     /* ADC Sequence 1                  */
    (int)&ADCSeq2_IRQHandler,     /* ADC Sequence 2                  */
    (int)&ADCSeq3_IRQHandler,     /* ADC Sequence 3                  */
    (int)&Watchdog_IRQHandler,    /* Watchdog timer                  */
    (int)&Timer0A_IRQHandler,     /* Timer 0 subtimer A              */
    (int)&Timer0B_IRQHandler,     /* Timer 0 subtimer B              */
    (int)&Timer1A_IRQHandler,     /* Timer 1 subtimer A              */
    (int)&Timer1B_IRQHandler,     /* Timer 1 subtimer B              */
    (int)&Timer2A_IRQHandler,     /* Timer 2 subtimer A              */
    (int)&Timer2B_IRQHandler,     /* Timer 2 subtimer B              */
    (int)&Comp0_IRQHandler,       /* Analog Comparator 0             */
    (int)&Comp1_IRQHandler,       /* Analog Comparator 1             */
    (int)&Comp2_IRQHandler,       /* Analog Comparator 2             */
    (int)&SysCtrl_IRQHandler,     /* System Control (PLL, OSC, BO)   */
    (int)&FlashCtrl_IRQHandler,   /* FLASH Control                   */
    (int)&GPIOPortF_IRQHandler,   /* GPIO Port F                     */
    (int)&GPIOPortG_IRQHandler,   /* GPIO Port G                     */
    (int)&GPIOPortH_IRQHandler,   /* GPIO Port H                     */
    (int)&UART2_IRQHandler,       /* UART2 Rx and Tx                 */
    (int)&SSI1_IRQHandler,        /* SSI1 Rx and Tx                  */
    (int)&Timer3A_IRQHandler,     /* Timer 3 subtimer A              */
    (int)&Timer3B_IRQHandler,     /* Timer 3 subtimer B              */
    (int)&I2C1_IRQHandler,        /* I2C1 Master and Slave           */
    (int)&QEI1_IRQHandler,        /* Quadrature Encoder 1            */
    (int)&CAN0_IRQHandler,        /* CAN0                            */
    (int)&CAN1_IRQHandler,        /* CAN1                            */
    (int)&CAN2_IRQHandler,        /* CAN2                            */
    0,                            /* Reserved                        */
    (int)&Hibernate_IRQHandler,   /* Hibernate                       */
    (int)&USB0_IRQHandler,        /* USB0                            */
    (int)&PWMGen3_IRQHandler,     /* PWM Generator 3                 */
    (int)&uDMAST_IRQHandler,      /* uDMA Software Transfer          */
    (int)&uDMAError_IRQHandler,   /* uDMA Error                      */
    (int)&ADC1Seq0_IRQHandler,    /* ADC1 Sequence 0                 */
    (int)&ADC1Seq1_IRQHandler,    /* ADC1 Sequence 1                 */
    (int)&ADC1Seq2_IRQHandler,    /* ADC1 Sequence 2                 */
    (int)&ADC1Seq3_IRQHandler,    /* ADC1 Sequence 3                 */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    (int)&GPIOPortJ_IRQHandler,   /* GPIO Port J                     */
    (int)&GPIOPortK_IRQHandler,   /* GPIO Port K                     */
    (int)&GPIOPortL_IRQHandler,   /* GPIO Port L                     */
    (int)&SSI2_IRQHandler,        /* SSI2 Rx and Tx                  */
    (int)&SSI3_IRQHandler,        /* SSI3 Rx and Tx                  */
    (int)&UART3_IRQHandler,       /* UART3 Rx and Tx                 */
    (int)&UART4_IRQHandler,       /* UART4 Rx and Tx                 */
    (int)&UART5_IRQHandler,       /* UART5 Rx and Tx                 */
    (int)&UART6_IRQHandler,       /* UART6 Rx and Tx                 */
    (int)&UART7_IRQHandler,       /* UART7 Rx and Tx                 */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    (int)&I2C2_IRQHandler,        /* I2C2 Master and Slave           */
    (int)&I2C3_IRQHandler,        /* I2C3 Master and Slave           */
    (int)&Timer4A_IRQHandler,     /* Timer 4 subtimer A              */
    (int)&Timer4B_IRQHandler,     /* Timer 4 subtimer B              */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    (int)&Timer5A_IRQHandler,     /* Timer 5 subtimer A              */
    (int)&Timer5B_IRQHandler,     /* Timer 5 subtimer B              */
    (int)&WideTimer0A_IRQHandler, /* Wide Timer 0 subtimer A         */
    (int)&WideTimer0B_IRQHandler, /* Wide Timer 0 subtimer B         */
    (int)&WideTimer1A_IRQHandler, /* Wide Timer 1 subtimer A         */
    (int)&WideTimer1B_IRQHandler, /* Wide Timer 1 subtimer B         */
    (int)&WideTimer2A_IRQHandler, /* Wide Timer 2 subtimer A         */
    (int)&WideTimer2B_IRQHandler, /* Wide Timer 2 subtimer B         */
    (int)&WideTimer3A_IRQHandler, /* Wide Timer 3 subtimer A         */
    (int)&WideTimer3B_IRQHandler, /* Wide Timer 3 subtimer B         */
    (int)&WideTimer4A_IRQHandler, /* Wide Timer 4 subtimer A         */
    (int)&WideTimer4B_IRQHandler, /* Wide Timer 4 subtimer B         */
    (int)&WideTimer5A_IRQHandler, /* Wide Timer 5 subtimer A         */
    (int)&WideTimer5B_IRQHandler, /* Wide Timer 5 subtimer B         */
    (int)&FPU_IRQHandler,         /* FPU                             */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    (int)&I2C4_IRQHandler,        /* I2C4 Master and Slave           */
    (int)&I2C5_IRQHandler,        /* I2C5 Master and Slave           */
    (int)&GPIOPortM_IRQHandler,   /* GPIO Port M                     */
    (int)&GPIOPortN_IRQHandler,   /* GPIO Port N                     */
    (int)&QEI2_IRQHandler,        /* Quadrature Encoder 2            */
    0,                            /* Reserved                        */
    0,                            /* Reserved                        */
    (int)&GPIOPortP0_IRQHandler,  /* GPIO Port P (Summary or P0)     */
    (int)&GPIOPortP1_IRQHandler,  /* GPIO Port P1                    */
    (int)&GPIOPortP2_IRQHandler,  /* GPIO Port P2                    */
    (int)&GPIOPortP3_IRQHandler,  /* GPIO Port P3                    */
    (int)&GPIOPortP4_IRQHandler,  /* GPIO Port P4                    */
    (int)&GPIOPortP5_IRQHandler,  /* GPIO Port P5                    */
    (int)&GPIOPortP6_IRQHandler,  /* GPIO Port P6                    */
    (int)&GPIOPortP7_IRQHandler,  /* GPIO Port P7                    */
    (int)&GPIOPortQ0_IRQHandler,  /* GPIO Port Q (Summary or Q0)     */
    (int)&GPIOPortQ1_IRQHandler,  /* GPIO Port Q1                    */
    (int)&GPIOPortQ2_IRQHandler,  /* GPIO Port Q2                    */
    (int)&GPIOPortQ3_IRQHandler,  /* GPIO Port Q3                    */
    (int)&GPIOPortQ4_IRQHandler,  /* GPIO Port Q4                    */
    (int)&GPIOPortQ5_IRQHandler,  /* GPIO Port Q5                    */
    (int)&GPIOPortQ6_IRQHandler,  /* GPIO Port Q6                    */
    (int)&GPIOPortQ7_IRQHandler,  /* GPIO Port Q7                    */
    (int)&GPIOPortR_IRQHandler,   /* GPIO Port R                     */
    (int)&GPIOPortS_IRQHandler,   /* GPIO Port S                     */
    (int)&PWM1Gen0_IRQHandler,    /* PWM 1 Generator 0               */
    (int)&PWM1Gen1_IRQHandler,    /* PWM 1 Generator 1               */
    (int)&PWM1Gen2_IRQHandler,    /* PWM 1 Generator 2               */
    (int)&PWM1Gen3_IRQHandler,    /* PWM 1 Generator 3               */
    (int)&PWM1Fault_IRQHandler,   /* PWM 1 Fault                     */
};

/*
*******************************************************************************
*                  LMF4F Exception Handlers Definition                        *
*******************************************************************************
*/

/* Entry point of the embedded application. */
void Reset_Handler(void) {
    unsigned long const *src;
    unsigned long *dst;

    /* Initializing The whole RAM */
    dst = &__data_start__;
    while(dst < &__stack_start__)
    {
        *dst = 0xFEEDC0DE;  /* A magic code for ram bytes. */
         dst++;
    }

    /* Copy the data segment initializers from flash to RAM... */
    src = &__data_load;
    for (dst = &__data_start__; dst < &__data_end__; ++dst, ++src) {
        *dst = *src;
    }

    /* zero fill the .bss segment in RAM... */
    for (dst = &__bss_start__; dst < &__bss_end__; ++dst) {
        *dst = 0;
    }


    main(); /* application's entry point; should never return! */

    /* the previous code should not return, but fail just in case... */
    BSP_onFailure("Reset_Handler", __LINE__);
}


__attribute__((naked)) void NMI_Handler(void);
void NMI_Handler(void) {
    __asm volatile (
        "    ldr r0,=str_nmi\n\t"
        "    mov r1,#1\n\t"
        "    b BSP_onFailure\n\t"
        "str_nmi: .asciz \"NMI\"\n\t"
        "  .align 2\n\t"
    );
}


__attribute__((naked)) void MemManage_Handler(void);
void MemManage_Handler(void) {
    __asm volatile (
        "    ldr r0,=str_mem\n\t"
        "    mov r1,#1\n\t"
        "    b BSP_onFailure\n\t"
        "str_mem: .asciz \"MemManage\"\n\t"
        "  .align 2\n\t"
    );
}


__attribute__((naked)) void HardFault_Handler(void);
void HardFault_Handler(void) {
    __asm volatile (
        "    ldr r0,=str_hrd\n\t"
        "    mov r1,#1\n\t"
        "    b BSP_onFailure\n\t"
        "str_hrd: .asciz \"HardFault\"\n\t"
        "  .align 2\n\t"
    );
}


__attribute__((naked)) void BusFault_Handler(void);
void BusFault_Handler(void) {
    __asm volatile (
        "    ldr r0,=str_bus\n\t"
        "    mov r1,#1\n\t"
        "    b BSP_onFailure\n\t"
        "str_bus: .asciz \"BusFault\"\n\t"
        "  .align 2\n\t"
    );
}


__attribute__((naked)) void UsageFault_Handler(void);
void UsageFault_Handler(void) {
    __asm volatile (
        "    ldr r0,=str_usage\n\t"
        "    mov r1,#1\n\t"
        "    b BSP_onFailure\n\t"
        "str_usage: .asciz \"UsageFault\"\n\t"
        "  .align 2\n\t"
    );
}

__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void) {
    __asm volatile (
        "    ldr r0,=str_dflt\n\t"
        "    mov r1,#1\n\t"
        "    b BSP_onFailure\n\t"
        "str_dflt: .asciz \"Default\"\n\t"
        "  .align 2\n\t"
    );
}
