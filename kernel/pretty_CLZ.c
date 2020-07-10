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


#include "pretty_os.h"


#ifndef CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT
#error "CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT Must be defined in pretty_arch.h"
#endif

#if(CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT == 0U)

#ifdef __cplusplus
extern "C" {
#endif

CPU_tWORD CPU_CountLeadZeros(CPU_tWORD val)
{
    return (0U);
}

#ifdef __cplusplus
}
#endif

#endif

