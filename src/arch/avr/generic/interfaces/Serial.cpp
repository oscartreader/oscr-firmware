
# include "arch/avr/syslibinc.h"

#if defined(OSCR_ARCH_AVR)

# include "config.h"

# if defined(ENABLE_SERIAL_OUTPUT) || defined(ENABLE_UPDATER)

#   include "arch/avr/generic/interfaces/Serial.h"
#   include "common/Updater.h"
#   include "common/Util.h"
#   include "ui.h"

namespace OSCR::Serial
{
  char command[commandMax];
#   if defined(ENABLE_SERIAL_ANSI)
  constexpr uint32_t const baud = 115200;
#   else /* !ENABLE_SERIAL_ANSI */
  constexpr uint32_t const baud = UPD_BAUD;
#   endif /* ENABLE_SERIAL_ANSI */

#   if SERIAL_TX_BUFFER_SIZE < 256
  constexpr uint8_t kTxBufferSize = (SERIAL_TX_BUFFER_SIZE - 1);
#   else
  constexpr uint16_t kTxBufferSize = (SERIAL_TX_BUFFER_SIZE - 1);
#   endif

  bool echoTyping = false;

  char const * getCommandPointer()
  {
    return &command[0];
  }

  uint8_t const * getCommandMaxPointer()
  {
    return &commandMax;
  }

  void begin()
  {
    ClockedSerial.begin(baud);
  }

  void begin(uint32_t b)
  {
    ClockedSerial.begin(b);
  }

  void clockSkew(uint32_t sclock)
  {
    ClockedSerial.clockSkew(sclock);
  }

  void end()
  {
    ClockedSerial.end();
  }

  void setup()
  {
    begin();

#   if defined(ENABLE_UPDATER)
    OSCR::Updater::printVersionToSerial();
#   endif /* ENABLE_UPDATER */
  }

  int available()
  {
    return ClockedSerial.available();
  }

  int availableForWrite()
  {
    return ClockedSerial.availableForWrite();
  }

  size_t write(uint8_t const byte)
  {
    return ClockedSerial.write(byte);
  }

  size_t write(char const chr)
  {
    return ClockedSerial.write((uint8_t)chr);
  }

  size_t write(uint8_t const * buffer, size_t size)
  {
    return ClockedSerial.write(buffer, size);
  }

  size_t write(char const * buffer, size_t size)
  {
    return ClockedSerial.write((uint8_t const *)buffer, size);
  }

  void printLine(void)
  {
    ClockedSerial.print(F("\r\n"));
  }

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
  void print(T string)
  {
    ClockedSerial.print(string);
  }

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
  void printLine(T string)
  {
    ClockedSerial.print(string);
    printLine();
  }

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
  void print(Tint number, int base)
  {
    ClockedSerial.print(number, base);
  }

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
  void printLine(Tint number, int base)
  {
    ClockedSerial.print(number, base);
    printLine();
  }

  void printLineSync(void)
  {
    printLine();
    flushSync();
  }

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
  void printSync(T string)
  {
    ClockedSerial.print(string);
    flushSync();
  }

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
  void printLineSync(T string)
  {
    ClockedSerial.print(string);
    printLineSync();
  }

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
  void printSync(Tint number, int base)
  {
    ClockedSerial.print(number, base);
    flushSync();
  }

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
  void printLineSync(Tint number, int base)
  {
    ClockedSerial.print(number, base);
    printLineSync();
  }

  template <bool prfx,
            typename T,
            OSCR::Util::if_is_any_unsigned_t<T> Enable>
  void printHex(T number)
  {
    uint8_t const kBuffSize = (sizeof(T) * 2) + (prfx * 2) + 1;
    char buff[kBuffSize];

    OSCR::Util::toHex<prfx>(buff, kBuffSize, number);

    printSync(buff);
  }

  template <bool prfx,
            typename T,
            OSCR::Util::if_is_any_unsigned_t<T> Enable>
  void printHexLine(T number)
  {
    printHex<prfx>(number);
    printLine();
  }

  // flash pointer
  void printLabel(char const * label)
  {
    OSCR::Serial::printSync(OSCR::Lang::formatLabel(label));
  }

