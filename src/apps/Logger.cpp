/**
 * @file
 * @brief OSCR Logger application
 */
#include "apps/Logger.h"
#include "common/Configuration.h"
#include "hardware/outputs/Serial.h"

namespace OSCR
{
  constexpr char const PROGMEM LogFile[] = "/.oscr/logs/";

  namespace Logger
  {
    constexpr uint8_t const kTempBufferSize = 50;

    OSCR::Storage::File file;

    bool enabled = true;
    bool debugLog = false;

    void setup()
    {
      if (!OSCR::Storage::exists(FS(LogFile)))
      {
        if (!OSCR::Storage::createFolder(FS(LogFile)))
        {
          OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::OSCR));
          OSCR::UI::fatalError(F("Log creation failed!"));
        }
      }

      file.open(F("/.oscr/logs/current.log"), O_RDWR | O_CREAT | O_APPEND);

      if (file.fileSize() == 0)
      {
        logLine(FS(OSCR::Strings::Headings::OSCR));
        log(F("Firmware "));
        logLine(FS(OSCR::Strings::Version));
        logLine();
        logLine();
      }

#if defined(ENABLE_CONFIG)
      OSCR::Configuration::getBool(F("oscr.logging"), enabled);
      OSCR::Configuration::getBool(F("oscr.debug"), debugLog);
#endif /* ENABLE_CONFIG */
    }

    void writeLog(char const * const message)
    {
      file.write(message);
    }

    void writeLog(__FlashStringHelper const * const message)
    {
      char tmpMessage[Logger::kTempBufferSize];
      OSCR::Util::copyStr_P(BUFFN(tmpMessage), message);
      file.write(tmpMessage);
    }

    bool isEnabled()
    {
      return enabled;
    }
  }

  void log(char const * const message)
  {
    if (!Logger::isEnabled()) return;
    Logger::writeLog(message);
  }

  void log(__FlashStringHelper const * const message)
  {
    if (!Logger::isEnabled()) return;
    Logger::writeLog(message);
  }

  void logLine(void)
  {
    log(FS(OSCR::Strings::Symbol::NewLine));
  }

  void logLine(char const * const message)
  {
    log(message);
    logLine();
  }

  void logLine(__FlashStringHelper const * const message)
  {
    log(message);
    logLine();
  }

  void logLabel(char const * const label, char const * const message)
  {
    log(label);
    log(message);
    logLine();
  }

  void logLabel(__FlashStringHelper const * const label, char const * const message)
  {
    log(label);
    log(message);
    logLine();
  }

  void logLabel(__FlashStringHelper const * const label, __FlashStringHelper const * const message)
  {
    log(label);
    log(message);
    logLine();
  }

#if (SERIAL_DEBUGGING)
  void debugLine(char const * const fileName, uint16_t lineNumber)
  {
    OSCR::Serial::print(FS(fileName));
    OSCR::Serial::print(FS(OSCR::Strings::Symbol::Colon));
    OSCR::Serial::printLineSync(lineNumber);
  }

  void debugLine(char const * const fileName, uint16_t lineNumber, char const * const message)
  {
    OSCR::Serial::print(FS(fileName));
    OSCR::Serial::print(FS(OSCR::Strings::Symbol::Colon));
    OSCR::Serial::print(lineNumber);
    OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
    OSCR::Serial::printLineSync(FS(message));
  }

  void debugLine(char const * const fileName, uint16_t lineNumber, __FlashStringHelper const * const message)
  {
    OSCR::Serial::print(FS(fileName));
    OSCR::Serial::print(FS(OSCR::Strings::Symbol::Colon));
    OSCR::Serial::print(lineNumber);
    OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
    OSCR::Serial::printLineSync(message);
  }
#endif
}
