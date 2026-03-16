/**
 * @file
 * @brief Support for AVR Architectures
 */
#pragma once
#if !defined(OSCR_AVR_MACROS_H_)
# define OSCR_AVR_MACROS_H_

# include "arch.h"

# if defined(OSCR_ARCH_AVR)

#   define NOP __asm__ __volatile__("nop\n\t")

#   define AVR_ASM(x) asm( x )
#   define AVR_INS(ins) ins "\n\t"

/**
 * @def __StringHelper
 * @brief Macro that resolves to the string helper for the target, if any.
 *
 * For AVR platforms this resolves to `__FlashStringHelper`
 */
#   define __StringHelper __FlashStringHelper

/**
 * @def FS(pmem_string)
 * @brief Cast a PROGMEM string pointer (`char const *`) to a flash string (`__FlashStringHelper const *`).
 */
#   define FS(pmem_string) (reinterpret_cast<__StringHelper const *>(pmem_string))

/**
 * @def FSP(pmem_string)
 * @brief Cast a PROGMEM string pointer (`__FlashStringHelper const *`) to a flash string (`char const *`).
 */
#   define FSP(pmem_string) (reinterpret_cast<char const *>(pmem_string))

/**
 * @def FP(pmem_pointer)
 * @brief Cast a char * pointer to a flash pointer.
 */
#   define FP(pmem_pointer) ((char*)pgm_read_word(&(pmem_pointer)))


#   define LitStr(s) F(s)
#   define LitStr_P(s) PSTR(s)

# endif /* OSCR_ARCH_AVR */

#endif /* OSCR_AVR_MACROS_H_ */
