/**
 * @file
 * @brief Support Multiple Architectures
 *
 * This file is a skeleton for eventually supporting targets other
 * than the Arduino Mega/ATmega2560.
 */
#pragma once
#ifndef OSCR_ARCH_H_
# define OSCR_ARCH_H_ 1

# if defined(ARDUINO_ARCH_AVR)

#   if defined(ARDUINO_AVR_MEGA2560)

#     define OSCR_ARCH_AVR 1
#     define OSCR_TARGET_MEGA2560 1
#     define BANKSET_MAX_BANKS 4

#     include "arch/avr.h"

#   else // probably ARDUINO_AVR_UNO / ARDUINO_AVR_NANO (ATmega328p)

#     error "Unsupported target architecture."

#   endif

# elif defined(TARGET_STM32) || defined(ARDUINO_ARCH_MBED)

#   if defined(TARGET_STM32H7) || defined(ARDUINO_GIGA)

#     define OSCR_ARCH_STM32 1
#     define OSCR_TARGET_STM32H7 1

#     pragma message !! Support for STM32H7 is experimental !!

#     include "arch/stm32.h"

#   elif defined(TARGET_STM32MP)

#     define OSCR_ARCH_STM32 1
#     define OSCR_TARGET_STM32MP 1

#     pragma message !! Support for STM32MP is experimental !!

#     include "arch/stm32.h"

#   else // Some other STM32

#     error "Unsupported target architecture."

#   endif

# else

#   error "Unsupported target architecture."

# endif /* ARDUINO_ARCH_AVR && TARGET_STM32 */

#endif /* OSCR_ARCH_H_ */