  // flash pointer + SRAM pointer
  void printValue(char const * label, char const * value)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatLabel(label, value));
  }

  // flash pointer + flash pointer
  void printValue_P(char const * label, char const * value)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatLabel_P(label, value));
  }

  // flash pointer + SRAM number
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable>
  void printValue(char const * label, T number)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatLabel(label, number));
  }

  // flash pointer + SRAM number + SRAM number
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable>
  void printValue(char const * label, T number, T total)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatLabel(label, number, total));
  }

  // flash pointer
  void printType(char const * label)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatType(label));
  }

  // flash pointer + SRAM pointer
  void printType(char const * label, char const * value)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatType(label, value));
  }

  // flash pointer + flash pointer
  void printType_P(char const * label, char const * value)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatType_P(label, value));
  }

  // flash pointer + SRAM number
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable>
  void printType(char const * label, T number)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatType(label, number));
  }

  // flash pointer + SRAM number + bool
  void printSize(char const * label, uint32_t size, bool isBytes)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatLabelSize(label, size, isBytes));
  }

  // flash pointer + SRAM number
  void printSize(char const * label, uint32_t size)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatLabelSize(label, size, false));
  }

  // flash pointer + flash pointer
  void printSize_P(char const * label, char const * size)
  {
    OSCR::Serial::printLineSync(OSCR::Lang::formatLabel_P(label, size));
  }

  void flush()
  {
    ClockedSerial.flush();
  }

  void flushSync()
  {
    bool flushed = false;
    while (ClockedSerial.availableForWrite() < kTxBufferSize)
    {
      ClockedSerial.flush();
      flushed = true;
    }

    if (flushed) delay(25);
  }

  bool echo(bool toggle)
  {
    echoTyping = toggle;
    return echoTyping;
  }

  bool echo()
  {
    return echoTyping;
  }

  int read()
  {
    return ClockedSerial.read();
  }

  int peek()
  {
    return ClockedSerial.peek();
  }

  int parseInt()
  {
    return ClockedSerial.parseInt();
  }

  bool getCommand()
  {
    static uint8_t index = 0;
    char data;

    while (available() > 0)
    {
      data = read();

      if (data != ending && data != ending2)
      {
        command[index] = data;

        index++;

        if (index >= commandMax)
        {
            index = commandMax - 1;
        }

#   if defined(ENABLE_SERIAL_ANSI)
        if ((echoTyping) && (ANSI::inputEsc[0] != command[0]))
#   else /* !ENABLE_SERIAL_ANSI */
        if (echoTyping)
#   endif /* ENABLE_SERIAL_ANSI */
          print(data); // Echo typing
      }
      else if (index != 0)
      {
        command[index] = '\0'; // terminate the string
        index = 0;

#   if defined(ENABLE_SERIAL_ANSI)
        if ((echoTyping) && (ANSI::inputEsc[0] != command[0]))
#   else /* !ENABLE_SERIAL_ANSI */
        if (echoTyping)
#   endif /* ENABLE_SERIAL_ANSI */
          printLine(); // new line

        return true;
      }

#   if defined(ENABLE_SERIAL_ANSI)
      if ( (index > 2) && (ANSI::inputEsc[0] == command[0]) && (ANSI::inputEsc[1] == command[1]) )
      {
        if (ANSI::state != ANSI::ANSIState::Waiting)
        {
          if (!ANSI::receiveControl(index - 1))
          {
            command[0] = '\0';
            index = 0;

            return false;
          }
        }

        switch (command[2])
        {
          case UI_INPUT_SERIAL_ANSI_NEXT:
          case UI_INPUT_SERIAL_ANSI_BACK:
          case UI_INPUT_SERIAL_ANSI_CONFIRM:
            command[index] = '\0';
            index = 0;

            return true;

          default:
            continue;
        }
      }
#   endif /* ENABLE_SERIAL_ANSI */
    }

    return false;
  }

#   if defined(ENABLE_SERIAL_OUTPUT)

  void gotoLast()
  {
    printLine();
  }

#   endif /* !ENABLE_SERIAL_OUTPUT */

} /* namespace OSCR::Serial */

//! @cond
template void OSCR::Serial::print<__StringHelper const *>(__StringHelper const *);
template void OSCR::Serial::print<String const &>(String const &);
template void OSCR::Serial::print<char const[]>(char const[]);
template void OSCR::Serial::print<char const *>(char const *);
template void OSCR::Serial::print<char *>(char *);
template void OSCR::Serial::print<OSCR::UI::UserInput>(OSCR::UI::UserInput);
template void OSCR::Serial::print<int8_t>(int8_t, int);
template void OSCR::Serial::print<uint8_t>(uint8_t, int);
template void OSCR::Serial::print<int16_t>(int16_t, int);
template void OSCR::Serial::print<uint16_t>(uint16_t, int);
template void OSCR::Serial::print<int32_t>(int32_t, int);
template void OSCR::Serial::print<uint32_t>(uint32_t, int);

template void OSCR::Serial::printLine<__StringHelper const *>(__StringHelper const *);
template void OSCR::Serial::printLine<String const &>(String const &);
template void OSCR::Serial::printLine<char const[]>(char const[]);
template void OSCR::Serial::printLine<char const *>(char const *);
template void OSCR::Serial::printLine<char *>(char *);
template void OSCR::Serial::printLine<OSCR::UI::UserInput>(OSCR::UI::UserInput);
template void OSCR::Serial::printLine<int8_t>(int8_t, int);
template void OSCR::Serial::printLine<uint8_t>(uint8_t, int);
template void OSCR::Serial::printLine<int16_t>(int16_t, int);
template void OSCR::Serial::printLine<uint16_t>(uint16_t, int);
template void OSCR::Serial::printLine<int32_t>(int32_t, int);
template void OSCR::Serial::printLine<uint32_t>(uint32_t, int);

