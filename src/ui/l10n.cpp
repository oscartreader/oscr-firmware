#include "common.h"
#include "ui.h"

#if OSCR_LANGUAGE == LANG_EN
# include "ui/l10n/en.h"
#endif

#include "ui/strings/core_names.h"
#include "ui/strings/core_file_types.h"
#include "ui/strings/symbols.h"
#include "ui/strings/units.h"
#include "ui/strings/power.h"

namespace
{
  using OSCR::Util::flashBufferStrSize,
        OSCR::Util::applyTemplate_P;

  char bufferStr[flashBufferStrSize] = {};

  constexpr uint32_t const sizeKilo = 1024;
  constexpr uint32_t const sizeMega = sizeKilo * 1024;
  constexpr uint32_t const sizeGiga = sizeMega * 1024;
}

namespace OSCR
{
  namespace Lang
  {
#if defined(ENABLE_UPDATER)
    namespace Updater
    {
      constexpr char const PROGMEM VersionInfo[]              = "%S::%S//HW%" PRIu8;
      constexpr char const PROGMEM HardwareFlagValueDec[]     = ";%S=%" PRIu8;
      constexpr char const PROGMEM HardwareFlagValueHex[]     = ";%S=0x%X";
      constexpr char const PROGMEM HardwareFlagValueString[]  = ";%S=%S";
      constexpr char const PROGMEM HardwareFlagValueStringR[] = ";%S=%s";
      constexpr char const PROGMEM FirmwareFlag[]             = "|%S";
      constexpr char const PROGMEM FirmwareFlagWithValue[]    = ";%S=";

      void oscrInfo()
      {
        snprintf_P(BUFFN(bufferStr), VersionInfo, OSCR::Strings::Common::OSCR, OSCR::Strings::Version, (uint8_t)HW_VERSION);
        OSCR::Serial::print(bufferStr);
      }

      void flagValue(char const flag[], uint8_t value)
      {
        snprintf_P(BUFFN(bufferStr), HardwareFlagValueDec, flag, value);
        OSCR::Serial::print(bufferStr);
      }

      void flagValue(char const flag[], uint16_t value)
      {
        snprintf_P(BUFFN(bufferStr), HardwareFlagValueHex, flag, value);
        OSCR::Serial::print(bufferStr);
      }

      void flagValue(char const flag[], uint32_t value)
      {
        snprintf_P(BUFFN(bufferStr), HardwareFlagValueHex, flag, value);
        OSCR::Serial::print(bufferStr);
      }

      void flagValue(char const flag[], uint64_t value)
      {
        constexpr size_t const hexStringLen = 19;
        char hexString[hexStringLen] = {};
        OSCR::Util::toHex(hexString, hexStringLen, value);

        snprintf_P(BUFFN(bufferStr), HardwareFlagValueStringR, flag, hexString);

        OSCR::Serial::print(bufferStr);
      }

      void flagValue(char const flag[], char const value[])
      {
        snprintf_P(BUFFN(bufferStr), HardwareFlagValueString, flag, value);
        OSCR::Serial::print(bufferStr);
      }

      void flagValue(char const flag[])
      {
        snprintf_P(BUFFN(bufferStr), FirmwareFlag, flag);
        OSCR::Serial::print(bufferStr);
      }

      void flagValueV(char const flag[], uint8_t values[], size_t valueCount)
      {
        snprintf_P(BUFFN(bufferStr), FirmwareFlagWithValue, flag);
        OSCR::Serial::print(bufferStr);

        for (uint8_t i = 0; i < valueCount; i++)
        {
          if (i)
          {
            OSCR::Serial::print(FS(OSCR::Strings::Symbol::Comma));
          }

          OSCR::Serial::print(values[i]);
        }
      }

      void flagValueV(char const flag[], uint32_t values[], size_t valueCount)
      {
        snprintf_P(BUFFN(bufferStr), FirmwareFlagWithValue, flag);
        OSCR::Serial::print(bufferStr);

        for (uint8_t i = 0; i < valueCount; i++)
        {
          if (i)
          {
            OSCR::Serial::print(FS(OSCR::Strings::Symbol::Comma));
          }

          OSCR::Serial::printHex(values[i]);
        }
      }
    } /* namespace OSCR::Lang::Updater */

#endif /* ENABLE_UPDATER */

