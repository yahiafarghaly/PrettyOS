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
#include "pretty_os.h"

#if(OS_CONFIG_ERRNO_EN == OS_CONFIG_ENABLE)
    OS_ERR OS_ERRNO = OS_ERR_NONE;              /* Holds the last error code returned by the last executed prettyOS function. */
#endif

/*
 * Function:  OS_StrError
 * --------------------
 * Return a constant string describing the error code.
 *
 * Arguments    : errno         is an error code defined in the enum list of OS_ERR
 *
 * Returns      :               A const pointer to a const char array values describing the error code.
 */
char const* const
OS_StrError(OS_ERR errno)
{
#if(OS_CONFIG_ERRNO_EN == OS_CONFIG_ENABLE)

#define str(x) #x               /* stringify the input x                                     */
#define xstr(x) str(x)          /* expand input x first if necessary, then stringify it.     */

    switch(errno)
    {
    case OS_ERR_NONE:
        return xstr(OS_ERR_NONE);

    case OS_ERR_PARAM:
        return xstr(OS_ERR_PARAM);

    case OS_ERR_PRIO_EXIST:
        return xstr(OS_ERR_PRIO_EXIST);

    case OS_ERR_PRIO_INVALID:
        return xstr(OS_ERR_PRIO_INVALID);

    case OS_ERR_TASK_CREATE_ISR:
        return xstr(OS_ERR_TASK_CREATE_ISR);

    case OS_ERR_TASK_SUSPEND_IDLE:
        return xstr(OS_ERR_TASK_SUSPEND_IDLE);

    case OS_ERR_TASK_SUSPEND_PRIO:
        return xstr(OS_ERR_TASK_SUSPEND_PRIO);

    case OS_ERR_TASK_SUSPENDED:
        return xstr(OS_ERR_TASK_SUSPENDED);

    case OS_ERR_TASK_CREATE_EXIST:
        return xstr(OS_ERR_TASK_CREATE_EXIST);

    case OS_ERR_TASK_RESUME_PRIO:
        return xstr(OS_ERR_TASK_RESUME_PRIO);

    case OS_ERR_TASK_NOT_EXIST:
        return xstr(OS_ERR_TASK_NOT_EXIST);

    case OS_ERR_TASK_DELETE_ISR:
        return xstr(OS_ERR_TASK_DELETE_ISR);

    case OS_ERR_TASK_DELETE_IDLE:
        return xstr(OS_ERR_TASK_DELETE_IDLE);

    case OS_ERR_EVENT_PEVENT_NULL:
        return xstr(OS_ERR_EVENT_PEVENT_NULL);

    case OS_ERR_EVENT_TYPE:
        return xstr(OS_ERR_EVENT_TYPE);

    case OS_ERR_EVENT_PEND_ISR:
        return xstr(OS_ERR_EVENT_PEND_ISR);

    case OS_ERR_EVENT_PEND_LOCKED:
        return xstr(OS_ERR_EVENT_PEND_LOCKED);

    case OS_ERR_EVENT_PEND_ABORT:
        return xstr(OS_ERR_EVENT_PEND_ABORT);

    case OS_ERR_EVENT_POST_ISR:
        return xstr(OS_ERR_EVENT_POST_ISR);

    case OS_ERR_EVENT_TIMEOUT:
        return xstr(OS_ERR_EVENT_TIMEOUT);

    case OS_ERR_EVENT_POOL_EMPTY:
        return xstr(OS_ERR_EVENT_POOL_EMPTY);

    case OS_ERR_EVENT_CREATE_ISR:
        return xstr(OS_ERR_EVENT_CREATE_ISR);

    case OS_ERR_MUTEX_LOWER_PCP:
        return xstr(OS_ERR_MUTEX_LOWER_PCP);

    case OS_ERR_MUTEX_NO_OWNER:
        return xstr(OS_ERR_MUTEX_NO_OWNER);

    case OS_ERR_MAILBOX_POST_NULL:
        return xstr(OS_ERR_MAILBOX_POST_NULL);

    case OS_ERR_MAILBOX_FULL:
        return xstr(OS_ERR_MAILBOX_FULL);

    case OS_ERR_SEM_OVERFLOW:
        return xstr(OS_ERR_SEM_OVERFLOW);

    case OS_ERR_MEM_INVALID_ADDR:
        return xstr(OS_ERR_MEM_INVALID_ADDR);

    case OS_ERR_MEM_INVALID_BLOCK_SIZE:
        return xstr(OS_ERR_MEM_INVALID_BLOCK_SIZE);

    case OS_ERR_MEM_NO_FREE_BLOCKS:
        return xstr(OS_ERR_MEM_NO_FREE_BLOCKS);

    case OS_ERR_MEM_FULL_PARTITION:
        return xstr(OS_ERR_MEM_FULL_PARTITION);

    default:
        break;
    }

    return "Unknown Error Code.";
#else
    return "Error Code is not Supported [OS_CONFIG_ERRNO_EN = OS_CONFIG_DISABLE]."
#endif                                      /* END of OS_CONFIG_ERRNO_EN == OS_CONFIG_ENABLE */
}

/*
 * Function:  OS_StrLastErrIfFail
 * --------------------
 * Return a constant string describing the last error occurred.
 *
 * Arguments    :    None.
 *
 * Returns      :    A const pointer to a const char array values describing the error code.
 *                   "Success" string if OS_ERR_NONE was the last error.
 */
char const* const
OS_StrLastErrIfFail (void)
{
#if(OS_CONFIG_ERRNO_EN == OS_CONFIG_ENABLE)
    if (OS_ERRNO != OS_ERR_NONE)
    {
        return OS_StrError(OS_ERRNO);
    }

    return "Success";
#else
    return "Error Code is not Supported [OS_CONFIG_ERRNO_EN = OS_CONFIG_DISABLE]."
#endif                                      /* END of OS_CONFIG_ERRNO_EN == OS_CONFIG_ENABLE */
}