template void OSCR::Serial::printSync<__StringHelper const *>(__StringHelper const *);
template void OSCR::Serial::printSync<String const &>(String const &);
template void OSCR::Serial::printSync<char const[]>(char const[]);
template void OSCR::Serial::printSync<char const *>(char const *);
template void OSCR::Serial::printSync<char *>(char *);
template void OSCR::Serial::printSync<OSCR::UI::UserInput>(OSCR::UI::UserInput);
template void OSCR::Serial::printSync<int8_t>(int8_t, int);
template void OSCR::Serial::printSync<uint8_t>(uint8_t, int);
template void OSCR::Serial::printSync<int16_t>(int16_t, int);
template void OSCR::Serial::printSync<uint16_t>(uint16_t, int);
template void OSCR::Serial::printSync<int32_t>(int32_t, int);
template void OSCR::Serial::printSync<uint32_t>(uint32_t, int);

template void OSCR::Serial::printLineSync<__StringHelper const *>(__StringHelper const *);
template void OSCR::Serial::printLineSync<String const &>(String const &);
template void OSCR::Serial::printLineSync<char const[]>(char const[]);
template void OSCR::Serial::printLineSync<char const *>(char const *);
template void OSCR::Serial::printLineSync<char *>(char *);
template void OSCR::Serial::printLineSync<OSCR::UI::UserInput>(OSCR::UI::UserInput);
template void OSCR::Serial::printLineSync<int8_t>(int8_t, int);
template void OSCR::Serial::printLineSync<uint8_t>(uint8_t, int);
template void OSCR::Serial::printLineSync<int16_t>(int16_t, int);
template void OSCR::Serial::printLineSync<uint16_t>(uint16_t, int);
template void OSCR::Serial::printLineSync<int32_t>(int32_t, int);
template void OSCR::Serial::printLineSync<uint32_t>(uint32_t, int);

template void OSCR::Serial::printHex<true >(uint8_t number);
template void OSCR::Serial::printHex<false>(uint8_t number);
template void OSCR::Serial::printHex<true >(uint16_t number);
template void OSCR::Serial::printHex<false>(uint16_t number);
template void OSCR::Serial::printHex<true >(uint32_t number);
template void OSCR::Serial::printHex<false>(uint32_t number);
template void OSCR::Serial::printHex<true >(uint64_t number);
template void OSCR::Serial::printHex<false>(uint64_t number);

template void OSCR::Serial::printHexLine<true >(uint8_t number);
template void OSCR::Serial::printHexLine<false>(uint8_t number);
template void OSCR::Serial::printHexLine<true >(uint16_t number);
template void OSCR::Serial::printHexLine<false>(uint16_t number);
template void OSCR::Serial::printHexLine<true >(uint32_t number);
template void OSCR::Serial::printHexLine<false>(uint32_t number);
template void OSCR::Serial::printHexLine<true >(uint64_t number);
template void OSCR::Serial::printHexLine<false>(uint64_t number);

template void OSCR::Serial::printValue(char const * label, int8_t number);
template void OSCR::Serial::printValue(char const * label, uint8_t number);
template void OSCR::Serial::printValue(char const * label, int16_t number);
template void OSCR::Serial::printValue(char const * label, uint16_t number);
template void OSCR::Serial::printValue(char const * label, int32_t number);
template void OSCR::Serial::printValue(char const * label, uint32_t number);

template void OSCR::Serial::printValue(char const * label, int8_t number, int8_t total);
template void OSCR::Serial::printValue(char const * label, uint8_t number, uint8_t total);
template void OSCR::Serial::printValue(char const * label, int16_t number, int16_t total);
template void OSCR::Serial::printValue(char const * label, uint16_t number, uint16_t total);
template void OSCR::Serial::printValue(char const * label, int32_t number, int32_t total);
template void OSCR::Serial::printValue(char const * label, uint32_t number, uint32_t total);

template void OSCR::Serial::printType(char const * label, int8_t number);
template void OSCR::Serial::printType(char const * label, uint8_t number);
template void OSCR::Serial::printType(char const * label, int16_t number);
template void OSCR::Serial::printType(char const * label, uint16_t number);
template void OSCR::Serial::printType(char const * label, int32_t number);
template void OSCR::Serial::printType(char const * label, uint32_t number);
  //! @endcond

# endif /* ENABLE_SERIAL_OUTPUT || ENABLE_UPDATER */

#endif /* OSCR_ARCH_AVR */
