#include "common/OSCR.h"
#include "common/Util.h"
#include <errno.h>

namespace
{
  using OSCR::Util::flashBufferStrSize;

  char bufferStr[flashBufferStrSize];

  constexpr uint8_t const kAsciiMinus       = '-';

  constexpr uint8_t const kAsciiZero        = '0';
  constexpr uint8_t const kAsciiNine        = '9';

  constexpr uint8_t const kAsciiThousands   = ',';
  constexpr uint8_t const kAsciiSpace       = ' ';

  constexpr uint8_t const kAsciiX           = 'x';
  constexpr uint8_t const kAsciiB           = 'b';
  constexpr uint8_t const kAsciiA           = 'A';
  constexpr uint8_t const kAsciiF           = 'F';
  constexpr uint8_t const kAsciiLowerA      = 'a';
  constexpr uint8_t const kAsciiCaseOffset  = kAsciiLowerA - kAsciiA;

  constexpr char const PROGMEM TemplateHex[]          = "0x%02X";

  static_assert(flashBufferStrSize > 1);

  static constexpr uint32_t const PROGMEM fastPow10[] = {
     1,
     10,
     100,
     1000,
     10000,
     100000,
     1000000,
     10000000,
     100000000,
     1000000000,
  };
}

namespace OSCR
{
  namespace Util
  {
    template <typename T>
    void swap(T &a, T &b)
    {
        T c = a;
        a = b;
        b = c;
    }

    template <typename T,
              if_is_integer_t<T> Enable>
    T power(T b, T n)
    {
      T r = 1;
      while (true)
      {
        if (!n--) return r;
        r *= b;
      }
    }

    template <__TEMPLATE_AUTO_UINT8 base, typename T,
              if_is_integer_t<T> Enable,
              enable_if_t<2 == base, bool> IsEnabled>
    T power(T n)
    {
      return (1 << n); // Bit shift for powers of 2.
    }

    template <__TEMPLATE_AUTO_UINT8 base, typename T,
              if_is_integer_t<T> Enable,
              enable_if_t<8 == base, bool> IsEnabled>
    T power(T n)
    {
      return (1 << (3 * n)); // Bit shift for powers of 8.
    }

    template <__TEMPLATE_AUTO_UINT8 base, typename T,
              if_is_integer_t<T> Enable,
              enable_if_t<10 == base, bool> IsEnabled>
    T power(T n)
    {
      return pgm_read_dword(fastPow10 + n); // LUT for powers of 10
    }

    template <__TEMPLATE_AUTO_UINT8 base, typename T,
              if_is_integer_t<T> Enable,
              enable_if_t<16 == base, bool> IsEnabled>
    T power(T n)
    {
      return (1 << (4 * n)); // Bit shift for powers of 16.
    }

    uint8_t __getBase(char const buffer[], uint8_t & offset)
    {
      if (buffer[offset] == kAsciiZero)
      {
        if (buffer[offset + 1] == kAsciiX)
        {
          offset += 2;
          return HEX;
        }

        if (buffer[offset + 1] == kAsciiB)
        {
          offset += 2;
          return BIN;
        }

        offset += 1;
        return OCT;
      }

      return DEC;
    }

    uint8_t __chrToInt(char chr)
    {
      if ((chr >= kAsciiZero) && (chr <= kAsciiNine)) return (chr - kAsciiZero);

      if (chr >= kAsciiLowerA) chr -= kAsciiCaseOffset; // lower case -> upper case

      if ((chr >= kAsciiA) && (chr <= kAsciiF)) return (chr - kAsciiA) + 10; // Hex

      return UINT8_MAX; // ??
    }

