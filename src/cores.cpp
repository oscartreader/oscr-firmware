/**
 * @file
 * @brief Register cores and handle the main menu.
 *
 * Cores (and features) should be registered in this file. Currently,
 * the main menu is also handled in this file. This is to avoid
 * having to edit several files just to define a newly implemented
 * core or feature.
 *
 * While it is possible to combine all of this into a single file,
 * the amount of macro abuse required to do it in a way that works
 * within the Arduino IDE bordered on a war crime. This is something
 * that will just have to wait until the move away from the Arduino
 * IDE.
 *
 * @sa core-types.h
 */

#include "cores.h"
#include "ui.h"
#include "common/OSCR.h"
#include "apps/Settings.h"

namespace OSCR::Menus
{
  constexpr CoreDefinition const MainMenuOptions[] = {
# if defined(ENABLE_GBX)
    {
      .id = CoreID::GBX,
      .title = OSCR::Strings::Cores::GameBoy,
      .menu = &OSCR::Cores::GameBoyX::menu,
    },
# endif

# if defined(ENABLE_NES)
    {
      .id = CoreID::NES,
      .title = OSCR::Strings::Cores::NES,
      .menu = &OSCR::Cores::NES::menu,
    },
# endif

# if defined(ENABLE_SNES)
    {
      .id = CoreID::SNES,
      .title = OSCR::Strings::Cores::SNES,
      .menu = &OSCR::Cores::SNES::menu,
    },
# endif

# if defined(ENABLE_N64)
    {
      .id = CoreID::N64,
      .title = OSCR::Strings::Cores::N64,
      .menu = &OSCR::Cores::N64::menu,
    },
# endif

# if defined(ENABLE_MD)
    {
      .id = CoreID::MD,
      .title = OSCR::Strings::Cores::MegaDrive,
      .menu = &OSCR::Cores::MegaDrive::menu,
    },
# endif

# if defined(ENABLE_SMS)
    {
      .id = CoreID::SMS,
      .title = OSCR::Strings::Cores::SMSGGSG,
      .menu = &OSCR::Cores::SMS::menu,
    },
# endif

# if defined(ENABLE_PCE)
    {
      .id = CoreID::PCE,
      .title = OSCR::Strings::Cores::PCEngine,
      .menu = &OSCR::Cores::PCEngine::menu,
    },
# endif

# if defined(ENABLE_WS)
    {
      .id = CoreID::WS,
      .title = OSCR::Strings::Cores::WonderSwan,
      .menu = &OSCR::Cores::WonderSwan::menu,
    },
# endif

# if defined(ENABLE_NGP)
    {
      .id = CoreID::NGP,
      .title = OSCR::Strings::Cores::NeoGeoPocket,
      .menu = &OSCR::Cores::NeoGeoPocket::menu,
    },
# endif

# if defined(ENABLE_INTV)
    {
      .id = CoreID::INTV,
      .title = OSCR::Strings::Cores::Intellivision,
      .menu = &OSCR::Cores::Intellivision::menu,
    },
# endif

# if defined(ENABLE_COLV)
    {
      .id = CoreID::COLV,
      .title = OSCR::Strings::Cores::Colecovision,
      .menu = &OSCR::Cores::Colecovision::menu,
    },
# endif

# if defined(ENABLE_VBOY)
    {
      .id = CoreID::VBOY,
      .title = OSCR::Strings::Cores::VirtualBoy,
      .menu = &OSCR::Cores::VirtualBoy::menu,
    },
# endif

# if defined(ENABLE_SUPERVISION)
    {
      .id = CoreID::WSV,
      .title = OSCR::Strings::Cores::Supervision,
      .menu = &OSCR::Cores::Supervision::menu,
    },
# endif

# if defined(ENABLE_PCW)
    {
      .id = CoreID::PCW,
      .title = OSCR::Strings::Cores::PocketChallengeW,
      .menu = &OSCR::Cores::PocketChallengeW::menu,
    },
# endif

# if defined(ENABLE_2600)
    {
      .id = CoreID::A2600,
      .title = OSCR::Strings::Cores::Atari2600,
      .menu = &OSCR::Cores::Atari2600::menu,
    },
# endif

# if defined(ENABLE_ODY2)
    {
      .id = CoreID::ODY2,
      .title = OSCR::Strings::Cores::Odyssey2,
      .menu = &OSCR::Cores::Odyssey2::menu,
    },
# endif

# if defined(ENABLE_ARC)
    {
      .id = CoreID::ARC,
      .title = OSCR::Strings::Cores::Arcadia2001,
      .menu = &OSCR::Cores::Arcadia2001::menu,
    },
# endif

# if defined(ENABLE_FAIRCHILD)
    {
      .id = CoreID::FAIRCHILD,
      .title = OSCR::Strings::Cores::ChannelF,
      .menu = &OSCR::Cores::ChannelF::menu,
    },
# endif

# if defined(ENABLE_SUPERACAN)
    {
      .id = CoreID::SUPERACAN,
      .title = OSCR::Strings::Cores::SuperAcan,
      .menu = &OSCR::Cores::SuperAcan::menu,
    },
# endif

# if defined(ENABLE_MSX)
    {
      .id = CoreID::MSX,
      .title = OSCR::Strings::Cores::MSX,
      .menu = &OSCR::Cores::MSX::menu,
    },
# endif

# if defined(ENABLE_POKE)
    {
      .id = CoreID::POKE,
      .title = OSCR::Strings::Cores::PokemonMini,
      .menu = &OSCR::Cores::PokemonMini::menu,
    },
# endif

# if defined(ENABLE_LOOPY)
    {
      .id = CoreID::LOOPY,
      .title = OSCR::Strings::Cores::CasioLoopy,
      .menu = &OSCR::Cores::CasioLoopy::menu,
    },
# endif

# if defined(ENABLE_C64)
    {
      .id = CoreID::C64,
      .title = OSCR::Strings::Cores::Commodore64,
      .menu = &OSCR::Cores::Commodore64::menu,
    },
# endif

# if defined(ENABLE_5200)
    {
      .id = CoreID::A5200,
      .title = OSCR::Strings::Cores::Atari5200,
      .menu = &OSCR::Cores::Atari5200::menu,
    },
# endif

# if defined(ENABLE_7800)
    {
      .id = CoreID::A7800,
      .title = OSCR::Strings::Cores::Atari7800,
      .menu = &OSCR::Cores::Atari7800::menu,
    },
# endif

# if defined(ENABLE_JAGUAR)
    {
      .id = CoreID::JAGUAR,
      .title = OSCR::Strings::Cores::AtariJaguar,
      .menu = &OSCR::Cores::AtariJaguar::menu,
    },
# endif

# if defined(ENABLE_LYNX)
    {
      .id = CoreID::LYNX,
      .title = OSCR::Strings::Cores::AtariLynx,
      .menu = &OSCR::Cores::AtariLynx::menu,
    },
# endif

# if defined(ENABLE_VECTREX)
    {
      .id = CoreID::VECTREX,
      .title = OSCR::Strings::Cores::Vectrex,
      .menu = &OSCR::Cores::Vectrex::menu,
    },
# endif

# if defined(ENABLE_ATARI8)
    {
      .id = CoreID::ATARI8,
      .title = OSCR::Strings::Cores::Atari8,
      .menu = &OSCR::Cores::Atari8::menu,
    },
# endif

# if defined(ENABLE_BALLY)
    {
      .id = CoreID::BALLY,
      .title = OSCR::Strings::Cores::BallyAstrocade,
      .menu = &OSCR::Cores::BallyAstrocade::menu,
    },
# endif

# if defined(ENABLE_LJ)
    {
      .id = CoreID::LJ,
      .title = OSCR::Strings::Cores::LittleJammer,
      .menu = &OSCR::Cores::LittleJammer::menu,
    },
# endif

# if defined(ENABLE_LJPRO)
    {
      .id = CoreID::LJPRO,
      .title = OSCR::Strings::Cores::LittleJammerPro,
      .menu = &OSCR::Cores::LittleJammerPro::menu,
    },
# endif

# if defined(ENABLE_PV1000)
    {
      .id = CoreID::PV1000,
      .title = OSCR::Strings::Cores::CasioPV1000,
      .menu = &OSCR::Cores::CasioPV1000::menu,
    },
# endif

# if defined(ENABLE_VIC20)
    {
      .id = CoreID::VIC20,
      .title = OSCR::Strings::Cores::VIC20,
      .menu = &OSCR::Cores::VIC20::menu,
    },
# endif

# if defined(ENABLE_LEAP)
    {
      .id = CoreID::LEAP,
      .title = OSCR::Strings::Cores::Leapster,
      .menu = &OSCR::Cores::Leapster::menu,
    },
# endif

# if defined(ENABLE_RCA)
    {
      .id = CoreID::RCA,
      .title = OSCR::Strings::Cores::RCAStudio2,
      .menu = &OSCR::Cores::RCAStudio2::menu,
    },
# endif

# if defined(ENABLE_TI99)
    {
      .id = CoreID::TI99,
      .title = OSCR::Strings::Cores::TI99,
      .menu = &OSCR::Cores::TI99::menu,
    },
# endif

# if defined(ENABLE_PYUUTA)
    {
      .id = CoreID::PYUUTA,
      .title = OSCR::Strings::Cores::TomyPyuuta,
      .menu = &OSCR::Cores::TomyPyuuta::menu,
    },
# endif

# if defined(ENABLE_TRS80)
    {
      .id = CoreID::TRS80,
      .title = OSCR::Strings::Cores::TRS80,
      .menu = &OSCR::Cores::TRS80::menu,
    },
# endif

# if defined(ENABLE_VSMILE)
    {
      .id = CoreID::VSMILE,
      .title = OSCR::Strings::Cores::VSmile,
      .menu = &OSCR::Cores::VSmile::menu,
    },
# endif

# if defined(ENABLE_FLASH8)
    {
      .id = CoreID::FLASH8,
      .title = OSCR::Strings::Cores::Flashrom,
      .menu = &OSCR::Cores::Flash::menu,
    },
# endif

# if defined(ENABLE_CPS3)
    {
      .id = CoreID::CPS3,
      .title = OSCR::Strings::Cores::CPS3,
      .menu = &OSCR::Cores::CPS3::menu,
    },
# endif

    {
      .id = CoreID::NONE,
      .title = OSCR::Strings::Features::Settings,
      .menu = &OSCR::Settings::menu,
    },

    {
      .id = CoreID::NONE,
      .title = OSCR::Strings::Features::Reset,
      .menu = &OSCR::resetMenu,
    },
  };

  /**
   * Number of entries in the MainMenuOptions array.
   */
  constexpr uint8_t const kMainMenuOptionCount = sizeofarray(MainMenuOptions);

  /**
   * Number of non-core options (i.e. About, Reset, ...) in the MainMenuOptions array.
   */
  constexpr uint8_t const kMainMenuNonCoreCount = 2;

  /**
   * Number of core options in the MainMenuOptions array.
   */
  constexpr uint8_t const kMainMenuCoreCount = kMainMenuOptionCount - kMainMenuNonCoreCount;

  /**
   * Require at least 1 core to be enabled.
   */
  static_assert(kMainMenuCoreCount > 0);

  void main()
  {
    if __if_constexpr (kMainMenuCoreCount == 1) // If only 1 core is enabled.
    {
      MainMenuOptions[0].menu();
      return;
    }

    __FlashStringHelper const * mainMenuOptions[kMainMenuOptionCount] = {};

    for (uint8_t i = 0; i < kMainMenuOptionCount; i++)
    {
      mainMenuOptions[i] = FS(MainMenuOptions[i].title);
    }

    uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::Headings::OSCR), mainMenuOptions, kMainMenuOptionCount);

    MainMenuOptions[menuSelection].menu();
  }
}
