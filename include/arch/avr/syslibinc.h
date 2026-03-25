/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_AVR_SYSLIBINC_H_
#define OSCR_AVR_SYSLIBINC_H_

# include "arch.h"

# if defined(OSCR_ARCH_AVR)

#   include <stdint.h>
#   include <Arduino.h>
#   include <SPI.h>
#   include <Wire.h>
#   include <avr/pgmspace.h>
#   include <avr/wdt.h>
//#   include <EEPROM.h>
#   include <SdFat.h>
#   include <assert.h>
#   include <util/atomic.h>

#   include "config.h"

#   include "arch/avr/generic.h"

# endif /* OSCR_ARCH_AVR */

#endif /* OSCR_AVR_SYSLIBINC_H_ */
