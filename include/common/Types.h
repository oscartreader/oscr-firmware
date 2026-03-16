#pragma once
#ifndef OSCR_TYPES_H_
#define OSCR_TYPES_H_

#include <stdint.h>
#include "config.h"
#include "core-types.h"
#include "common/crc32_t.h"

namespace OSCR
{
  typedef void (*ConfigCallback)();

  enum class HW : uint8_t
  {
    Custom,
    V1,
    V2,
    V3,
    V4,
    V5,
  };

  /**
   * @brief Metadata for OSCR system cores and main menu options.
   *
   * @param id    Should be `0` for menu-only options. For cores, use a unique integer that never changes.
   * @param title Pointer to the menu entry name to use.
   * @param menu  Entry function to be executed when selected from the main menu.
   */
  struct CoreDefinition
  {
    CoreID const id = CoreID::NONE;
    char const * title;
    void (*menu)();
  };

  /**
   * Enumeration for the return values of %OSCR modules.
   */
  enum class ModuleResult : uint8_t
  {
    Success,      /*!< The method call was successful. */
    Error,        /*!< An error occurred. */
    NotEnabled,   /*!< The module is not enabled or is unavailable */
    Unknown       /*!< Unknown */
  };

  /**
   * Enumeration of clock speeds.
   */
  enum class ClockSpeed : bool
  {
    //! @hideinitializer
    k16MHz  = 0,      /*!< 16MHz (ClockScale = 0) */
    //! @hideinitializer
    k8MHz   = 1,      /*!<  8MHz (ClockScale = 1) */
    //! @hideinitializer
    Full    = k16MHz, /*!< Alias of k16MHz */
    //! @hideinitializer
    Half    = k8MHz,  /*!< Alias of k8MHz */
  };

  /**
   * Enumeration of voltages.
   */
  enum class Voltage : uint8_t
  {
    k5V,          /*!< 5V */
    k3V3,         /*!< 3.3V */
    Unknown,

    //! @hideinitializer
    k5V0 = k5V,   /*!< Alias of k5V */
    //! @hideinitializer
    k3V = k3V3,   /*!< Alias of k3V3 */

    //! @hideinitializer
#if defined(ENABLE_ONBOARD_ATMEGA)
    Default = k5V, /*!< Default of k5V (OBMEGA) */
#else /* !ENABLE_ONBOARD_ATMEGA */
    Default = k3V3, /*!< Default of k3V3 (MEGAMOD) */
#endif /* ENABLE_ONBOARD_ATMEGA */
  };

  /**
   * Enumeration of sleep states
   */
  enum class SleepState : uint8_t
  {
    Awake,
    Drowsy,
    Sleep,
    DeepSleep,
  };

  /**
   * Enumeration of data directions
   */
  enum class DataDirection : uint8_t
  {
    Unknown,
    In,
    Out,
  };

  /**
   * @brief Data struct for the file browser
   */
  struct BrowserFile
  {
    uint32_t index;
    char name[UI_FILE_BROWSER_FILENAME_MAX + 1];
    bool isDir = false;
  };

  namespace UI
  {
    /**
     * Enumeration of user input events.
     */
    enum UserInput: uint8_t
    {
      kUserInputIgnore,
      kUserInputConfirm,
      kUserInputNext,
      kUserInputBack,
      kUserInputConfirmShort,
      kUserInputConfirmLong,
      kUserInputUnknown,
    };

    /**
     * Enumeration of interface input events.
     */
    enum InterfaceInput: uint8_t
    {
      kInterfaceInputIgnore,
      kInterfaceInputPress,
      kInterfaceInputPressDouble,
      kInterfaceInputPressShort,
      kInterfaceInputPressLong,
    };

    /**
     * Enumeration of navigation events.
     */
    enum NavDir: bool
    {
      kNavDirBackward,
      kNavDirForward,
    };

    /**
     * @brief A structured RGB value.
     */
    struct rgb_t
    {
      uint8_t red;
      uint8_t green;
      uint8_t blue;
    };

    /**
     * Menu modes for the type of array that was passed to the Menu constructor.
     */
    enum MenuMode: uint8_t
    {
      kMenuFlashString,
      kMenuRamFlashString,
      kMenuOptionStruct,
      kMenuNullChar,
      kMenuFlashStringTemplate,
      kMenuIntegerTemplate,
    };

  }

  namespace Serial
  {
    enum class Style : uint8_t
    {
      Reset = 0,
      Bold, // also called "Bright"
      Faint, // ECMA-48
      Italic,
      Underline,
      Blink,
      // 6 = Unused
      Inverse = 7,
      Invisible, // ECMA-48
      Strikethrough, // ECMA-48
    };

    enum class Foreground : uint8_t
    {
      Black = 30,
      Red,
      Green,
      Yellow,
      Blue,
      Magenta,
      Cyan,
      White,
      Default, // ECMA-48
    };

    enum class Background : uint8_t
    {
      Black = 40,
      Red,
      Green,
      Yellow,
      Blue,
      Magenta,
      Cyan,
      White,
      Default, // ECMA-48
    };

    union Format
    {
      uint8_t value;
      Style style;
      Foreground foreground;
      Background background;
    };

    enum class Cursor: bool
    {
      Show,
      Hide,
    };
  }
}

#endif /* OSCR_TYPES_H_ */
