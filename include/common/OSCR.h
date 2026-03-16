/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_H_
#define OSCR_H_

#include "config.h"
#include "syslibinc.h"
#include "ui/l10n.h"

#include "common/specializations.h"
#include "common/Types.h"
#include "common/crc32.h"

#if defined(ARDUINO_ARCH_AVR)
# include <EEPROM.h>
#endif

/**
 * @brief Main program
 */
namespace OSCR
{
  inline constexpr HW const kHardwareVersion = OSCR_HARDWARE_VERSION;

  inline constexpr OutputInterfaceID const kHardwareOutput =
#if (HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864)
    OutputInterfaceID::OS12864
#elif defined(ENABLE_OLED)
    OutputInterfaceID::SSD1306
#else
    OutputInterfaceID::Serial
#endif
  ;

  inline constexpr InputInterfaceID const kHardwareInput =
#if (HARDWARE_INPUT_TYPE == INPUT_ROTARY)
    InputInterfaceID::RotaryButton
#elif (HARDWARE_INPUT_TYPE == INPUT_2BUTTON)
    InputInterfaceID::TwoButtons
#elif (HARDWARE_INPUT_TYPE == INPUT_1BUTTON)
    InputInterfaceID::OneButton
#elif (HARDWARE_INPUT_TYPE == INPUT_SERIAL)
    InputInterfaceID::Serial
#else
# error Unknown Input Method
#endif
  ;

  inline constexpr uint16_t const kFirmwareFeatures =
    OSCR_FEATURE_FLAG(HAS_UPDATER,              FeatureID::Updater              ) |
    OSCR_FEATURE_FLAG(HAS_CONFIG,               FeatureID::Config               ) |
    OSCR_FEATURE_FLAG(HAS_CLOCKGEN,             FeatureID::ClockGen             ) |
    OSCR_FEATURE_FLAG(HAS_CLOCKGEN_CALIBRATED,  FeatureID::ClockGenCalibrated   ) |
    OSCR_FEATURE_FLAG(HAS_CLOCKGEN_CALIBRATION, FeatureID::ClockGenCalibration  ) |
    OSCR_FEATURE_FLAG(HAS_VSELECT,              FeatureID::VSelect              ) |
    OSCR_FEATURE_FLAG(HAS_RTC,                  FeatureID::RealTimeClock        ) |
    OSCR_FEATURE_FLAG(HAS_ONBOARD_ATMEGA,       FeatureID::OnBoardMega          ) |
    OSCR_FEATURE_FLAG(HAS_STABILITYFIX,         FeatureID::StabilityFix         ) |
    OSCR_FEATURE_FLAG(HAS_POWERSAVING,          FeatureID::PowerSaving          ) |
    0;

  inline constexpr LanguageID const kFirmwareLanguage =
# if   (OSCR_LANGUAGE == LANG_EN)
    LanguageID::EN
# endif
  ;

  inline constexpr RegionID const kFirmwareRegion =
# if   (OSCR_REGION == REGN_AF) /* Africa */
    RegionID::AF
# elif (OSCR_REGION == REGN_AN) /* Antarctica (for completeness) */
    RegionID::AN
# elif (OSCR_REGION == REGN_AS) /* Asia */
    RegionID::AS
# elif (OSCR_REGION == REGN_EU) /* Europe */
    RegionID::EU
# elif (OSCR_REGION == REGN_NA) /* North America */
    RegionID::NA
# elif (OSCR_REGION == REGN_OC) /* Oceania */
    RegionID::OC
# elif (OSCR_REGION == REGN_SA) /* South America */
    RegionID::SA
# else
    RegionID::Global
# endif
  ;

  /**
   * @brief String constants
   */
  namespace Strings
  {
    // Version
    __constinit extern char const PROGMEM Version[];

    /**
     * @brief Core-specific string constants
     */
    namespace Cores
    {
      // ...
    }
  }

  /**
   * @brief Menu options for internal features
   */
  namespace Menus
  {
    /**
     * @brief List of cores shown on the main menu screen.
     *
     * Changing the order of the cores in this array will change their
     * display order on the main menu as well.
     */
    __constinit extern CoreDefinition const MainMenuOptions[];

    __constinit extern uint8_t const kMainMenuOptionCount;
    __constinit extern uint8_t const kMainMenuNonCoreCount;
    __constinit extern uint8_t const kMainMenuCoreCount;

    /**
     * Renders Main Menu and executes menu method for the selected core.
     */
    extern void main();
  } /* namespace Menus */

  extern void tick();
  extern void idle();
  extern void busy();

# if defined(ENABLE_POWERSAVING)
  extern bool wakeEvent();
#else
  inline constexpr bool wakeEvent()
  {
    return false;
  }
# endif

  extern void aboutMenu();

  [[ noreturn ]] extern void resetArduino();
  [[ noreturn ]] extern void resetMenu();
}

// CRDB
#define CRDB_RECORD_SIZE_BASIC          104
#define CRDB_RECORD_SIZE_STANDARD       112
#define CRDB_RECORD_SIZE_EXTENDED       116
#define CRDB_RECORD_SIZE_BASIC_MAPPER   116
#define CRDB_RECORD_SIZE_NES            138
#define CRDB_RECORD_SIZE_NES_MAPPER     100
#define CRDB_RECORD_SIZE_SNES           116
#define CRDB_RECORD_SIZE_GBA            112
#define CRDB_RECORD_SIZE_N64            111
#define CRDB_RECORD_SIZE_A2600          106
#define CRDB_RECORD_SIZE_A5200          108
#define CRDB_RECORD_SIZE_JAG            108
#define CRDB_RECORD_SIZE_TI99           112

enum PIXEL: uint8_t
{
#if OPTION_NEOPIXEL_ORDER == 1
  PIXEL_BG,
  PIXEL_BTN1,
  PIXEL_BTN2,
#elif OPTION_NEOPIXEL_ORDER == 2
  PIXEL_BTN1,
  PIXEL_BTN2,
  PIXEL_BG,
#endif
};

#if defined(ENABLE_GLOBAL_LOG)
extern FsFile myLog;
#endif

template<class T>
int EEPROM_writeAnything(int ee, T const & value)
{
#if defined(ARDUINO_ARCH_AVR)
  uint8_t const * p = (uint8_t const *)(void const *)&value;
  uint16_t i;

  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);

  return i;
#else
  return 0;
#endif
}

template<class T>
int EEPROM_readAnything(size_t ee, T & value)
{
#if defined(ARDUINO_ARCH_AVR)
  uint8_t* p = (uint8_t*)(void*)&value;
  uint16_t i;

  for (i = 0; i < sizeof(value); i++)
  {
    *p++ = EEPROM.read(ee++);
  }

  return i;
#else
  return 0;
#endif
}

#endif /* OSCR_H_ */