    bool strToInt(char const buffer[], int32_t & dest)
    {
      size_t const len = strlen(buffer);

      if (len == 0) return false;

      bool const negative = (buffer[0] == kAsciiMinus);
      uint8_t offset = negative;
      uint8_t base = __getBase(buffer, offset);
      uint8_t digit = 0;
      uint32_t value = 0;

      for (size_t i = len; i > offset; i--)
      {
        size_t idx = i - 1;

        if ((buffer[idx] == kAsciiThousands) || (buffer[idx] == kAsciiSpace)) continue;

        uint8_t num = __chrToInt(buffer[idx]);

        if ((num >= base) || (digit > 10)) return false; // invalid

        // If the number is zero, there is nothing to add
        if (num == 0)
        {
          digit++;
          continue;
        }

        uint32_t multiplier;

        switch (base)
        {
        case BIN: multiplier = OSCR::Util::power<BIN>(digit++); break;
        case OCT: multiplier = OSCR::Util::power<OCT>(digit++); break;
        case DEC: multiplier = OSCR::Util::power<DEC>(digit++); break;
        case HEX: multiplier = OSCR::Util::power<HEX>(digit++); break;
        default:  multiplier = OSCR::Util::power(base, digit++); break;
        }

        uint32_t addition = num * multiplier;

        // Check if our new value overflows
        // - If we are trying to add less than we multiplied by, there was an overflow
        // - If we subtract `addition` from `INT32_MAX` and `value` is higher than that, adding will overflow.
        if ((addition < multiplier) || (value > (INT32_MAX - addition))) return false;

        value += addition;
      }

      // Final overflow check
      if (value > INT32_MAX) return false;

      dest = negative ? -value : value;

      return true;
    }

    bool strToInt(char const buffer[], uint32_t & dest)
    {
      size_t const len = strlen(buffer);

      if (len == 0) return false;

      uint8_t offset = 0;
      uint8_t base = __getBase(buffer, offset);
      uint8_t digit = 0;
      uint32_t value = 0;

      for (size_t i = len; i > offset; i--)
      {
        size_t idx = i - 1;

        if ((buffer[idx] == kAsciiThousands) || (buffer[idx] == kAsciiSpace)) continue;

        uint8_t num = __chrToInt(buffer[idx]);

        if ((num >= base) || (digit > 10)) return false; // invalid

        // If the number is zero, there is nothing to add
        if (num == 0)
        {
          digit++;
          continue;
        }

        uint32_t multiplier;

        switch (base)
        {
        case BIN: multiplier = OSCR::Util::power<BIN>(digit++); break;
        case OCT: multiplier = OSCR::Util::power<OCT>(digit++); break;
        case DEC: multiplier = OSCR::Util::power<DEC>(digit++); break;
        case HEX: multiplier = OSCR::Util::power<HEX>(digit++); break;
        default:  multiplier = OSCR::Util::power(base, digit++); break;
        }

        uint32_t addition = num * multiplier;

        // Check if our new value overflows
        // - If we are trying to add less than we multiplied by, there was an overflow
        // - If we subtract `addition` from `UINT32_MAX` and `value` is higher than that, adding will overflow.
        if ((addition < multiplier) || (value > (UINT32_MAX - addition))) return false;

        value += addition;
      }

      dest = value;

      return true;
    }

    template <bool prfx = true>
    int _toHex(char * buff, size_t len, uint8_t number)
    {
      if __if_constexpr (prfx)
      {
        return snprintf_P(buff, len, TemplateHex, number);
      }
      else
      {
        return snprintf_P(buff, len, TemplateHex + 2, number);
      }
    }

    template <bool prfx,
              typename T,
              if_is_any_unsigned_t<T> Enable>
    int toHex(char * buff, size_t len, T number)
    {
      uint_fast8_t x = sizeof(T) - 1;
      uint_fast8_t offset = _toHex<prfx>(buff, len, (uint8_t)((number >> (x * 8)) & 0xFF));

      for (; x-- > 0;)
      {
        uint_fast8_t written = _toHex<false>(buff + offset, len - offset, (uint8_t)((number >> (x * 8)) & 0xFF));
        offset += written;
        if (written < 2) break; // full
      }

      return offset;
    }

    bool copyStr(char buffer[], size_t bufferSize, char const * str)
    {
      if (bufferSize < 1) return false;

      strncpy(buffer, str, bufferSize);

      // Check if null terminated
      if (buffer[bufferSize - 1] == '\0') return true;

      // Manually null terminate the string
      buffer[bufferSize - 1] = '\0';

      return false;
    }