    void menuHeader(char buffer[], size_t size, char const * flashStr)
    {
      applyTemplate_P(buffer, size, OSCR::Strings::Templates::OSCRHeaderPrefix, flashStr);
    }

    char * formatSubmenuTitle(char const * flashStr)
    {
      applyTemplate_P(bufferStr, flashBufferStrSize, OSCR::Strings::Templates::OSCRHeaderPrefix, flashStr);

      return bufferStr;
    }

    void printUpper(char const * str)
    {
      OSCR::Util::copyStrUpr(bufferStr, sizeof(bufferStr), str);

      OSCR::UI::print(bufferStr);
    }

    void printUpperLine(char const * str)
    {
      printUpper(str);
      OSCR::UI::printLine();
    }

    // Flash strings

    void printUpper_P(char const * flashStr)
    {
      OSCR::Util::copyStrUpr_P(bufferStr, sizeof(bufferStr), flashStr);

      OSCR::UI::print(bufferStr);
    }

    void printUpper_P(__FlashStringHelper const * flashStr)
    {
      printUpper_P(reinterpret_cast<char const *>(flashStr));
    }

    void printUpper(__FlashStringHelper const * flashStr)
    {
      printUpper_P(reinterpret_cast<char const *>(flashStr));
    }

    void printUpperLine_P(char const * flashStr)
    {
      printUpper_P(flashStr);
      OSCR::UI::printLine();
    }

    void printUpperLine_P(__FlashStringHelper const * flashStr)
    {
      printUpperLine_P(reinterpret_cast<char const *>(flashStr));
    }

    void printUpperLine(__FlashStringHelper const * flashStr)
    {
      printUpperLine_P(reinterpret_cast<char const *>(flashStr));
    }

    void printSwitchVoltage(Voltage voltage)
    {
      applyTemplate_P(bufferStr, flashBufferStrSize, OSCR::Strings::Templates::VoltageSwitchTo, ((voltage == Voltage::k3V3) ? OSCR::Strings::Power::Voltage3V3 : OSCR::Strings::Power::Voltage5));

      OSCR::UI::printLine(bufferStr);
    }

    void __printBitsBytes(uint32_t bitsBytes, bool isBytes)
    {
      char const * flashTemplate;

      if ((bitsBytes >= sizeGiga) && ((bitsBytes % sizeGiga) == 0)) // G (lol)
      {
        bitsBytes = bitsBytes / sizeGiga;
        flashTemplate = ((isBytes) ? OSCR::Strings::Templates::SizeGB : OSCR::Strings::Templates::SizeG);
      }
      else if ((bitsBytes >= sizeMega) && ((bitsBytes % sizeMega) == 0)) // M
      {
        bitsBytes = bitsBytes / sizeMega;
        flashTemplate = ((isBytes) ? OSCR::Strings::Templates::SizeMB : OSCR::Strings::Templates::SizeM);
      }
      else if ((bitsBytes >= sizeKilo) && ((bitsBytes % sizeKilo) == 0)) // K
      {
        bitsBytes = bitsBytes / sizeKilo;
        flashTemplate = ((isBytes) ? OSCR::Strings::Templates::SizeKB : OSCR::Strings::Templates::SizeK);
      }
      else
      {
        flashTemplate = ((isBytes) ? OSCR::Strings::Templates::SizeBytes : OSCR::Strings::Templates::SizeBits);
      }

      applyTemplate_P(bufferStr, flashBufferStrSize, flashTemplate, bitsBytes);

      OSCR::UI::print(bufferStr);
    }

    void printBits(uint32_t bits)
    {
      __printBitsBytes(bits, false);
    }

    void printBitsLine(uint32_t bits)
    {
      printBits(bits);
      OSCR::UI::printLine();
    }

    void printBytes(uint32_t bytes)
    {
      __printBitsBytes(bytes, true);
    }

    void printBytesLine(uint32_t bytes)
    {
      printBytes(bytes);
      OSCR::UI::printLine();
    }

    void printErrorVerifyBytes(uint32_t byteCount)
    {
      applyTemplate_P(bufferStr, flashBufferStrSize, OSCR::Strings::Templates::ErrorVerifyBytes, byteCount);

      OSCR::UI::error(bufferStr);
    }
  }
}
