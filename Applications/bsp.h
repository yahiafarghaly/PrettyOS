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
 * Purpose  : Generic header file of the exposed function prototypes independent
 *            on the target board which can be used by your embedded applications.
 *
 * Language:  C
 */

#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/

/*
*******************************************************************************
*                               BSP Macros                                    *
*******************************************************************************
*/

#define BSP_TICKS_PER_SEC_CONFIG 100U   /* Configure number of BSP CPU ticks per second. */

/*
*******************************************************************************
*                           BSP Functions Prototypes                          *
*******************************************************************************
*/

/*===================== BSP board Miscellaneous function ===================*/

/*
 * Function:  BSP_HardwareSetup
 * ----------------------------
 * Do everything necessary to setup the hardware used by other BSP APIs.
 */
extern void BSP_HardwareSetup (void);

/*
 * Function:  BSP_DelayMilliseconds
 * --------------------------------
 * Delay the next instruction to be executed for a number of Milliseconds.
 * This call should be blocking, Act like a busy-wait loop with approximation of delay time.
 *
 * Arguments    : ms     is the blocking time in milliseconds.
 *
 * Returns      : None.
 */
extern void BSP_DelayMilliseconds (unsigned long ms);

/*
 * Function:  BSP_Write_to_Console
 * --------------------------------
 * Write string data to an output stream, similar to C function "int printf(const char *format, ...)"
 */
extern int BSP_Write_to_Console(const char *format, ...);

/*========================== BSP UART functions =============================*/

/*
 * Function:  BSP_UARTSendByte
 * ---------------------------
 * Send an unsigned byte data via UART port.
 *
 * Arguments    : cData     is an unsigned char data type to send via UART port.
 *
 * Returns      : None.
 */
extern void BSP_UART_SendByte (const unsigned char cData);

/*
 * Function:  BSP_UARTReceiveByte
 * ------------------------------
 * Receive an unsigned byte data via UART port.
 *
 * Arguments    : cData     is a pointer to an unsigned char variable to put the received byte via UART port.
 *
 * Returns      : None.
 */
extern void BSP_UART_ReceiveByte (unsigned char* cData);

/*
 * Function:  BSP_UART_ClearVirtualTerminal
 * ------------------------------
 * This function send byte command sequences to clear the contents on the virtual COM connected via UART port.
 *
 * Arguments    : None.
 *
 * Returns      : None.
 */
extern void BSP_UART_ClearVirtualTerminal (void);

/*========================= BSP LEDs Turn [On/Off] ==========================*/

/*
 * Function:  BSP_LED_<Color>[on|off]
 * ---------------------------------
 * Group of functions to turn on/off LEDs on the target board.
 *
 * Arguments    : None.
 *
 * Returns      : None.
 */
extern void BSP_LED_RedOn   (void);
extern void BSP_LED_RedOff  (void);

extern void BSP_LED_BlueOn  (void);
extern void BSP_LED_BlueOff (void);

extern void BSP_LED_GreenOn (void);
extern void BSP_LED_GreenOff(void);


/*=========================== BSP CPU functions ============================*/

/*
 * Function:  BSP_CPU_FrequencyGet
 * ------------------------------
 * Gets CPU Clock/Frequency in Hz.
 *
 * Arguments    : None.
 *
 * Returns      : Returns CPU/Frequency Clock in Hz.
 */
extern unsigned long BSP_CPU_FrequencyGet (void);

/*
 * Function:  BSP_CPU_WFI
 * ----------------------
 * Wait For Interrupt is a CPU hint instruction that suspends execution until an Interrupt is triggered.
 */
extern inline void BSP_CPU_WFI (void);

/*
 * Function:  BSP_CPU_NOP
 * ----------------------
 * No Operation, CPU does nothing.
 */
extern inline void BSP_CPU_NOP (void);

/*
 * Function:  BSP_CPU_ISB
 * ----------------------
 * Instruction Synchronization Barrier flushes the pipeline in the processor, so that all instructions
 * following the ISB are fetched from cache or memory, after the instruction has been completed.
 */
extern inline void BSP_CPU_ISB (void);

/*
 * Function:  BSP_CPU_DSB
 * ----------------------
 * Data Synchronization Barrier, It executes when all explicit memory accesses before this instruction complete.
 */
extern inline void BSP_CPU_DSB (void);


/*========================== BSP Failure functions =======================*/

/*
 * Function:  BSP_onFailure
 * ------------------------
 * Handle system failure in a proper way.
 *
 * This function should be called with `module` name and file line `location`
 * when you suspect a system failure in a certain point.
 *
 * Note: Preferred to call this function with no stacking to debug
 *      the memory without any more corruption on the stack.
 *      ex: __attribute__((naked)) in GNU Compiler.
 *          __stackless            in IAR Compiler.
 */
void BSP_onFailure (char const *module, int location);


#ifdef __cplusplus
}
#endif

#endif // __BSP_H__
