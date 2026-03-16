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
}

#endif /* OSCR_LOGGER_H_ */
