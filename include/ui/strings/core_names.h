/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#ifndef OSCR_LANG_CORE_NAMES_H_
# define OSCR_LANG_CORE_NAMES_H_

# include "ui/l10n.h"

namespace OSCR::Strings::Cores
{
  constexpr char const PROGMEM GameBoy[]            = "Game Boy (Color)";
  constexpr char const PROGMEM GBMemoryModule[]     = "GB Memory Module";
  constexpr char const PROGMEM GBSmartModule[]      = "Game Boy Smart Module";

# if OPTION_VOLTAGE_SPECIFIER & 2
  constexpr char const PROGMEM GameBoyAdvance[]     = "Game Boy Advance (3V)";
# else
  constexpr char const PROGMEM GameBoyAdvance[]     = "Game Boy Advance";
# endif

# if ((OSCR_REGION == REGN_AS) || ((OSCR_REGION == REGN_AUTO) && (OSCR_LANGUAGE == LANG_JA)))
  constexpr char const PROGMEM NES[]                = "Famicom";
  constexpr char const PROGMEM SNES[]               = "Super Famicom";
# else
  constexpr char const PROGMEM NES[]                = "NES";
  constexpr char const PROGMEM SNES[]               = "SNES";
# endif

  constexpr char const PROGMEM Satellaview[]        = "Satellaview";
  constexpr char const PROGMEM SFM[]                = "SF Memory Cassette";
  constexpr char const PROGMEM ST[]                 = "Sufami Turbo";
  constexpr char const PROGMEM GPC[]                = "Game Processor RAM";

# if OPTION_VOLTAGE_SPECIFIER & 2
  constexpr char const PROGMEM N64[]                = "Nintendo 64 (3V)";
# else
  constexpr char const PROGMEM N64[]                = "Nintendo 64";
# endif

# if (OSCR_REGION == REGN_NA) // NA calls it the Sega Genesis
  constexpr char const PROGMEM MegaDrive[]          = "Genesis";
# elif ((OSCR_REGION != REGN_AUTO) || ((OSCR_REGION == REGN_AUTO) && (OSCR_LANGUAGE != LANG_EN))) // Everywhere else calls it the Sega Mega Drive
  constexpr char const PROGMEM MegaDrive[]          = "Mega Drive";
# else
  constexpr char const PROGMEM MegaDrive[]          = "Mega Drive/Genesis";
# endif

  constexpr char const PROGMEM SMSGGSG[]            = "SMS/GG/MIII/SG-1000";
  constexpr char const PROGMEM SMS[]                = "Sega Master System";
  constexpr char const PROGMEM GameGear[]           = "Game Gear";
  constexpr char const PROGMEM SG1000[]             = "SG-1000";

# if (OSCR_REGION == REGN_NA) // NA calls it TurboGrafx-16
  constexpr char const PROGMEM PCEngine[]           = "TurboGrafx-16";
# elif ((OSCR_REGION != REGN_AUTO) || ((OSCR_REGION == REGN_AUTO) && (OSCR_LANGUAGE != LANG_EN))) // Everywhere else calls it PC Engine
  constexpr char const PROGMEM PCEngine[]           = "PC Engine";
# else
  constexpr char const PROGMEM PCEngine[]           = "PC Engine/TG16";
# endif

# if (OPTION_VOLTAGE_SPECIFIER & 2)
  constexpr char const PROGMEM WonderSwan[]         = "WonderSwan (3V)";
# else
  constexpr char const PROGMEM WonderSwan[]         = "WonderSwan";
# endif

# if (OPTION_VOLTAGE_SPECIFIER & 2)
  constexpr char const PROGMEM NeoGeoPocket[]       = "NeoGeo Pocket (3V)";
# else
  constexpr char const PROGMEM NeoGeoPocket[]       = "NeoGeo Pocket";
# endif

  constexpr char const PROGMEM Intellivision[]      = "Intellivision";

  constexpr char const PROGMEM Colecovision[]       = "Colecovision";

  constexpr char const PROGMEM VirtualBoy[]         = "Virtual Boy";

# if (OPTION_VOLTAGE_SPECIFIER & 2)
  constexpr char const PROGMEM Supervision[]        = "Watara Supervision (3V)";
# else
  constexpr char const PROGMEM Supervision[]        = "Watara Supervision";
# endif

  constexpr char const PROGMEM PocketChallengeW[]   = "Pocket Challenge W";

  constexpr char const PROGMEM Atari2600[]          = "Atari 2600";
  constexpr char const PROGMEM Atari5200[]          = "Atari 5200";
  constexpr char const PROGMEM Atari7800[]          = "Atari 7800";

  constexpr char const PROGMEM Odyssey2[]           = "Magnavox Odyssey 2";

  constexpr char const PROGMEM Arcadia2001[]        = "Arcadia 2001";

  constexpr char const PROGMEM ChannelF[]           = "Fairchild Channel F";

  constexpr char const PROGMEM SuperAcan[]          = "Super A'can";

  constexpr char const PROGMEM MSX[]                = "MSX";

# if (OPTION_VOLTAGE_SPECIFIER & 2)
  constexpr char const PROGMEM PokemonMini[]        = "Pokemon Mini (3V)";
# else
  constexpr char const PROGMEM PokemonMini[]        = "Pokemon Mini";
# endif

  constexpr char const PROGMEM CasioLoopy[]         = "Casio Loopy";

  constexpr char const PROGMEM Commodore64[]        = "Commodore 64";

  constexpr char const PROGMEM AtariJaguar[]        = "Atari Jaguar";

  constexpr char const PROGMEM AtariLynx[]          = "Atari Lynx";

  constexpr char const PROGMEM Vectrex[]            = "Vectrex";

  constexpr char const PROGMEM Atari8[]             = "Atari 8-bit";

  constexpr char const PROGMEM BallyAstrocade[]     = "Bally Astrocade";

  constexpr char const PROGMEM LittleJammer[]       = "Bandai LJ";

  constexpr char const PROGMEM LittleJammerPro[]    = "Bandai LJ Pro";

  constexpr char const PROGMEM CasioPV1000[]        = "Casio PV-1000";

  constexpr char const PROGMEM VIC20[]              = "Commodore VIC-20";

# if (OPTION_VOLTAGE_SPECIFIER & 2)
  constexpr char const PROGMEM Leapster[]           = "LF Leapster (3V)";
# else
  constexpr char const PROGMEM Leapster[]           = "LF Leapster";
# endif

  constexpr char const PROGMEM RCAStudio2[]         = "RCA Studio II";

  constexpr char const PROGMEM TI99[]               = "TI-99";

  constexpr char const PROGMEM TomyPyuuta[]         = "Tomy Pyuuta";

  constexpr char const PROGMEM TRS80[]              = "TRS-80";

# if (OPTION_VOLTAGE_SPECIFIER & 2)
  constexpr char const PROGMEM VSmile[]             = "Vtech V.Smile (3V)";
# else
  constexpr char const PROGMEM VSmile[]             = "Vtech V.Smile";
# endif

  constexpr char const PROGMEM Flashrom[]           = "Flashrom Programmer";

  constexpr char const PROGMEM CPS3[]               = "CP System III";

# if (OPTION_VOLTAGE_SPECIFIER & 2)
  constexpr char const PROGMEM SelfTest[]           = "Self Test (3V)";
# else
  constexpr char const PROGMEM SelfTest[]           = "Self Test";
# endif

} /* namespace OSCR::Strings::Cores */

#endif /* OSCR_LANG_CORE_NAMES_H_ */
