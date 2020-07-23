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
 * Purpose  : Contains the implementation of CPU_CountLeadZeros() instruction in C. If no assembly instruction is supported.
 *
 * Language:  C
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"
#include "pretty_shared.h"



#ifndef CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT
#error "CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT Must be defined in pretty_arch.h"
#endif

#if(CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT == 0U)

#ifdef __cplusplus
extern "C" {
#endif

#if (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_08)
CPU_tWORD CntLeadZeros08 (CPU_t08U val)
{
    return (0);
}
#endif

#if (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_16)
CPU_tWORD CntLeadZeros16 (CPU_t16U val)
{
    return (0);
}
#endif

#if (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_32)
CPU_tWORD CntLeadZeros32 (CPU_t32U val)
{
    return (0);
}
#endif

#if (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_64)
CPU_tWORD CntLeadZeros64 (CPU_t64U val)
{
    return (0);
}
#endif

CPU_tWORD CPU_CountLeadZeros(CPU_tWORD val)
{
    CPU_tWORD  number_of_lead_zeros;

#if   (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_08)
    number_of_lead_zeros = CntLeadZeros08((CPU_t08U)val);

#elif (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_16)
    number_of_lead_zeros = CntLeadZeros16((CPU_t16U)val);

#elif (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_32)
    number_of_lead_zeros = CntLeadZeros32((CPU_t32U)val);

#elif (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_64)
    number_of_lead_zeros = CntLeadZeros64((CPU_t64U)val);

#else                                                         
        #error "Undefined CPU_tWord type."
#endif


    return (number_of_lead_zeros);
}

#ifdef __cplusplus
}
#endif

#endif      /* End of CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT   */

