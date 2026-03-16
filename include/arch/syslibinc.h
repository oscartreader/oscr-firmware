/**
 * @file
 * @brief Architecture-dependent system libraries
 * @sa arch.h
 */
#pragma once
#ifndef OSCR_ARCH_SYSLIBINC_H_
# define OSCR_ARCH_SYSLIBINC_H_

# include "arch.h"

# if defined(OSCR_ARCH_AVR)
#   include "arch/avr/syslibinc.h"
# elif defined(OSCR_ARCH_STM32)
#   include "arch/stm32/syslibinc.h"
# else
#   error "Unsupported target architecture."
# endif /* OSCR_ARCH_* */

#endif /* OSCR_ARCH_SYSLIBINC_H_ */
