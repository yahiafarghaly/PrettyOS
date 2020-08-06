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
 * Purpose  : Contains the implementation of CPU_CountLeadZeros()in C
 *            which can be used if no assembly instruction is supported by the target processor/Compiler.
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

/*
 * Fixed values of known leading zeros of numbers from 0x00 to 0xFF.
 * This will be used for fast calculating the leading zeros for numbers larger than 0xFF ( more than 1 byte.)           */
static const CPU_t08U CPU_CntLeadZerosTbl[256] = {                                  /* Data values :                    */
/*   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F   */
    8u,  7u,  6u,  6u,  5u,  5u,  5u,  5u,  4u,  4u,  4u,  4u,  4u,  4u,  4u,  4u,  /*   0x00 to 0x0F                   */
    3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  3u,  /*   0x10 to 0x1F                   */
    2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  /*   0x20 to 0x2F                   */
    2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  2u,  /*   0x30 to 0x3F                   */
    1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  /*   0x40 to 0x4F                   */
    1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  /*   0x50 to 0x5F                   */
    1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  /*   0x60 to 0x6F                   */
    1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  1u,  /*   0x70 to 0x7F                   */
    0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  /*   0x80 to 0x8F                   */
    0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  /*   0x90 to 0x9F                   */
    0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  /*   0xA0 to 0xAF                   */
    0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  /*   0xB0 to 0xBF                   */
    0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  /*   0xC0 to 0xCF                   */
    0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  /*   0xD0 to 0xDF                   */
    0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  /*   0xE0 to 0xEF                   */
    0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u   /*   0xF0 to 0xFF                   */
};

#endif

#if(CPU_CONFIG_COUNT_LEAD_ZEROS_ASM_PRESENT == 0U)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Declare CntLeadZeros*() as static inline to prevent linker complaining about
 * not finding a reference for a function if it's declared only with inline keyword. */
#if (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_08)
CPU_tWORD static inline CntLeadZeros08 (CPU_t08U val)
{
    return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)val]);
}
#endif

#if (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_16)
CPU_tWORD static inline CntLeadZeros16 (CPU_t16U val)
{
    if(val > 0x00FFU)
    {
        return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)(val >> 8U)]);
    }
    else
    {
        return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)val] + 8U);   /* 8 bits of most byte + #number of bits in the  least byte. */
    }
}
#endif

#if (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_32)
CPU_tWORD static inline CntLeadZeros32 (CPU_t32U val)
{
    if(val > 0x0000FFFFU)
    {
        if(val > 0x00FFFFFFU)
        {
            return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)(val >> 24U)] + 0U);
        }
        else
        {
            return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)(val >> 16U)] + 8U);
        }
    }
    else
    {
        if(val > 0x00FFU)
        {
            return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)(val >> 8U)] + 16U);
        }
        else
        {
            return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)val] + 24U);
        }
    }
}
#endif

#if (CPU_CONFIG_DATA_SIZE_BITS == CPU_WORD_SIZE_64)
CPU_tWORD static inline CntLeadZeros64 (CPU_t64U val)
{
    if (val > 0x00000000FFFFFFFFU) {
        if(val > 0x0000FFFFFFFFFFFFU)
        {
            if(val > 0x00FFFFFFFFFFFFFFU)
            {
                return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)(val >> 56U)] + 0U);
            }
            else
            {
                return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)(val >> 48U)] + 8U);
            }
        }
        else
        {
            if (val > 0x000000FFFFFFFFFFU)
            {
                return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)(val >> 40U)] + 16U);
            }
            else
            {
                return (CPU_tWORD)(CPU_CntLeadZerosTbl[(CPU_tWORD)(val >> 32U)] + 24U);
            }
        }
    }
    else
    {
        return (CPU_tWORD)(CntLeadZeros32((CPU_t32U)val) + 32U);
    }
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

