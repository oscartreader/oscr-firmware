/**
 * @file
 * @brief Support for AVR Architectures
 */
#pragma once
#if !defined(OSCR_AVR_GENERIC_H_)
# define OSCR_AVR_GENERIC_H_

# include "arch/avr/syslibinc.h"

# if defined(OSCR_ARCH_AVR)
#   include "arch/avr/atmega2560/pins.h"
# endif /* OSCR_ARCH_AVR */

#endif /* OSCR_AVR_GENERIC_H_ */
