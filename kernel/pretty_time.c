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
 * Author   : 	Yahia Farghaly Ashour
 *
 * Purpose  :	Implementation of various of time delay functions in prettyOS.
 *
 * Language	:  	C
 * 
 * Set 1 tab = 4 spaces for better comments readability.
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"
#include "pretty_shared.h"

#if(OS_CONFIG_EDF_EN == OS_CONFIG_DISABLE)
/*
 * Function:  OS_DelayTicks
 * --------------------
 * Block the current task execution for number of system ticks.
 *
 * Arguments    :   ticks   is the number of ticks for the task to be blocked.
 *
 * Returns      :   None.
 *
 * Note(s)      :   1) This function is called only from task level code.
 */
void
OS_DelayTicks (OS_TICK ticks)
{
    CPU_SR_ALLOC();

    if (OS_IntNestingLvl > 0U) {                                /* Don't call from an ISR.                      */
        return;
    }
    if (OS_LockSchedNesting > 0U) {                             /* Don't delay while the scheduler is locked.   */
        return;
    }

    OS_CRTICAL_BEGIN();

    if(ticks == 0U)
    {
        OS_CRTICAL_END();
        return;
    }

    if(OS_currentTask != OS_tblTCBPrio[OS_IDLE_TASK_PRIO_LEVEL])  /* Don't allow blocking the ideal task.         */
    {
        OS_currentTask->TASK_Ticks = ticks;
        OS_currentTask->TASK_Stat |= OS_TASK_STAT_DELAY;

        OS_RemoveReady(OS_currentTask->TASK_priority);
        OS_BlockTime(OS_currentTask->TASK_priority);

        OS_Sched();                                             /* Preempt Another Task.                        */
    }

    OS_CRTICAL_END();
}

/*
 * Function:  OS_DelayTime
 * --------------------
 * Block the current task execution for a time specified in the OS_TIME structure.
 *
 * Arguments    :   ptime   is a pointer to an OS_TIME structure where time is specified ( Hours, Minutes, seconds and milliseconds.)
 *
 * Returns      :   None.
 *
 * Note(s)      :   1) This function is called only from task level code.
 *                  2) A non valid value of any member of the internal structure of the OS_TIME object results in an immediate return.
 *                  3) This call can be expensive for some MCUs.
 */
void
OS_DelayTime(OS_TIME* ptime)
{
    OS_TICK ticks;

    if(ptime == (OS_TIME*)0U)
    {
        return;
    }

    if(ptime->minutes > 59U)
    {
        return;
    }

    if(ptime->seconds > 59U)
    {
        return;
    }

    if(ptime->milliseconds > 999U)
    {
        return;
    }

    ticks = (OS_CONFIG_TICKS_PER_SEC * ( (CPU_t32U)(ptime->seconds) + (CPU_t32U)(ptime->minutes)*60U  + (CPU_t32U)(ptime->hours)*3600U ))
            + ((OS_CONFIG_TICKS_PER_SEC * ( (CPU_t32U)(ptime->milliseconds) + (500U / OS_CONFIG_TICKS_PER_SEC) )) / 1000U);                       /* Rounded to the nearest tick. */

    OS_DelayTicks(ticks);
}

#endif

#if (OS_CONFIG_SYSTEM_TIME_SET_GET_EN == OS_CONFIG_ENABLE)
/*
 * Function:  OS_TickTimeGet
 * --------------------------
 * Obtain the current value of the time counter which keeps track of the number of clock ticks
 * occurred since the first system tick of system ISR ticker.
 *
 * Arguments    :   None.
 *
 * Returns      :   The current value of OS_TickTime
 */
OS_TICK OS_TickTimeGet (void)
{
	OS_TICK   sys_ticks;
	CPU_SR_ALLOC();

    OS_CRTICAL_BEGIN();
    sys_ticks = OS_TickTime;
    OS_CRTICAL_END();
    return (sys_ticks);
}

/*
 * Function:  OS_TickTimeSet
 * --------------------------
 * Set the OS_TickTime to a new value.
 *
 * Arguments    :   tick	is the new value of OS_TickTime to be set.
 *
 * Returns      :   None.
 */
void OS_TickTimeSet (OS_TICK tick)
{
	CPU_SR_ALLOC();

	OS_CRTICAL_BEGIN();
	OS_TickTime = tick;
	OS_CRTICAL_END();
}

/*
 * Function:  OS_TimeGet
 * --------------------------
 * Obtain the current value of system time in OS_TIME structure.
 *
 * Arguments    :   ptime	is a pointer to a valid OS_TIME structure which will contain the current system time.
 *
 * Returns      :   None.
 */
void OS_TimeGet (OS_TIME* ptime)
{
    OS_TICK 	sys_ticks;
    CPU_t32U	systime_ms;

    if(ptime == (OS_TIME*)0U)
    {
        return;
    }

    sys_ticks = OS_TickTimeGet();

    systime_ms = (sys_ticks*1000U / OS_CONFIG_TICKS_PER_SEC);

    ptime->hours 	 = systime_ms / (60*60*1000);
    systime_ms  	-= ptime->hours * (60*60*1000);

    ptime->minutes 	 = systime_ms / (60*1000);
    systime_ms 		-= ptime->minutes * (60*1000);

    ptime->seconds 	 = systime_ms / (1000);
    systime_ms 		-= ptime->seconds * (1000);

    ptime->milliseconds = systime_ms;
}

#endif /* OS_CONFIG_SYSTEM_TIME_SET_GET_EN */
