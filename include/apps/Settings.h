/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_SETTINGS_H_
#define OSCR_SETTINGS_H_

#include "syslibinc.h"
#include "config.h"

namespace OSCR::Settings
{
  enum class MenuOption : uint8_t
  {
# if defined(ENABLE_RTC)
    SetRTC,
# endif /* ENABLE_RTC */

# if defined(ENABLE_VSELECT)
    Voltage,
# endif /* ENABLE_VSELECT */

# if defined(ENABLE_SELFTEST)
    SelfTest,
# endif /* ENABLE_SELFTEST */

# if defined(OPTION_CLOCKGEN_CALIBRATION)
    Calibration,
# endif /* OPTION_CLOCKGEN_CALIBRATION */
    About,
    Back,
  };

  extern char const PROGMEM menuOptionSetRTC[];
  extern char const PROGMEM menuOptionVoltage[];
  extern char const PROGMEM menuOptionCalibration[];

  extern char const * const PROGMEM menuOptions[];

  extern uint8_t const kMenuOptionCount;

  void menu();
} /* namespace OSCR::Settings */

#endif/* OSCR_SETTINGS_H_ */
