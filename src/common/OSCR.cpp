/********************************************************************
*                   Open Source Cartridge Reader                    */
/*H******************************************************************
* FILENAME :        OSCR.cpp
*
* DESCRIPTION :
*       Contains various enums, variables, etc, for the main program.
*
* NOTES :
*       This file is a WIP, I've been moving things into it on my local working
*       copy, but they are not ready yet. Rather than put this in the main file
*       only to move them back again, I decided to commit it early. If you need
*       to add new globals, enums, defines, etc, please use this file!
*
* LICENSE :
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation, either version 3 of the License, or
*       (at your option) any later version.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License
*       along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
* CHANGES :
*
* REF NO    VERSION  DATE        WHO            DETAIL
*           13.2     2024-03-02  Ancyker        Add string constants
*           13.2     2024-02-29  Ancyker        Add config support
*           12.5     2023-03-29  Ancyker        Initial version
*
*H*/
#include "arch.h"
#include "common.h"
#include "hardware.h"
#include "ui.h"

#if !defined(ARDUINO_AVR_MEGA2560)
uint8_t fakePin = 0xFF;
uint8_t fakePort = 0xFF;
uint8_t fakeDDR = 0xFF;
#endif

namespace OSCR
{
  constexpr uint8_t const kFirmwareVersionMajor = MAJOR_VERSION;
  constexpr uint8_t const kFirmwareVersionMinor = MINOR_VERSION;
  constexpr uint8_t const kFirmwareVersionPatch = PATCH_VERSION;

#pragma region Strings

  namespace Strings
  {
    constexpr char PROGMEM Version[] = VERSION_STRING;
  }

#pragma region Methods

  void tick()
  {
# if defined(ENABLE_UPDATER) && !defined(ENABLE_SERIAL_OUTPUT) // Updater handled by interface when used as output
    OSCR::Updater::check();
# endif /* ENABLE_UPDATER */

    /**
     * It's important that the `tick()` method does not get optimized
     * away because `tick()` might be the only method called inside
     * of a while loop and if it is optimized away then the loop may
     * also be optimized away.
     *
     * This is not an `else` because if other methods are added it
     * can either be removed or checks for them need added as well.
     *
     * Note: This needs to check for other methods that may be valid,
     *  even if they aren't enabled, such as `Updater::check()`, due
     *  to the fact they get optimized away if they aren't enbled.
     */
# if !defined(ENABLE_UPDATER)
    NOP;
# endif /* !ENABLE_UPDATER */
  }

  void idle()
  {
# if defined(ENABLE_POWERSAVING)
    OSCR::Power::idle();
# endif /* ENABLE_POWERSAVING */

    /**
     * It's important that the `idle()` method does not get optimized
     * away because it might be the only method called in loop and if
     * it's optimized away then the while loop itself would be too.
     *
     * This is not an `else` because if other methods are added it
     * can either be removed or checks for them need added as well.
     *
     * Note: This needs to check for other methods that may be valid,
     *  even if they aren't enabled, such as `Power::idle()`, due to
     *  the fact they get optimized away if they aren't enbled.
     */
# if !defined(ENABLE_POWERSAVING)
    NOP;
# endif
  }

  void busy()
  {
# if defined(ENABLE_POWERSAVING)
    OSCR::Power::busy();
# endif /* ENABLE_POWERSAVING */

    /**
     * This doesn't need the checks `tick()`/`idle()` do because
     * it's ok if it gets optimized away.
     */
  }

# if defined(ENABLE_POWERSAVING)
  bool wakeEvent()
  {
    return OSCR::Power::wakeEvent();
  }
# endif

  [[ noreturn ]] void resetArduino()
  {
    // Reset CIC
    DDRG |= (1 << 1);
    PORTG |= (1 << 1);

    // Disable cart power in case it hasn't already been
    OSCR::Power::disableCartridge();

#if defined(ENABLE_CLOCKGEN)
    // If ClockGen has been initilized, reset it
    if (OSCR::ClockGen::exists())
    {
      OSCR::ClockGen::clockgen.reset();
    }
#endif /* ENABLE_CLOCKGEN */

    /**
     * Just in case this doesn't work, we print a message to inform
     * the user we intend to reset.
     */
    OSCR::UI::printHeader(FS(OSCR::Strings::Headings::OSCR));
    OSCR::UI::printLine(F("Resetting..."));
    OSCR::UI::printLine();
    OSCR::UI::printLine(F("If reset doesn't occur,"));
    OSCR::UI::printLineSync(F(" please reset manually."));

    delay(200); // Give display time to render

    wdt_enable(WDTO_15MS); // Enable WDT to reset

    // If WDT reset fails, we could fall back to jmp 0, but that won't
    //  clear vars, leaving the program in a bad state.
    //AVR_ASM(AVR_INS("jmp 0"));

    // Instead, we infinite loop to guarantee noreturn and wait for
    //  the user to reset manually.
    AVR_ASM(
      AVR_INS("LResetLoop:")
      AVR_INS("nop")
      AVR_INS("rjmp LResetLoop")
    );

    OSCR::Util::unreachable();
  }

  void aboutMenu()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Headings::OSCR));

    OSCR::UI::printLine(F("github.com/sanni"));
    OSCR::UI::printLine(FS(OSCR::Strings::Version));

    OSCR::UI::print(F("SdFat V"));
    OSCR::UI::printLine(F(SD_FAT_VERSION_STR));

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::PressButton));

    // if the confirm button is held
    if (OSCR::UI::waitInput() == OSCR::UI::UserInput::kUserInputConfirmLong)
    {
      OSCR::UI::clear();
      OSCR::UI::printLineSync(F("Resetting folder..."));

      delay(2000);

      OSCR::Storage::Shared::folderIncrement = 0;
      EEPROM_writeAnything(0, OSCR::Storage::Shared::folderIncrement);
    }
  }

  [[ noreturn ]] void resetMenu()
  {
    resetArduino();
  }
}

/*==== VARIABLES ==================================================*/

#if defined(ENABLE_CONFIG)
# if defined(ENABLE_GLOBAL_LOG)
bool loggingEnabled = true;
# endif /* ENABLE_GLOBAL_LOG */
#endif /* ENABLE_CONFIG */

/*==== /VARIABLES =================================================*/
