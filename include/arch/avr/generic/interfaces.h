/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#if !defined(OSCR_AVR_INTERFACES_H_)
# define OSCR_AVR_INTERFACES_H_

# include "arch/avr/syslibinc.h"

# if defined(OSCR_ARCH_AVR)
#   include "arch/avr/generic/interfaces/ClockedSerial.h"
# endif /* OSCR_ARCH_AVR */

#endif /* OSCR_AVR_INTERFACES_H_ */