    bool copyStrUpr(char buffer[], size_t bufferSize, char const * str)
    {
      if (bufferSize == 0) return false;

      bool copyResult = copyStr(buffer, bufferSize, str);

      strupr(buffer);

      return copyResult;
    }

    bool copyStrLwr(char buffer[], size_t bufferSize, char const * str)
    {
      if (bufferSize == 0) return false;

      bool copyResult = copyStr(buffer, bufferSize, str);

      strlwr(buffer);

      return copyResult;
    }

    bool copyStr_P(char buffer[], size_t bufferSize, char const * flashStr)
    {
      if (bufferSize < 1) return false;

      strncpy_P(buffer, flashStr, bufferSize);

      // Check if null terminated
      if (buffer[bufferSize - 1] == '\0') return true;

      // Manually null terminate the string
      buffer[bufferSize - 1] = '\0';

      return false;
    }

    bool copyStr_P(char buffer[], size_t bufferSize, __FlashStringHelper const * flashStr)
    {
      return copyStr_P(buffer, bufferSize, reinterpret_cast<char const *>(flashStr));
    }

    bool copyStrUpr_P(char buffer[], size_t bufferSize, char const * flashStr)
    {
      if (bufferSize == 0) return false;

      bool copyResult = copyStr_P(buffer, bufferSize, flashStr);

      strupr(buffer);

      return copyResult;
    }

    bool copyStrUpr_P(char buffer[], size_t bufferSize, __FlashStringHelper const * flashStr)
    {
      return copyStrUpr_P(buffer, bufferSize, reinterpret_cast<char const *>(flashStr));
    }

    bool copyStrLwr_P(char buffer[], size_t bufferSize, char const * flashStr)
    {
      if (bufferSize == 0) return false;

      bool copyResult = copyStr_P(buffer, bufferSize, flashStr);

      strlwr(buffer);

      return copyResult;
    }

    bool copyStrLwr_P(char buffer[], size_t bufferSize, __FlashStringHelper const * flashStr)
    {
      return copyStrLwr_P(buffer, bufferSize, reinterpret_cast<char const *>(flashStr));
    }


    bool applyTemplate_P(char buffer[], size_t bufferSize, char const * templateStr, int32_t num)
    {
      if (bufferSize < 1) return false;

      int32_t bufferWritten = snprintf_P(buffer, bufferSize, templateStr, num);

      return ((bufferWritten >= 0) && (bufferWritten < bufferSize));
    }

    bool applyTemplate_P(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, int32_t num)
    {
      return applyTemplate_P(buffer, bufferSize, reinterpret_cast<char const *>(templateStr), num);
    }

    bool applyTemplate_P(char buffer[], size_t bufferSize, char const * templateStr, uint32_t num)
    {
      if (bufferSize < 1) return false;

      int32_t bufferWritten = snprintf_P(buffer, bufferSize, templateStr, num);

      return ((bufferWritten >= 0) && (bufferWritten < bufferSize));
    }

    bool applyTemplate_P(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, uint32_t num)
    {
      return applyTemplate_P(buffer, bufferSize, reinterpret_cast<char const *>(templateStr), num);
    }

    bool applyTemplate_P(char buffer[], size_t bufferSize, char const * templateStr, char const * flashStr)
    {
      if (bufferSize < 1) return false;

      bool bufferStrWasSuccess = copyStr_P(bufferStr, flashBufferStrSize, flashStr);

      int32_t bufferWritten = snprintf_P(buffer, bufferSize, templateStr, bufferStr);

      return (bufferStrWasSuccess && (bufferWritten >= 0) && (bufferWritten < bufferSize));
    }

    bool applyTemplate_P(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, char const * flashStr)
    {
      return applyTemplate_P(buffer, bufferSize, reinterpret_cast<char const *>(templateStr), flashStr);
    }

