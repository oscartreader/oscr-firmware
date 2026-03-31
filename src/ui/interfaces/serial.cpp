#include "common.h"

#if defined(ENABLE_SERIAL_OUTPUT)
# include "common/Updater.h"
# include "ui/interfaces.h"
# include "ui/l10n.h"
# include "ui/interfaces/serial.h"

namespace OSCR::UI
{
  uint8_t printText(char const * const text)
  {
    printLine(text);
    return 1;
  }

  template <bool sync>
  void printLine(void)
  {
    ClockedSerial.print(FS(OSCR::Strings::Symbol::NewLine));

    if __if_constexpr (sync)
    {
      OSCR::UI::update();
    }
  }

  template <bool sync,
            typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
  void print(T string)
  {
    ClockedSerial.print(string);

    if __if_constexpr (sync)
    {
      OSCR::UI::update();
    }
  }

  template <bool sync,
            typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
  void printLine(T string)
  {
    ClockedSerial.print(string);

    printLine();

    if __if_constexpr (sync)
    {
      OSCR::UI::update();
    }
  }

  template <bool sync,
            typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
  void print(Tint number, int base)
  {
    ClockedSerial.print(number, base);

    if __if_constexpr (sync)
    {
      OSCR::UI::update();
    }
  }

  template <bool sync,
            typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
  void printLine(Tint number, int base)
  {
    ClockedSerial.print(number, base);

    printLine();

    if __if_constexpr (sync)
    {
      OSCR::UI::update();
    }
  }

  void gotoLast()
  {
    printLine();
  }
} /* namespace OSCR::UI */

//! @cond
template void OSCR::UI::print<false, __FlashStringHelper const *>(__FlashStringHelper const *);
template void OSCR::UI::print<true,  __FlashStringHelper const *>(__FlashStringHelper const *);
template void OSCR::UI::print<false, String const &>(String const &);
template void OSCR::UI::print<true,  String const &>(String const &);
template void OSCR::UI::print<false, char const[]>(char const[]);
template void OSCR::UI::print<true,  char const[]>(char const[]);
template void OSCR::UI::print<false, char *>(char *);
template void OSCR::UI::print<true,  char *>(char *);
template void OSCR::UI::print<false, char const *>(char const *);
template void OSCR::UI::print<true,  char const *>(char const *);
template void OSCR::UI::print<false, OSCR::UI::UserInput>(OSCR::UI::UserInput);
template void OSCR::UI::print<true,  OSCR::UI::UserInput>(OSCR::UI::UserInput);
template void OSCR::UI::print<false, int8_t>(int8_t, int);
template void OSCR::UI::print<true,  int8_t>(int8_t, int);
template void OSCR::UI::print<false, uint8_t>(uint8_t, int);
template void OSCR::UI::print<true,  uint8_t>(uint8_t, int);
template void OSCR::UI::print<false, int16_t>(int16_t, int);
template void OSCR::UI::print<true,  int16_t>(int16_t, int);
template void OSCR::UI::print<false, uint16_t>(uint16_t, int);
template void OSCR::UI::print<true,  uint16_t>(uint16_t, int);
template void OSCR::UI::print<false, int32_t>(int32_t, int);
template void OSCR::UI::print<true,  int32_t>(int32_t, int);
template void OSCR::UI::print<false, uint32_t>(uint32_t, int);
template void OSCR::UI::print<true,  uint32_t>(uint32_t, int);

template void OSCR::UI::printLine<false, __FlashStringHelper const *>(__FlashStringHelper const *);
template void OSCR::UI::printLine<true,  __FlashStringHelper const *>(__FlashStringHelper const *);
template void OSCR::UI::printLine<false, String const &>(String const &);
template void OSCR::UI::printLine<true,  String const &>(String const &);
template void OSCR::UI::printLine<false, char const[]>(char const[]);
template void OSCR::UI::printLine<true,  char const[]>(char const[]);
template void OSCR::UI::printLine<false, char *>(char *);
template void OSCR::UI::printLine<true,  char *>(char *);
template void OSCR::UI::printLine<false, char const *>(char const *);
template void OSCR::UI::printLine<true,  char const *>(char const *);
template void OSCR::UI::printLine<false, OSCR::UI::UserInput>(OSCR::UI::UserInput);
template void OSCR::UI::printLine<true,  OSCR::UI::UserInput>(OSCR::UI::UserInput);
template void OSCR::UI::printLine<false, int8_t>(int8_t, int);
template void OSCR::UI::printLine<true,  int8_t>(int8_t, int);
template void OSCR::UI::printLine<false, uint8_t>(uint8_t, int);
template void OSCR::UI::printLine<true,  uint8_t>(uint8_t, int);
template void OSCR::UI::printLine<false, int16_t>(int16_t, int);
template void OSCR::UI::printLine<true,  int16_t>(int16_t, int);
template void OSCR::UI::printLine<false, uint16_t>(uint16_t, int);
template void OSCR::UI::printLine<true,  uint16_t>(uint16_t, int);
template void OSCR::UI::printLine<false, int32_t>(int32_t, int);
template void OSCR::UI::printLine<true,  int32_t>(int32_t, int);
template void OSCR::UI::printLine<false, uint32_t>(uint32_t, int);
template void OSCR::UI::printLine<true,  uint32_t>(uint32_t, int);

template void OSCR::UI::printLine<false>();
template void OSCR::UI::printLine<true >();
//! @endcond

#endif /* ENABLE_SERIAL_OUTPUT */
