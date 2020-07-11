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

#if(OS_CONFIG_ERRNO_EN == 1U)
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
    switch(errno)
    {
    case OS_ERR_NONE:
        return "Success";
    case OS_ERR_PARAM:
        return "Invalid parameter";
    case OS_ERR_PRIO_EXIST:
        return "Reserved priority";
    default:
        break;
    }
    return "Unknown error code";
}


