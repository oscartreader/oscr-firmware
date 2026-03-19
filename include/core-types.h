/**
 * @file
 * @brief Define core and feature IDs
 *
 * Core (and feature) IDs should be defined in this file. Take care
 * that the IDs defined in this file never change. The enums here
 * do not check if the feature is enabled because the ID assignements
 * MUST remain constant over builds and time. Even if a feature is
 * removed, the definition for it must remain here.
 *
 * If a feature or core no longer exists and you want to note it here,
 * you could either add a comment after it or add `__REMOVED` to the
 * end of the name. The latter has the advantage of making any code
 * that still references it invalid.
 *
 * These IDs are meant for both internal and external tools to assist
 * with identifying and referring to enabled features by ID rather
 * than using strings. This saves resources on our extremely resource
 * constrained ATmega2560.
 *
 * Remember: Enum names don't cost PROGMEM, the amount of resources
 * used for an enum is equivilent to the size of the the enum's type,
 * For instance, a variable using an enum with a type of `uint16_t`
 * will use 1 byte of PROGMEM or SRAM to store that data. If you
 * compare that to a string, i.e. "SNES" which uses at least 5 bytes
 * of PROGMEM to store and you should be able to see why it matters.
 *
 * While it is possible to combine all of this into a single file,
 * the amount of macro abuse required to do it in a way that works
 * within the Arduino IDE bordered on a war crime. This is something
 * that will just have to wait until the move away from the Arduino
 * IDE.
 *
 * @note While you should not change the IDs, you can change their
 *       names. So, you can rename them to switch to a better name
 *       or change the case. However, do NOT hijack an ID to refer
 *       to a different core or feature.
 *
 * @sa Cores.cpp
 */
#pragma once
#ifndef OSCR_CORE_TYPES_H_
# define OSCR_CORE_TYPES_H_

#include <stdint.h>

namespace OSCR
{
  /**
   * @brief Core IDs
   *
   * Every core should have a unique ID assigned to it, even if it's
   * not listed on the main menu. These IDs are primarily used by the
   * oscr.tools updater to identify which cores are enabled.
   *
   * If you just want to change the order of the cores on the menu
   * screen, look at `Cores.cpp`.
   *
   * @warning Do NOT delete/remove or change the order of these. Add
   *          new ones at the end!
   */
  enum class CoreID : uint8_t
  {
    NONE,
    GBX,
    NES,
    SNES,
    N64,
    MD,
    SMS,
    PCE,
    WS,
    NGP,
    INTV,
    COLV,
    VBOY,
    WSV,
    PCW,
    A2600,
    ODY2,
    ARC,
    FAIRCHILD,
    SUPERACAN,
    MSX,
    POKE,
    LOOPY,
    C64,
    A5200,
    A7800,
    JAGUAR,
    LYNX,
    VECTREX,
    ATARI8,
    BALLY,
    LJ,
    LJPRO,
    PV1000,
    VIC20,
    LEAP,
    RCA,
    TI99,
    PYUUTA,
    TRS80,
    VSMILE,
    FLASH8,
    CPS3,
    SELFTEST,
  };

  /**
   * @brief Language IDs
   */
  enum class LanguageID : uint8_t
  {
    EN,
    JA,
  };

  /**
   * @brief Region IDs
   */
  enum class RegionID : uint8_t
  {
    Global, //!< All/World/Global
    AF, //!< Africa
    AN, //!< Antarctica (for completeness)
    AS, //!< Asia
    EU, //!< Europe
    NA, //!< North America
    OC, //!< Oceania
    SA, //!< South America
  };

  /**
   * @brief Feature IDs
   *
   * Every feature should have a unique ID assigned to it. These IDs
   * are primarily used by the oscr.tools updater to identify which
   * features are enabled.
   *
   * @warning Do NOT delete/remove or change the order of these. Add
   *          new ones at the end!
   */
  enum class FeatureID : uint8_t
  {
    Updater,              //!< Support updating via the OSCR.Tools Firmware Updater
    Config,               //!< Allow the use of config.txt
    ClockGen,             //!< Clock Generator
    VSelect,              //!< Automatic Voltage Select (VSELECT) via TPS211x
    RealTimeClock,        //!< RTC Support
    ClockGenCalibrated,    //!< Support for calibrating the clock generator
    ClockGenCalibration,  //!< Load clock generator calibration data
    OnBoardMega,          //!< Enable on-board ATmega2560 options
    StabilityFix,         //!< Fix for ATmega2560s with stability issues at 3.3V
    PowerSaving,          //!< Power Saving
  };

  /**
   * @brief Output Interface IDs
   *
   * Every output interface needs a unique ID assigned to it. These
   * IDs are primarily used by the oscr.tools updater to identify
   * which interfaces are enabled.
   *
   * @warning Do NOT delete/remove or change the order of these. Add
   *          new ones at the end!
   */
  enum class OutputInterfaceID : uint8_t
  {
    Serial,     //!< ASCII Serial
    SerialANSI, //!< ANSI Serial
    SSD1306,    //!< OLED
    OS12864,    //!< LCD (a.k.a. MKS/BTT MINI12864, ST7567, etc)
  };

  /**
   * @brief Input Interface IDs
   *
   * Every input interface needs a unique ID assigned to it. These
   * IDs are primarily used by the oscr.tools updater to identify
   * which interfaces are enabled.
   *
   * @warning Do NOT delete/remove or change the order of these. Add
   *          new ones at the end!
   */
  enum class InputInterfaceID : uint8_t
  {
    Serial,       //!< ASCII Serial
    SerialANSI,   //!< ANSI Serial
    OneButton,    //!< 1-button interface (HW1-2)
    TwoButtons,   //!< 2-button interface (HW3)
    RotaryButton, //!< Rotary-button interface (HW4-5)
  };

  /**
   * @brief Option IDs
   *
   * Every major option should have a unique ID assigned to it. These
   * IDs are primarily used by the oscr.tools updater to identify
   * which options are toggled.
   *
   * @warning Do NOT delete/remove or change the order of these. Add
   *          new ones at the end!
   */
  enum class OptionID : uint8_t
  {
    LCDType,
    NeoPixelOrder,
    VoltageSpecifier,
    VoltageMonitorMethod,
  };
}

#endif /* OSCR_CORE_TYPES_H_ */
