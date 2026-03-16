#include "common.h"

#if defined(ENABLE_SERIAL_ANSI)

#include "ui.h"
#include "hardware/outputs/SerialANSI.h"
#include "hardware/outputs/Serial.h"

namespace OSCR::Serial::ANSI
{
  constexpr char const PROGMEM ESC[]              = "\033[";
  constexpr char const PROGMEM SEP[]              = ";";
  constexpr char const PROGMEM END[]              = "m";
  constexpr char const PROGMEM CLEAR[]            = "\033[2J\033[3J\033[1;1H";
  constexpr char const PROGMEM TITLE[]            = "\033]2;%s\007";
  constexpr char const PROGMEM CURSOR[]           = "?25";
  constexpr char const PROGMEM SHOW[]             = "h";
  constexpr char const PROGMEM HIDE[]             = "l";

  constexpr char const PROGMEM C_SAVE[]           = "s";
  constexpr char const PROGMEM C_RESTORE[]        = "u";
  constexpr char const PROGMEM C_HOME[]           = "H";
  constexpr char const PROGMEM C_ERASE_LINE[]     = "K";
  constexpr char const PROGMEM C_ERASE_DIR[]      = "J";
  constexpr char const PROGMEM C_MOVE_UP[]        = "A";
  constexpr char const PROGMEM C_MOVE_DOWN[]      = "B";
  constexpr char const PROGMEM C_MOVE_FORWARD[]   = "C";
  constexpr char const PROGMEM C_MOVE_BACKWARD[]  = "D";

  Foreground foreground = Foreground::Default;
  Background background = Background::Default;
  Cursor cursor = Cursor::Show;

  void format(uint8_t code)
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(code, DEC);
    OSCR::Serial::print(FS(END));
  }

  void format(Format code)
  {
    format(code.value);
  }

  void format(Format codes[], uint8_t size)
  {
    OSCR::Serial::print(FS(ESC));

    for (uint8_t i = 0; i < size; ++i)
    {
      OSCR::Serial::print(codes[i].value, DEC);
      if (i < (size-1)) OSCR::Serial::print(FS(SEP));
    }

    OSCR::Serial::print(FS(END));
  }

  void _cursor(Cursor status)
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(FS(CURSOR));
    OSCR::Serial::print(FS((status == Cursor::Show) ? SHOW : HIDE));
    OSCR::Serial::flush();
    cursor = status;
  }

  void showCursor()
  {
    _cursor(Cursor::Show);
  }

  void hideCursor()
  {
    _cursor(Cursor::Hide);
  }

  void saveCursorPos()
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(FS(C_SAVE));
    OSCR::Serial::flush();
  }

  void restoreCursorPos()
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(FS(C_RESTORE));
    OSCR::Serial::flush();
  }

  void moveCursor(uint8_t row, uint8_t col)
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(row, DEC);
    OSCR::Serial::print(FS(SEP));
    OSCR::Serial::print(col, DEC);
    OSCR::Serial::print(FS(C_HOME));
    OSCR::Serial::flush();
  }

  void moveCursorUp(uint8_t rows)
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(rows, DEC);
    OSCR::Serial::print(FS(C_MOVE_UP));
    OSCR::Serial::flush();
  }

  void moveCursorDown(uint8_t rows)
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(rows, DEC);
    OSCR::Serial::print(FS(C_MOVE_DOWN));
    OSCR::Serial::flush();
  }

  void moveCursorForward(uint8_t cols)
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(cols, DEC);
    OSCR::Serial::print(FS(C_MOVE_FORWARD));
    OSCR::Serial::flush();
  }

  void moveCursorBackward(uint8_t cols)
  {
    OSCR::Serial::print(FS(ESC));
    OSCR::Serial::print(cols, DEC);
    OSCR::Serial::print(FS(C_MOVE_BACKWARD));
    OSCR::Serial::flush();
  }

  void eraseTo(uint8_t where, bool line)
  {
    OSCR::Serial::print(FS(ESC));
    if (where > 0) OSCR::Serial::print(where, DEC);
    OSCR::Serial::print(FS(line ? C_ERASE_LINE : C_ERASE_DIR));
    OSCR::Serial::flush();
  }

  void eraseToEnd()
  {
    eraseTo(0, true);
  }

  void eraseToStart()
  {
    eraseTo(1, true);
  }

  void eraseLine()
  {
    eraseTo(2, true);
  }

  void eraseDown()
  {
    eraseTo(0, false);
  }

  void eraseUp()
  {
    eraseTo(1, false);
  }

  void setForeground(Foreground fg)
  {
    foreground = fg;
  }

  void setBackground(Background bg)
  {
    background = bg;
  }

  void apply(bool resetStyles)
  {
    Format codes[2] = {
      { .foreground = foreground },
      { .background = background },
    };

    if (resetStyles) format(Style::Reset);

    format(codes, 2);

    OSCR::Serial::flush();
  }

  void reset()
  {
    foreground = Foreground::Default;
    background = Background::Default;

    format(Style::Reset);

    OSCR::Serial::flush();
  }

  void clear(bool resetStyle)
  {
    OSCR::Serial::print(FS(CLEAR));

    if (resetStyle) reset();
    else OSCR::Serial::flush();
  }

    void boldUpper(__FlashStringHelper const * flashStr)
    {
      OSCR::Serial::ANSI::format(Style::Bold);
      OSCR::Lang::printUpper(flashStr);
      OSCR::Serial::ANSI::format(Style::Reset);
    }

    void boldUpperLine(__FlashStringHelper const * flashStr)
    {
      boldUpper(flashStr);
      OSCR::Serial::printLine();
    }

    void boldUpper(char const * str)
    {
      OSCR::Serial::ANSI::format(Style::Bold);
      OSCR::Lang::printUpper(str);
      OSCR::Serial::ANSI::format(Style::Reset);
    }

    void boldUpperLine(char const * str)
    {
      boldUpper(str);
      OSCR::Serial::printLine();
    }
}

#endif /* ENABLE_SERIAL_ANSI */
