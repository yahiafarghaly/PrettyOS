#ifndef __BSP_H__
#define __BSP_H__

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

#define BSP_TICKS_PER_SEC 100U          /* A Timer interrupt should be set to fire every (1/BSP_TICKS_PER_SEC) second. */

/*
*******************************************************************************
*                           BSP Functions Prototypes                          *
*******************************************************************************
*/

/*
 * Function:  BSP_HardwareSetup
 * ----------------------------
 * Do everything necessary to setup the hardware used by other BSP APIs.
 */
void BSP_HardwareSetup(void);

/*
 * Function:  BSP_led<Color>[on|off]
 * ---------------------------------
 * Group of functions to turn on/off LEDs on the target board.
 */
void BSP_ledRedOn(void);
void BSP_ledRedOff(void);

void BSP_ledBlueOn(void);
void BSP_ledBlueOff(void);

void BSP_ledGreenOn(void);
void BSP_ledGreenOff(void);

/*
 * Function:  BSP_UARTSend
 * ----------------------------
 * Send a signed byte data via UART port.
 */
void BSP_UARTSend(const char cData);

/*
 * Function:  BSP_SystemClockGet
 * ----------------------------
 * Return System Clock in Hz.
 */
unsigned long BSP_SystemClockGet(void);

/*
 * Function:  BSP_onFailure
 * ----------------------------
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
void BSP_onFailure(char const *module, int location);

/*
 * Function:  BSP_WaitForInterrupt
 * ----------------------------
 * Wait For Interrupt is a hint instruction that suspends execution until an event occurs.
 */
void BSP_WaitForInterrupt(void);

#endif // __BSP_H__
