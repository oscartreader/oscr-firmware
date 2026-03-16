/**
 * @file
 * @brief Target-specific macros
 * @sa arch.h
 */
#pragma once
#ifndef OSCR_ARCH_MACROS_H_
# define OSCR_ARCH_MACROS_H_

# include "arch.h"

# if defined(OSCR_ARCH_AVR)
#   include "arch/avr/macros.h"
# elif defined(OSCR_ARCH_STM32)
#   include "arch/stm32/macros.h"
# else
#   error "Unsupported target architecture."
# endif /* OSCR_ARCH_* */

#endif
