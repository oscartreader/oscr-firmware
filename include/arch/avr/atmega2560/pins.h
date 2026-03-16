/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_MEGA2560_PINS_H_
# define OSCR_MEGA2560_PINS_H_

# include "arch/avr/syslibinc.h"

# if defined(OSCR_ARCH_AVR)

#   define BANKSET_SUPPORTED_BANKS 4

namespace OSCR::Hardware
{
  enum class PinBank : uint8_t
  {
    SetA = 0,  // [0] Bank A
    SetC,      // [1] Bank C
    SetF,      // [2] Bank F
    SetH,      // [3] Bank H
    SetK,      // [4] Bank K
    SetL,      // [5] Bank L

    // Used pin by pin
    SetG,      // [6] Bank G
  };

  enum class Port : uint8_t
  {
    SetA = 0,  // [0] Bank A
    SetB,      // [1] Bank B
    SetC,      // [1] Bank C
    SetD,      // [1] Bank D
    SetE,      // [1] Bank E
    SetF,      // [2] Bank F
    SetG,      // [2] Bank G
    SetH,      // [3] Bank H
    SetJ,      // [3] Bank J
    SetK,      // [4] Bank K
    SetL,      // [5] Bank L
  };

  enum class Pins
  {
    // ...
  };

  constexpr uint8_t const kTotalPinBanks = 7;
  constexpr uint8_t const kAvailablePinBanks = 6;
  constexpr uint8_t const kBanksetSupportedBanks = BANKSET_SUPPORTED_BANKS;
}

# endif /* OSCR_ARCH_AVR */

#endif /* OSCR_MEGA2560_PINS_H_ */
