/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#if !defined(OSCR_HARDWARE_OUTPUTS_SERIAL_H_)
# define OSCR_HARDWARE_OUTPUTS_SERIAL_H_

# include "config.h"

# if defined(ENABLE_SERIAL_OUTPUT) || defined(ENABLE_UPDATER)

#   include "common/specializations.h"

namespace OSCR::Serial
{
  constexpr char const ending = '\n';
  constexpr char const ending2 = '\r';
  constexpr uint8_t const commandMax = 32;
  constexpr uint8_t const ansiEsc[2] = { 0x1B, 0x5B };

  extern uint32_t const baud;

  extern char command[commandMax];
  char const * getCommandPointer();
  uint8_t const * getCommandMaxPointer();

  extern void begin();
  extern void begin(uint32_t b);
  extern void end();

  extern void setup();

  extern int read();

  extern void printLine(void);

  extern void flush();
  extern void flushSync();

  extern bool echo(bool toggle);
  extern bool echo();

  extern bool getCommand();

  //! @cond

  extern int available();
  extern int availableForWrite();

  extern size_t write(uint8_t const byte);
  extern size_t write(char const chr);
  extern size_t write(uint8_t const * buffer, size_t size);
  extern size_t write(char const * buffer, size_t size);

  extern void printLine(void);

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
  extern void print(T string);

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
  extern void printLine(T string);

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
  extern void print(Tint number, int base = DEC);

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
  extern void printLine(Tint number, int base = DEC);

  extern void printLineSync(void);

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
  extern void printSync(T string);

  template <typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
  extern void printLineSync(T string);

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
  extern void printSync(Tint number, int base = DEC);

  template <typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
  extern void printLineSync(Tint number, int base = DEC);

  template <bool prfx = true,
            typename T,
            OSCR::Util::if_is_any_unsigned_t<T> Enable = true>
  extern void printHex(T number);

  template <bool prfx = true,
            typename T,
            OSCR::Util::if_is_any_unsigned_t<T> Enable = true>
  extern void printHexLine(T number);

  /**
   * @brief Print a label.
   *
   * Prints a label using the provided string.
   *
   * Example Output: `LabelText: `
   *
   * @param label A flash pointer to use for label.
   */
  extern void printLabel(char const * label);

  /**
   * @brief Print a label and a value.
   *
   * Prints a line with the provided label and value pair.
   *
   * Example Output: `LabelText: ValueText`
   *
   * @param label A flash pointer to use for label.
   * @param value A string (char array) in SRAM to use as the value.
   */
  extern void printValue(char const * label, char const * value);

  /**
   * @brief Print a label and a value.
   *
   * Prints a line with the provided label and value pair.
   *
   * Example Output: `LabelText: ValueText`
   *
   * @param label A flash pointer to use for label.
   * @param value A flash pointer to use as the value.
   */
  extern void printValue_P(char const * label, char const * value);

  /**
   * @brief Print a label and a value.
   *
   * Prints a line with the provided label and value pair.
   *
   * Example Output: `LabelText: Value`
   *
   * @param label A flash pointer to use for label.
   * @param value A number in SRAM to use as the value.
   */
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable = true>
  extern void printValue(char const * label, T value);

  /**
   * @brief Print a label and a value out of a total.
   *
   * Prints a line with the provided label, value, and total.
   *
   * Example Output: `LabelText: Value/Total`
   *
   * @param label A flash pointer to use for label.
   * @param value A number in SRAM to use as the value.
   * @param total A number in SRAM to use as the total.
   */
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable = true>
  extern void printValue(char const * label, T value, T total);


  void printSize(char const * label, uint32_t size);
  void printSize(char const * label, uint32_t size, bool isBytes);
  void printSize_P(char const * label, char const * size);

  void printType(char const * label);
  void printType(char const * label, char const * value);
  void printType_P(char const * label, char const * value);

  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable = true>
  extern void printType(char const * label, T number);

  //! @cond

  /**
   * Invalid. Remove `FS()`, or use `PSTR()` instead of `F()`.
   */
  inline void printLabel(__FlashStringHelper const * label) = delete;

  /**
   * Invalid. Remove `FS()`, or use `PSTR()` instead of `F()`.
   */
  inline void printValue_P(__FlashStringHelper const * label, __FlashStringHelper const * value) = delete;

  /**
   * Invalid. Remove `FS()`, or use `PSTR()` instead of `F()`.
   */
  inline void printValue_P(__FlashStringHelper const * label, char const * value) = delete;

  /**
   * Invalid. Remove `FS()`, or use `PSTR()` instead of `F()`.
   */
  inline void printValue_P(char const * label, __FlashStringHelper const * value) = delete;

  /**
   * Invalid. Remove `FS()`, or use `PSTR()` instead of `F()`.
   */
  inline void printValue(char const * label, __FlashStringHelper const * value) = delete;

  /**
   * Invalid. Remove `FS()`, or use `PSTR()` instead of `F()`.
   */
  inline void printValue(__FlashStringHelper const * label, char const * value) = delete;

  //! @endcond

  extern void gotoLast();

  //! @endcond

#   if defined(ENABLE_SERIAL_OUTPUT)

  //! @cond

  template <bool sync,
            typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
  extern void print(T string);

  template <bool sync,
            typename T,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
            OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
  extern void printLine(T string);

  template <bool sync,
            typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
  extern void print(Tint number, int base);

  template <bool sync,
            typename Tint,
            OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
  extern void printLine(Tint number, int base);

  //! @endcond

#   endif /* ENABLE_SERIAL_OUTPUT */

} /* namespace OSCR::UI */

# endif /* ENABLE_SERIAL_OUTPUT || ENABLE_UPDATER */

#endif /* !OSCR_HARDWARE_OUTPUTS_SERIAL_H_ */