    bool applyTemplate_P(char buffer[], size_t bufferSize, char const * templateStr, __FlashStringHelper const * flashStr)
    {
      return applyTemplate_P(buffer, bufferSize, templateStr, reinterpret_cast<char const *>(flashStr));
    }

    bool applyTemplate_P(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, __FlashStringHelper const * flashStr)
    {
      return applyTemplate_P(buffer, bufferSize, reinterpret_cast<char const *>(templateStr), reinterpret_cast<char const *>(flashStr));
    }

    bool applyTemplate(char buffer[], size_t bufferSize, char const * templateStr, char const * str)
    {
      if (bufferSize < 1) return false;

      int32_t bufferWritten = snprintf_P(buffer, bufferSize, templateStr, str);

      return ((bufferWritten >= 0) && (bufferWritten < bufferSize));
    }

    bool applyTemplate(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, char const * str)
    {
      return applyTemplate(buffer, bufferSize, reinterpret_cast<char const *>(templateStr), str);
    }

    bool isAlphaNumeric(uint8_t src)
    {
      return (((src >= 'a') && (src <= 'z')) || ((src >= 'A') && (src <= 'Z')) || ((src >= '0') && (src <= '9')));
    }
  }
}

template int8_t OSCR::Util::power<int8_t>(int8_t, int8_t);
template uint8_t OSCR::Util::power<uint8_t>(uint8_t, uint8_t);
template int16_t OSCR::Util::power<int16_t>(int16_t, int16_t);
template uint16_t OSCR::Util::power<uint16_t>(uint16_t, uint16_t);
template int32_t OSCR::Util::power<int32_t>(int32_t, int32_t);
template uint32_t OSCR::Util::power<uint32_t>(uint32_t, uint32_t);

template int8_t OSCR::Util::power<2>(int8_t);
template uint8_t OSCR::Util::power<2>(uint8_t);
template int16_t OSCR::Util::power<2>(int16_t);
template uint16_t OSCR::Util::power<2>(uint16_t);
template int32_t OSCR::Util::power<2>(int32_t);
template uint32_t OSCR::Util::power<2>(uint32_t);

template int8_t OSCR::Util::power<8>(int8_t);
template uint8_t OSCR::Util::power<8>(uint8_t);
template int16_t OSCR::Util::power<8>(int16_t);
template uint16_t OSCR::Util::power<8>(uint16_t);
template int32_t OSCR::Util::power<8>(int32_t);
template uint32_t OSCR::Util::power<8>(uint32_t);

template int8_t OSCR::Util::power<10>(int8_t);
template uint8_t OSCR::Util::power<10>(uint8_t);
template int16_t OSCR::Util::power<10>(int16_t);
template uint16_t OSCR::Util::power<10>(uint16_t);
template int32_t OSCR::Util::power<10>(int32_t);
template uint32_t OSCR::Util::power<10>(uint32_t);

template int8_t OSCR::Util::power<16>(int8_t);
template uint8_t OSCR::Util::power<16>(uint8_t);
template int16_t OSCR::Util::power<16>(int16_t);
template uint16_t OSCR::Util::power<16>(uint16_t);
template int32_t OSCR::Util::power<16>(int32_t);
template uint32_t OSCR::Util::power<16>(uint32_t);

template int OSCR::Util::toHex<true , uint8_t>(char * buff, size_t len, uint8_t number);
template int OSCR::Util::toHex<false, uint8_t>(char * buff, size_t len, uint8_t number);
template int OSCR::Util::toHex<true , uint16_t>(char * buff, size_t len, uint16_t number);
template int OSCR::Util::toHex<false, uint16_t>(char * buff, size_t len, uint16_t number);
template int OSCR::Util::toHex<true , uint32_t>(char * buff, size_t len, uint32_t number);
template int OSCR::Util::toHex<false, uint32_t>(char * buff, size_t len, uint32_t number);
template int OSCR::Util::toHex<true , uint64_t>(char * buff, size_t len, uint64_t number);
template int OSCR::Util::toHex<false, uint64_t>(char * buff, size_t len, uint64_t number);
