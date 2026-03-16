/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#ifndef OSCR_LANG_FILE_TYPES_H_
# define OSCR_LANG_FILE_TYPES_H_

# include "ui/l10n.h"

namespace OSCR::Strings::FileType
{
  constexpr char const PROGMEM Save[]               = "sav";
  constexpr char const PROGMEM SaveRAM[]            = "srm";
  constexpr char const PROGMEM SaveRA[]             = "sra";
  constexpr char const PROGMEM SaveFlash[]          = "fla";
  constexpr char const PROGMEM SaveEEPROM[]         = "eep";
  constexpr char const PROGMEM SaveBackup[]         = "brm";
  constexpr char const PROGMEM Raw[]                = "bin";
  constexpr char const PROGMEM Map[]                = "map";
  constexpr char const PROGMEM CRCX[]               = "crc";
  constexpr char const PROGMEM U2[]                 = "u2";

  constexpr char const PROGMEM DefaultName[]        = "CART";

  // Because these are also used for the core subfolders, generic definitions go above.
  // Try to keep the below ones unique.

  constexpr char const PROGMEM GameBoy[]            = "gb";
  constexpr char const PROGMEM GBMemoryModule[]     = "gbm";
  constexpr char const PROGMEM GBSmartModule[]      = "gbs";
  constexpr char const PROGMEM GameBoyAdvance[]     = "gba";
  constexpr char const PROGMEM NES[]                = "nes";
  constexpr char const PROGMEM SNES[]               = "sfc";
  constexpr char const PROGMEM SNESD[]              = "snes";
  constexpr char const PROGMEM Satellaview[]        = "bs";
  constexpr char const PROGMEM SFM[]                = "sfm";
  constexpr char const PROGMEM SFM_NP[]             = "np"; // ??
  constexpr char const PROGMEM ST[]                 = "st";
  constexpr char const PROGMEM GPC[]                = "gpc";
  constexpr char const PROGMEM N64[]                = "z64";
  constexpr char const PROGMEM N64F[]               = "n64";
  constexpr char const PROGMEM MPK[]                = "mpk";
  constexpr char const PROGMEM MegaDrive[]          = "md";
  constexpr char const PROGMEM MegaDrive32X[]       = "32x";
  constexpr char const PROGMEM SMS[]                = "sms";
  constexpr char const PROGMEM GameGear[]           = "gg";
  constexpr char const PROGMEM SG1000[]             = "sg";
  constexpr char const PROGMEM PCEngine[]           = "pce";
  constexpr char const PROGMEM WonderSwan[]         = "ws";
  constexpr char const PROGMEM NeoGeoPocket[]       = "ngp";
  constexpr char const PROGMEM Intellivision[]      = "int";
  constexpr char const PROGMEM Colecovision[]       = "col";
  constexpr char const PROGMEM VirtualBoy[]         = "vb";
  constexpr char const PROGMEM Supervision[]        = "wsv";
  constexpr char const PROGMEM PocketChallengeW[]   = "pcw";
  constexpr char const PROGMEM Atari2600[]          = "a26";
  constexpr char const PROGMEM Atari5200[]          = "a52";
  constexpr char const PROGMEM Atari7800[]          = "a78";
  constexpr char const PROGMEM Odyssey2[]           = "ody2";
  constexpr char const PROGMEM Arcadia2001[]        = "arc";
  constexpr char const PROGMEM ChannelF[]           = "chf";
  constexpr char const PROGMEM SuperAcan[]          = "acan";
  constexpr char const PROGMEM MSX[]                = "msx";
  constexpr char const PROGMEM PokemonMini[]        = "";
  constexpr char const PROGMEM CasioLoopy[]         = "loopy";
  constexpr char const PROGMEM Commodore64[]        = "c64";
  constexpr char const PROGMEM AtariJaguar[]        = "j64";
  constexpr char const PROGMEM AtariLynx[]          = "lnx";
  constexpr char const PROGMEM Vectrex[]            = "vec";
  constexpr char const PROGMEM Atari8[]             = "at8bit";
  constexpr char const PROGMEM BallyAstrocade[]     = "bally";
  constexpr char const PROGMEM LittleJammer[]       = "lj";
  constexpr char const PROGMEM LittleJammerPro[]    = "ljpro";
  constexpr char const PROGMEM CasioPV1000[]        = "pv1k";
  constexpr char const PROGMEM VIC20[]              = "";
  constexpr char const PROGMEM Leapster[]           = "";
  constexpr char const PROGMEM RCAStudio2[]         = "";
  constexpr char const PROGMEM TI99[]               = "";
  constexpr char const PROGMEM TomyPyuuta[]         = "";
  constexpr char const PROGMEM TRS80[]              = "trs";
  constexpr char const PROGMEM VSmile[]             = "";
  constexpr char const PROGMEM CPS3[]               = "cps3";

  constexpr char const PROGMEM Flash[]              = "flash";

} /* namespace OSCR::Strings::FileType */

#endif /* OSCR_LANG_FILE_TYPES_H_ */
