/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#if !defined(OSCR_HARDWARE_OUTPUTS_SERIAL_ANSI_H_)
# define OSCR_HARDWARE_OUTPUTS_SERIAL_ANSI_H_

# include "config.h"

# if defined(ENABLE_SERIAL_ANSI)

#   include "common/Types.h"
#   include "common/specializations.h"
#   include "hardware/outputs/Serial.h"

namespace OSCR::Serial
{
  namespace ANSI
  {
    extern char const PROGMEM ESC[];
    extern char const PROGMEM SEP[];
    extern char const PROGMEM END[];
    extern char const PROGMEM CLEAR[];
    extern char const PROGMEM TITLE[];
    extern char const PROGMEM CURSOR[];
    extern char const PROGMEM SHOW[];
    extern char const PROGMEM HIDE[];

    extern char const PROGMEM C_SAVE[];
    extern char const PROGMEM C_RESTORE[];
    extern char const PROGMEM C_HOME[];
    extern char const PROGMEM C_ERASE_LINE[];
    extern char const PROGMEM C_ERASE_DIR[];
    extern char const PROGMEM C_MOVE_UP[];
    extern char const PROGMEM C_MOVE_DOWN[];
    extern char const PROGMEM C_MOVE_FORWARD[];
    extern char const PROGMEM C_MOVE_BACKWARD[];

    // Escape sequence prefix for input events (arrow keys)
    inline constexpr uint8_t const inputEsc[2] = { 0x1B, 0x5B };

    extern Foreground foreground;
    extern Background background;
    extern Cursor cursor;

    template <typename T,
              OSCR::Util::enable_if_t<OSCR::Util::is_format<T>::value, bool> Enable = true>
    inline uint8_t code(T _code)
    {
      return static_cast<uint8_t>(_code);
    }

    extern void format(uint8_t code);
    extern void format(Format code);
    extern void format(Format codes[], uint8_t size);

    template <typename T,
              OSCR::Util::enable_if_t<OSCR::Util::is_format<T>::value, bool> Enable = true>
    inline void format(T code)
    {
      format(static_cast<uint8_t>(code));
    }

    extern void showCursor();
    extern void hideCursor();

    extern void saveCursorPos();
    extern void restoreCursorPos();

    extern void moveCursor(uint8_t row, uint8_t col = 0);

    extern void moveCursorUp(uint8_t rows = 1);
    extern void moveCursorDown(uint8_t rows = 1);
    extern void moveCursorForward(uint8_t cols = 1);
    extern void moveCursorBackward(uint8_t cols = 1);

    extern void eraseToEnd();
    extern void eraseToStart();
    extern void eraseLine();
    extern void eraseDown();
    extern void eraseUp();

    extern void setForeground(Foreground fg);
    extern void setBackground(Background bg);

    extern void apply(bool resetStyles = true);

    extern void clear(bool reset = true);
    extern void reset();

    template <typename T>
    void bold(T text)
    {
      Serial::ANSI::format(Style::Bold);
      OSCR::Serial::print(text);
      Serial::ANSI::format(Style::Reset);
    }

    template <typename T>
    void boldLine(T text)
    {
      bold(text);
      OSCR::Serial::printLine();
    }

    extern void boldUpper(char const * str);
    extern void boldUpper(__FlashStringHelper const * flashStr);

    extern void boldUpperLine(char const * str);
    extern void boldUpperLine(__FlashStringHelper const * flashStr);
  }
}

# endif /* ENABLE_SERIAL_ANSI */

#endif /* OSCR_HARDWARE_OUTPUTS_SERIAL_ANSI_H_ */
