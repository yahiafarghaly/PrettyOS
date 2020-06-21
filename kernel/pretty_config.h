/*
 * pretty_config.h
 *
 *  Created on: Jun 8, 2020
 *      Author: yf
 */

#ifndef __PRETTY_CONFIG_H_
#define __PRETTY_CONFIG_H_

/*
*******************************************************************************
*                       OS Miscellaneous Configurations                       *
*******************************************************************************
*/

#define OS_MAX_PRIO_ENTRIES  (3U)           /* Number of priority entries (levels), minimum value = 1 */

/*
*******************************************************************************
*                             Data Types Sizes                                *
*******************************************************************************
*/

typedef CPU_tWORD       OS_tRet;            /* Fit to the easiest type of memory for CPU   */

typedef CPU_t32U        OS_TICK;            /* Clock tick counter                          */

typedef CPU_t08U        OS_PRIO;            /* Max task priority can hold                  */

typedef CPU_t08U        OS_STATUS;          /* Task status                                 */


#endif /* __PRETTY_CONFIG_H_ */
