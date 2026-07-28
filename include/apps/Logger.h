/**
 * @file
 * @brief OSCR Logger application
 */
#pragma once
#ifndef OSCR_LOGGER_H_
# define OSCR_LOGGER_H_

#include "common/OSCR.h"
#include "common/Util.h"
#include "ui.h"

namespace OSCR
{
  namespace Logger
  {
    extern OSCR::Storage::File file;
    extern void setup();
    extern bool isEnabled();
  }

  extern void log(char const * const message);
  extern void log(__FlashStringHelper const * const message);

  extern void logLine(void);
  extern void logLine(char const * const message);
  extern void logLine(__FlashStringHelper const * const message);

  extern void logLabel(char const * const label, char const * const message);
  extern void logLabel(__FlashStringHelper const * const label, char const * const message);
  extern void logLabel(__FlashStringHelper const * const label, __FlashStringHelper const * const message);

  extern void debugPrintLine(void);

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
  extern void debugPrint(T string);

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
  extern void debugPrintLine(T string);

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
  extern void debugPrint(Tint number, int base = DEC);

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
  extern void debugPrintLine(Tint number, int base = DEC);

#if (HAS_SERIAL_ASCII_OUT) && (SERIAL_DEBUGGING)
  extern void debugLine(char const * const fileName, uint16_t lineNumber);
  extern void debugLine(char const * const fileName, uint16_t lineNumber, char const * const message);
  extern void debugLine(char const * const fileName, uint16_t lineNumber, __FlashStringHelper const * const message);
#else
  inline void debugLine(char const * const fileName __attribute__((unused)), uint16_t lineNumber __attribute__((unused))){}
  inline void debugLine(char const * const fileName __attribute__((unused)), uint16_t lineNumber __attribute__((unused)), char const * const message __attribute__((unused))){}
  inline void debugLine(char const * const fileName __attribute__((unused)), uint16_t lineNumber __attribute__((unused)), __FlashStringHelper const * const message __attribute__((unused))){}
#endif
}

#endif /* OSCR_LOGGER_H_ */
