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
 * Purpose  : Implementation of BSP APIs for EK-LM4F120XL development board
 *            based on ARM Cortex-M4 CPU.
 *
 * Language:  C
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/

#include <bsp.h>                    /* BSP Exposed APIs.                                                      */
#include "TM4C123GH6PM.h"           /* Macros for registers access on this BSP. TM4C123 is similar to LM4F120 */

/*
*******************************************************************************
*                               BSP Macros                                    *
*******************************************************************************
*/

/* Running System Clock in Hertz. */
#define SYS_CLOCK_HZ    16000000U

/* LM4F120 On-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

static inline void
BSP_UART_init(void)
{
    /* Configure UART1 For Transmitting */

    /* Enable UART1/GPIOB Clock */
    SYSCTL->RCGCGPIO    |= (1U << 1);
    /* Enable and provide a clock to UART1 */
    SYSCTL->RCGCUART    |= (1U << 1);
    /* Enable UART1 to be used instead of GPIO on PB0(U1Rx) and PB1(U1Tx) Ports. */
    GPIOB->AFSEL    |= 0x03;
    /* Enable Digital Input/output for UART1 pins. */
    GPIOB->DEN      |= 0x03;
    /* Disable Analog functions on UART1 pins if necessary. */
    GPIOB->AMSEL    &= ~(0x03);
    /* 2mA Output Drive For PB0 and PB1 */
    GPIOB->DR2R     |= (0x03);
    /* Select Port Control encoding For UART */
    GPIOB->PCTL     = (GPIOB->PCTL & 0xFFFFFF00)  + (0x00000011);

    /* Configure Baudrate of 115200 with 16MHz as a clock source (POSIC) for UART.
     *
     * This leds that BRD 16000000/(16 * 115200) = 8.6805
     * DIVINT = 8
     * DIVFRAC = integer(0.6805 * 64 + 0.5) = 44
     * */
    UART1->CTL &= ~(1U << 0); /* Disable UART1 by clearing UARTEN .      */
    UART1->CTL &= ~(1U << 5); /* Disable High Speed Mode (System clock divided by 16 ). */
    UART1->IBRD = 8U;         /* Write Integer portion of the baudrate.  */
    UART1->FBRD = 44U;        /* Write fraction portion of the baudrate. */
    UART1->LCRH = 0x00000060; /* Set for 8 bit data transfer, character mode(FIFO Disabled), no parity check. */
    UART1->FR   = 0;          /* Clear the Flag register */
    UART1->CC   = 0x5;        /* Configure UART clock to be from POSIC which is 16 MHz. */
    UART1->CTL |= (1U << 0) | (1U << 8) | (1U << 9);   /* Enable UART1 Tx and Rx and module itself.  */
}

void
BSP_HardwareSetup(void) {

    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */

    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    BSP_UART_init();
}

void
BSP_DelayMilliseconds (unsigned long ms) {
    volatile unsigned long ticks;

    /*TODO: Still needs extra work  */
    if((1000U / BSP_TICKS_PER_SEC_CONFIG) > ms )
    {
        ticks = 1U;
    }
    else
    {
        ticks = (ms * BSP_TICKS_PER_SEC_CONFIG) / 1000U;
    }


    while(ticks)
    {
        BSP_CPU_NOP();
        ticks -= 1U;
    }

    return;
    /*
     * F_CPU = 16*1000*1000 Hz
     * instruction_time (one cycle) = 1/F_CPU seconds
     *                  = (1000/F_CPU) milliseconds.
     * to achieve a delay of milliseconds = (1000*ms/F_CPU)
     * */
    /*
     * 1000 ms -> bsp_ticks (ex:100)
     * ms ->  ticks
     * ticks = (ms * bsp_ticks)/1000U
     *
     * resolution = 1000/bsp_ticks ms
     * */
}

void
BSP_UART_SendByte(const unsigned char cData)
{
    /* Wait until the FIFO is empty. */
    while(UART1->FR & 0x00000020);
    /* This should initiate a write transmission. */
    UART1->DR = cData;
}

void
BSP_UART_ClearVirtualTerminal (void)
{
    /* The command sequences for minicom software to clear the screen is Esc[2J
     * which can be interpreted as
     *  Esc     the ASCII Escape character, value 0x1B.
     *  [       the ASCII left square brace character, value 0x5B.
     *  2       the ASCII character for numeral 2, value 0x32.
     *  J       the ASCII character for the letter J, value 0x4A.
     * */
    BSP_UART_SendByte(0x1B);
    BSP_UART_SendByte(0x5B);
    BSP_UART_SendByte(0x32);
    BSP_UART_SendByte(0x4A);
}

void
BSP_LED_RedOn(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
}

void
BSP_LED_RedOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
}

void
BSP_LED_BlueOn(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
}

void
BSP_LED_BlueOff(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
}

void
BSP_LED_GreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
}

void
BSP_LED_GreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
}

unsigned long
BSP_CPU_FrequencyGet(void)
{
    return SYS_CLOCK_HZ;
}

void
BSP_onFailure(char const *module, int location)
{
    //NVIC_SystemReset();
    extern void UARTprintf(const char *pcString, ...);
    UARTprintf("\n%s %d\n",module,location);
    for(;;);
}

void
BSP_CPU_WFI (void)
{
    __WFI(); /* stop the CPU and Wait for Interrupt */
}

void BSP_CPU_NOP (void)
{
    __NOP();
}
