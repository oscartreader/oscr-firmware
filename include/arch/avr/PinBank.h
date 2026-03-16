/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_AVR_PINBANK_H_
#define OSCR_AVR_PINBANK_H_

#include "arch/avr/syslibinc.h"

#if defined(OSCR_ARCH_AVR) && defined(ENABLE_PINCONTROL)

# include "common/OSCR.h"
# include "common/Util.h"

namespace OSCR::Hardware
{
  class PinBankBase
  {

  };
}

# endif /* OSCR_ARCH_AVR */

#endif /* OSCR_AVR_PINBANK_H_ */
