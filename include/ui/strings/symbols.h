/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#ifndef OSCR_LANG_SYMBOLS_H_
# define OSCR_LANG_SYMBOLS_H_

# include "ui/l10n.h"

namespace OSCR::Strings::Symbol
{
  // Generic Symbols
  constexpr char const PROGMEM Empty[]              = "";
  constexpr char const PROGMEM Space[]              = " ";
  constexpr char const PROGMEM Comma[]              = ",";
  constexpr char const PROGMEM Plus[]               = "+";
  constexpr char const PROGMEM Minus[]              = "-";
  constexpr char const PROGMEM Percent[]            = "%";
  constexpr char const PROGMEM Asterisk[]           = "*";
  constexpr char const PROGMEM Slash[]              = "/";
  constexpr char const PROGMEM MenuSpaces[]         = "   ";
  constexpr char const PROGMEM MenuSelection[]      = " > ";
  constexpr char const PROGMEM Arrow[]              = " -> ";
  constexpr char const PROGMEM NotEqual[]           = " != ";
  constexpr char const PROGMEM Colon[]              = ":";
  constexpr char const PROGMEM LabelEnd[]           = ": ";
  constexpr char const PROGMEM FullStop[]           = ".";
  constexpr char const PROGMEM Ellipsis[]           = "...";
  constexpr char const PROGMEM NewLine[]            = "\r\n";
  constexpr char const PROGMEM X[]                  = "X";

  constexpr char const PROGMEM PaddedSlash[]        = " / ";
  constexpr char const PROGMEM PaddedX[]            = " X ";

  // Progress Bar
  constexpr char const PROGMEM ProgressBarOpen[]    = "[";
  constexpr char const PROGMEM ProgressBarClose[]   = "]";

#   if defined(ENABLE_SERIAL_ANSI)
  constexpr char const PROGMEM ProgressBarEmpty[]   = "░";
  constexpr char const PROGMEM ProgressBarFilled[]  = "▓";
  constexpr char const PROGMEM ProgressBarUnknown[] = "▓";
#   else /* !ENABLE_SERIAL_ANSI */
  constexpr char const PROGMEM ProgressBarEmpty[]   = "-";
  constexpr char const PROGMEM ProgressBarFilled[]  = "*";
  constexpr char const PROGMEM ProgressBarUnknown[] = "=";
#   endif /* ENABLE_SERIAL_ANSI */
} /* namespace OSCR::Strings::Symbol */

#endif /* OSCR_LANG_SYMBOLS_H_ */
