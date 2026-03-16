#include "common.h"
#include "api.h"
#include "cores/SelfTest.h"

#include "ui.h"

#include "apps/Settings.h"

namespace OSCR::Settings
{
  constexpr char const PROGMEM menuOptionSetRTC[] = "Set Date/Time";
  constexpr char const PROGMEM menuOptionVoltage[] = "Voltage";
  constexpr char const PROGMEM menuOptionCalibration[] = "ClockGen Calibration";

  constexpr char const * const PROGMEM menuOptions[] = {
# if defined(ENABLE_RTC)
    menuOptionSetRTC,
# endif /* ENABLE_RTC */

# if defined(ENABLE_VSELECT)
    menuOptionVoltage,
# endif /* ENABLE_VSELECT */

# if defined(ENABLE_SELFTEST)
    OSCR::Strings::Cores::SelfTest,
# endif /* ENABLE_SELFTEST */

# if defined(OPTION_CLOCKGEN_CALIBRATION)
    menuOptionCalibration,
# endif /* OPTION_CLOCKGEN_CALIBRATION */

    OSCR::Strings::Features::About,
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr uint8_t const kMenuOptionCount = sizeofarray(menuOptions);

  void menu()
  {
    do
    {
      if __if_constexpr (kMenuOptionCount == 2)
      {
        OSCR::aboutMenu();
        return;
      }

      MenuOption menuSelection = static_cast<MenuOption>(OSCR::UI::menu(OSCR::Lang::formatSubmenuTitle(OSCR::Strings::Features::Settings), menuOptions, kMenuOptionCount));

      switch (menuSelection)
      {
# if defined(ENABLE_RTC)
      case MenuOption::SetRTC:
        OSCR::Time::menu();
        continue;
# endif /* ENABLE_RTC */

# if defined(ENABLE_VSELECT)
      case MenuOption::Voltage:
        OSCR::Power::voltageLockMenu();
        continue;
# endif /* ENABLE_VSELECT */

# if defined(ENABLE_SELFTEST)
      case MenuOption::SelfTest:
        OSCR::Cores::SelfTest::run();
        continue;
# endif /* ENABLE_SELFTEST */

# if defined(OPTION_CLOCKGEN_CALIBRATION)
      case MenuOption::Calibration:
        OSCR::ClockGen::clkcal();
        continue;
# endif /* OPTION_CLOCKGEN_CALIBRATION */

      case MenuOption::About:
        OSCR::aboutMenu();
        continue;

      case MenuOption::Back:
        return;
      }
    }
    while (true);
  }
}
