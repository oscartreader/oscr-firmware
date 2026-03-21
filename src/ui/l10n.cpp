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

    char const * formatLabel(char const * label)
    {
      snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::LabelP, label);

      return bufferStr;
    }


    char const * formatLabel(char const * label, char const * value)
    {
      snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::LabelPValue, label, value);

      return bufferStr;
    }

    char const * formatLabel_P(char const * label, char const * value)
    {
      snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::LabelPValueP, label, value);

      return bufferStr;
    }

    char const * formatType(char const * label)
    {
      snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::TypePValue, label, OSCR::Strings::Symbol::Empty);

      return bufferStr;
    }

    char const * formatType(char const * label, char const * value)
    {
      snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::TypePValue, label, value);

      return bufferStr;
    }

    char const * formatType_P(char const * label, char const * value)
    {
      snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::TypePValueP, label, value);

      return bufferStr;
    }

    // flash pointer + SRAM number
    template <typename T,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable>
    char const * formatType(char const * label, T number)
    {
      if __if_constexpr (OSCR::Util::is_unsigned_v<T>)
      {
        snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::TypePValueU32, label, (uint32_t)number);
      }
      else
      {
        snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::TypePValueD32, label, (int32_t)number);
      }

      return bufferStr;
    }

    char const * formatSize_P(char const * label, char const * value)
    {
      snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::SizePValueP, label, value);

      return bufferStr;
    }

    // flash pointer + SRAM number
    template <typename T,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable>
    char const * formatLabel(char const * label, T number)
    {
      if __if_constexpr (OSCR::Util::is_unsigned_v<T>)
      {
        snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::LabelPValueU32, label, (uint32_t)number);
      }
      else
      {
        snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::LabelPValueD32, label, (int32_t)number);
      }

      return bufferStr;
    }

    // flash pointer + SRAM number + SRAM number
    template <typename T,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable>
    char const * formatLabel(char const * label, T number, T total)
    {
      if __if_constexpr (OSCR::Util::is_unsigned_v<T>)
      {
        snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::LabelPValuesU32, label, (uint32_t)number, (uint32_t)total);
      }
      else
      {
        snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::LabelPValuesD32, label, (int32_t)number, (int32_t)total);
      }

      return bufferStr;
    }

    char const * formatLabelSize(char const * label, uint32_t size, bool isBytes)
    {
      uint8_t const offset = (label == nullptr) ? 3 : 0;
      DataSize myLabel = {
        size: size,
        isBytes: isBytes,
      };

      updateDataSize(myLabel);

      snprintf_P(BUFFN(bufferStr), OSCR::Strings::Templates::SizeLabelFormat + offset, label, myLabel.formatSize, myLabel.formatUnit);

      return bufferStr;
    }

    //__AVR__NEVER_INLINE__
    void updateDataSize(DataSize & dataSize)
    {
      if ((dataSize.size >= sizeGiga) && ((dataSize.size % sizeGiga) == 0)) // G (lol)
      {
        dataSize.unit = DataSizeUnit::Giga;
        dataSize.format = ((dataSize.isBytes) ? OSCR::Strings::Templates::SizeGB : OSCR::Strings::Templates::SizeG);
        dataSize.formatSize = dataSize.size / sizeGiga;
        dataSize.formatUnit = ((dataSize.isBytes) ? OSCR::Strings::Units::GB : OSCR::Strings::Units::G);
      }
      else if ((dataSize.size >= sizeMega) && ((dataSize.size % sizeMega) == 0)) // M
      {
        dataSize.unit = DataSizeUnit::Mega;
        dataSize.format = ((dataSize.isBytes) ? OSCR::Strings::Templates::SizeMB : OSCR::Strings::Templates::SizeM);
        dataSize.formatSize = dataSize.size / sizeMega;
        dataSize.formatUnit = ((dataSize.isBytes) ? OSCR::Strings::Units::MB : OSCR::Strings::Units::M);
      }
      else if ((dataSize.size >= sizeKilo) && ((dataSize.size % sizeKilo) == 0)) // K
      {
        dataSize.unit = DataSizeUnit::Kilo;
        dataSize.format = ((dataSize.isBytes) ? OSCR::Strings::Templates::SizeKB : OSCR::Strings::Templates::SizeK);
        dataSize.formatSize = dataSize.size / sizeKilo;
        dataSize.formatUnit = ((dataSize.isBytes) ? OSCR::Strings::Units::KB : OSCR::Strings::Units::K);
      }
      else
      {
        dataSize.unit = DataSizeUnit::Base;
        dataSize.format = ((dataSize.isBytes) ? OSCR::Strings::Templates::SizeBytes : OSCR::Strings::Templates::SizeBits);
        dataSize.formatSize = dataSize.size;
        dataSize.formatUnit = ((dataSize.isBytes) ? OSCR::Strings::Units::B : OSCR::Strings::Units::b);
      }
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
      DataSize dataSize = {
        size: bitsBytes,
        isBytes: isBytes,
      };

      updateDataSize(dataSize);

      applyTemplate_P(bufferStr, flashBufferStrSize, dataSize.format, dataSize.formatSize);

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

template char const * OSCR::Lang::formatLabel(char const * label, int8_t number);
template char const * OSCR::Lang::formatLabel(char const * label, uint8_t number);
template char const * OSCR::Lang::formatLabel(char const * label, int16_t number);
template char const * OSCR::Lang::formatLabel(char const * label, uint16_t number);
template char const * OSCR::Lang::formatLabel(char const * label, int32_t number);
template char const * OSCR::Lang::formatLabel(char const * label, uint32_t number);

template char const * OSCR::Lang::formatLabel(char const * label, int8_t number, int8_t total);
template char const * OSCR::Lang::formatLabel(char const * label, uint8_t number, uint8_t total);
template char const * OSCR::Lang::formatLabel(char const * label, int16_t number, int16_t total);
template char const * OSCR::Lang::formatLabel(char const * label, uint16_t number, uint16_t total);
template char const * OSCR::Lang::formatLabel(char const * label, int32_t number, int32_t total);
template char const * OSCR::Lang::formatLabel(char const * label, uint32_t number, uint32_t total);

template char const * OSCR::Lang::formatType(char const * label, int8_t number);
template char const * OSCR::Lang::formatType(char const * label, uint8_t number);
template char const * OSCR::Lang::formatType(char const * label, int16_t number);
template char const * OSCR::Lang::formatType(char const * label, uint16_t number);
template char const * OSCR::Lang::formatType(char const * label, int32_t number);
template char const * OSCR::Lang::formatType(char const * label, uint32_t number);
