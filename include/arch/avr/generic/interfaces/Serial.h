/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#ifndef OSCR_AVR_SERIAL_H_
#define OSCR_AVR_SERIAL_H_
#pragma once

# include "arch/avr/syslibinc.h"

# if defined(OSCR_ARCH_AVR)
#   include "hardware/outputs/Serial.h"
#   include "hardware/outputs/SerialANSI.h"

namespace OSCR::Serial
{
  void clockSkew(uint32_t sclock);
}

# endif

#endif
