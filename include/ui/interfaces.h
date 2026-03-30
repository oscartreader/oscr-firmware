/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_INTERFACES_H_
#define OSCR_INTERFACES_H_

#include "syslibinc.h"
#include "config.h"
#include "common/specializations.h"
#include "common/Util.h"

#define menuoptionsize(menu) (size_t)sizeof(menu.options[0])
#define menusize(menu) (size_t)(sizeof(menu.options)/sizeof(menu.options[0]))

namespace OSCR
{
  /**
   * @brief User interface methods
   */
  namespace UI
  {
    extern bool const kSupportsLineAdjustments;

    /**
     * @brief Display width in pixels (0 = unsupported).
     *
     * The width of the display in pixels, or 0 if the output does
     *  not support graphics.
     */
    extern uint8_t const kDisplayWidth;

    /**
     * @brief Display height in pixels (0 = unsupported).
     *
     * The height of the display in pixels, or 0 if the output does
     *  not support graphics.
     */
    extern uint8_t const kDisplayHeight;

    /**
     * @brief Line height in pixels (0 = unsupported).
     *
     * The height of a line on the display in pixels, or 0 if the
     *  output does not support graphics.
     */
    extern uint8_t const kLineHeight;

    /**
     * @brief Number of fixed-width columns.
     *
     * The number of characters that can fit on a line, or 0 if the
     *  output does not use a fixed-width font.
     */
    extern uint8_t const kDisplayCols;

    /**
     * @brief Number of fixed-width rows.
     *
     * The number of lines an output using a fixed-width font can
     *   display, or 0 if it does not use a fixed-width font.
     */
    extern uint8_t const kDisplayRows;

    /**
     * @brief Lines that can be seen at once (0 = unlimited).
     *
     * The number of lines the output can shown at a time, or 0 for
     *   an unlimited (infinite) number of lines.
     */
    extern uint8_t const kDisplayLines;

#if IS_DOXYGEN

    /**
     * @brief Maximum number of rows/options in most menus
     *
     * The maximum number of rows/options that will be shown at once
     *   in most menus. Though it will typically be the same value as
     *   `kDisplayLines`, this one may not be a constant. It should be
     *   treated as a read-only variable that can only be changed by
     *   the file which defines it.
     */
    extern uint8_t const kPageRowMax;
#endif

    /**
     * @brief Y position of the first line.
     *
     * The Y position of the first line. This variable can be 0, so
     *  check `kDisplayLines` or another variable for line support.
     */
    extern uint8_t const kDisplayLineStart;

    /**
     * @brief Data struct for Menu instances.
     *
     * This struct is for holding data related to the options of
     * a menu. The template parameters are the maximums. There
     * will need to enough memory free to hold all of the options
     * while the menu exists.
     *
     * Do not change the count parameter to one larger than the
     * value passed to the template.
     */
    template <uint8_t optionCount, uint8_t optionLength>
    struct MenuOptions
    {
        char options[optionCount][optionLength];
        OSCR::Util::clamped_value<uint8_t, 0, optionCount> count = optionCount;
        uint8_t const length = optionLength;
        uint8_t const max = optionCount;

        char * operator[](size_t idx) { return options[idx]; }
        char const * operator[](size_t idx) const { return options[idx]; }
    };

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

    /**
     * Enter drowse state (dim displays, LEDs, etc.)
     */
    extern void drowse();

    /**
     * Enter sleep state (disable displays, LEDs, etc.)
     */
    extern void sleep();

    /**
     * Enter wake state (enable displays, LEDs, etc.)
     */
    extern void wake();

    /**
     * Wait for any user input (button press, etc).
     */
    extern void wait();

    /**
     * Prompt the user to push a button and wait for them to do so.
     */
    extern void waitButton();

    /**
     * Wait for any user input and return that input event.
     */
    extern UserInput waitInput();

    /**
     * Run %UI setup.
     */
    extern void setup();

    /**
     * Toggle the LED
     */
    extern void blinkLED();

    /**
     * Update the output target with any pending data.
     */
    extern void update();

    /**
     * Clear the output.
     */
    extern void clear();

    /**
     * Clear the current line. Creates a newline if unsupported.
     */
    extern void clearLine();

    /**
     * Check for user input.
     */
    extern UserInput checkButton();

    class RangeSelect
    {
    public:
      /**
       * @brief Create a menu interface for selecting a number from a range.
       *
       * @param menuTitle A flash string to use for the menu title.
       * @param rangeMin The minimum selectable number.
       * @param rangeMax The maximum selectable number.
       */
      RangeSelect(__FlashStringHelper const * menuTitle, uint16_t const rangeMin = 0, uint16_t const rangeMax = UINT16_MAX);

      // From UI submodule
      void render();

      /**
       * Prompt the user to select a number within range and return it.
       *
       * @return  The selected number.
       */
      uint16_t select();

      /**
       * Prompt the user to select a number within range and return it.
       *
       * @param startAt The number to start at.
       * @return  The selected number.
       */
      uint16_t select(uint16_t const startAt);

      /**
       * Increment the currently selected number.
       */
      void valueIncrement();

      /**
       * Decrement the currently selected number.
       */
      void valueDecrement();

    protected:
      __FlashStringHelper const * title;
      uint16_t minValue;
      uint16_t maxValue;
      uint16_t currentValue;

      uint8_t positions;
      uint8_t currentPosition = 0;
      bool rendered = false;

      bool valueInBounds(uint32_t newValue);
      int32_t diffValue(bool willRollover, int32_t tens, int8_t rolloverNumber, int8_t step);
      bool adjustValue(bool willRollover, int32_t tens, int8_t rolloverNumber, int8_t step);

      uint8_t getPositionsFor(uint16_t value);
      int8_t getNumberAt(uint16_t value, uint8_t position);
      void _setup();

      // From UI submodule
      void navigate(NavDir direction);
      void onSelectionChange();
    };

    /**
     * @class MenuRenderer
     * @brief Abstract menu renderer class
     *
     * Base class used for writing menus to the output. Methods need
     * to be implemented by a UI output implementation.
     *
     * @note
     * This abstract class cannot be used directly. It needs to be
     * extended. See the other menu classes for examples.
     */
    class MenuRenderer
    {
    public:
      MenuRenderer(char const * menuTitle);
      MenuRenderer(char const * menuTitle, uint16_t entryCount);

      MenuRenderer(__FlashStringHelper const * menuTitle);
      MenuRenderer(__FlashStringHelper const * menuTitle, uint16_t entryCount);

      virtual ~MenuRenderer() {}; // Virtual destructor

      uint16_t getPage();
      uint16_t getPageCount();
      bool isLast();
      uint16_t getPageEntryCount();
      uint16_t getPageEntryOffset();
      uint16_t getEntryIndex();
      void gotoPage(uint16_t page);
      void navNext();
      void navPrev();
      void nextPage();
      void prevPage();

      // From UI submodule
      void render();

      /**
       * Prompt the user with a menu and return the index of the selection.
       *
       * @return  The index of the selected option.
       */
      uint16_t select();

    protected:
      char title[30] = {};
      uint16_t count;
      uint16_t totalPages;
      uint16_t pageEntriesLast;

      uint16_t currentPage = 1;
      uint16_t selection = 0;
      bool rendered = false;

      char pageEntries[UI_PAGE_SIZE][UI_PAGE_ENTRY_LENTH_MAX];

      // From UI submodule
      void navigate(NavDir direction);
      void onSelectionChange();

      // Implementation
      virtual bool onConfirm() = 0;
      virtual void setup() = 0;
      virtual void onPageChange() = 0;
    };

    /**
     * @class MenuBase
     * @brief A base interface for handling menus.
     *
     * Create a base menu interface from an array of choices.
     *
     * @note
     * Available memory on embedded microcontrollers is limited. You
     * should consider using OSCR::UI::menu() instead of implementing
     * this class directly.
     */
    class MenuBase: public MenuRenderer
    {
    public:
      MenuBase(char const * menuTitle, uint8_t entryCount, MenuMode menuBaseMode, uint8_t entryLength = UI_PAGE_ENTRY_LENTH_MAX);
      MenuBase(__FlashStringHelper const * menuTitle, uint8_t entryCount, MenuMode menuBaseMode, uint8_t entryLength = UI_PAGE_ENTRY_LENTH_MAX);

    protected:
      MenuMode menuMode;
      uint8_t entryLengthMax;

      // Must be implemented in UI submodule
      void setup();
      bool onConfirm();
    };

    /**
     * @class Menu
     * @brief An interface for handling menus.
     *
     * Create a menu interface from an array of choices.
     *
     * @note
     * Available memory on embedded microcontrollers is limited. You
     * should consider using OSCR::UI::menu() instead of implementing
     * this class directly.
     */
    class Menu: public MenuBase
    {
    public:
      /**
       * @brief Create a menu interface from an array of choices.
       *
       * @param menuTitle A flash string to use for the menu title.
       * @param menuEntries An array of null-terminated flash strings.
       * @param entryCount How many entries are in `menuEntries` (usually `sizeofarray(menuEntries)`).
       */
      Menu(char const * menuTitle, char const * const menuEntries[], uint8_t entryCount);
      Menu(__FlashStringHelper const * menuTitle, char const * const menuEntries[], uint8_t entryCount);

      Menu(char const * menuTitle, __FlashStringHelper const * const * menuEntries, uint8_t entryCount);
      Menu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * const * menuEntries, uint8_t entryCount);

      /**
       * @brief Create a menu interface from an array of choices.
       *
       * @param menuTitle A flash string to use for the menu title.
       * @param menuEntries An array of null-terminated strings.
       * @param entryCount How many entries are in `menuEntries` (usually `sizeofarray(menuEntries)`).
       * @param entryLength Max length of an entry in `menuEntries`.
       */
      Menu(char const * menuTitle, char * menuEntries[], uint8_t entryCount, uint8_t entryLength = UI_PAGE_ENTRY_LENTH_MAX);
      Menu(__FlashStringHelper const * menuTitle, char * menuEntries[], uint8_t entryCount, uint8_t entryLength = UI_PAGE_ENTRY_LENTH_MAX);

    protected:
      char const * const * entries;

      // Must be implemented in UI submodule
      void onPageChange();
    };

    template<uint8_t optionCount, uint8_t optionLength>
    class MenuOptionsMenu : public MenuBase
    {
    public:
      /**
       * @brief Create a menu interface from an array of choices.
       *
       * @param menuTitle A flash string to use for the menu title.
       * @param menuEntries An instance of MenuOptions ready for use.
       */
      MenuOptionsMenu(char const * menuTitle, MenuOptions<optionCount, optionLength> & menuEntries);
      MenuOptionsMenu(__FlashStringHelper const * menuTitle, MenuOptions<optionCount, optionLength> & menuEntries);

    protected:
      __FlashStringHelper const * templateString;
      MenuOptions<optionCount, optionLength> * entries;

      // Must be implemented in UI submodule
      void onPageChange();
    };

    class FlashTemplateMenu : public MenuBase
    {
    public:
      /**
       * @brief Create a menu interface from an array of choices using a template.
       *
       * @param menuTitle A flash string to use for the menu title.
       * @param templateStr A flash string template to use for menu options.
       * @param menuEntries An array of null-terminated flash strings.
       * @param entryCount How many entries are in `menuEntries` (usually `sizeofarray(menuEntries)`).
       */
      FlashTemplateMenu(char const * menuTitle, __FlashStringHelper const * templateStr, char const * const menuEntries[], uint8_t entryCount);
      FlashTemplateMenu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * templateStr, char const * const menuEntries[], uint8_t entryCount);

    protected:
      __FlashStringHelper const * templateString;
      char const * const * entries;

      // Must be implemented in UI submodule
      void onPageChange();
    };

    template <typename T,
              OSCR::Util::if_is_integer_t<T> Enable = true>
    class IntegerTemplateMenu : public MenuBase
    {
    public:
      /**
       * @brief Create a menu interface from an array of choices using a template.
       *
       * @param menuTitle A flash string to use for the menu title.
       * @param templateStr A flash string template to use for menu options.
       * @param menuEntries An array of integers.
       * @param entryCount How many entries are in `menuEntries` (usually `sizeofarray(menuEntries)`).
       */
      IntegerTemplateMenu(char const * menuTitle, __FlashStringHelper const * templateStr, T const * menuEntries, uint8_t entryCount);
      IntegerTemplateMenu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * templateStr, T const * menuEntries, uint8_t entryCount);

    protected:
      __FlashStringHelper const * templateString;
      T const * entries;

      // Must be implemented in UI submodule
      void onPageChange();
    };

    uint16_t rangeSelect(__FlashStringHelper const * menuTitle, uint16_t const rangeMax);
    uint16_t rangeSelect(__FlashStringHelper const * menuTitle, uint16_t const rangeMin, uint16_t const rangeMax);
    uint16_t rangeSelect(__FlashStringHelper const * menuTitle, uint16_t const rangeMin, uint16_t const rangeMax, uint16_t const startAt);

    /**
     * Prompt a user to select from an array of choices and return the index of the selection.
     *
     * @param menuTitle A flash string to use for the menu title.
     * @param menuEntries An array of flash strings located in PROGMEM.
     * @param entryCount How many entries are in `menuEntries` (usually `sizeofarray(menuEntries)`).
     */
    extern uint8_t menu(__FlashStringHelper const * menuTitle, char const * const menuEntries[], uint8_t entryCount);
    extern uint8_t menu(char const * menuTitle, char const * const menuEntries[], uint8_t entryCount);

    /**
     * Prompt a user to select from an array of choices and return the index of the selection.
     *
     * @param menuTitle A flash string to use for the menu title.
     * @param menuEntries An array of flash strings located in SRAM.
     * @param entryCount How many entries are in `menuEntries` (usually `sizeofarray(menuEntries)`).
     */
    extern uint8_t menu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * const * menuEntries, uint8_t entryCount);
    extern uint8_t menu(char const * menuTitle, __FlashStringHelper const * const * menuEntries, uint8_t entryCount);

    /**
     * Prompt a user to select from an array of choices and return the index of the selection.
     *
     * @param menuTitle A flash string to use for the menu title.
     * @param menuEntries An array of null-terminated strings.
     * @param entryCount Total entries in `menuEntries` (usually `sizeofarray(menuEntries)`).
     * @param entryLength Maxinum length of an entry in `menuEntries`.
     */
    extern uint8_t menu(__FlashStringHelper const * menuTitle, char * menuEntries[], uint8_t entryCount, uint8_t entryLength = UI_PAGE_ENTRY_LENTH_MAX);
    extern uint8_t menu(char const * menuTitle, char * menuEntries[], uint8_t entryCount, uint8_t entryLength = UI_PAGE_ENTRY_LENTH_MAX);

    extern uint8_t menu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * templateStr, char const * const menuEntries[], uint8_t entryCount);
    extern uint8_t menu(char const * menuTitle, __FlashStringHelper const * templateStr, char const * const menuEntries[], uint8_t entryCount);

    template <typename T,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable = true>
    extern uint8_t menu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * templateStr, T const * menuEntries, uint8_t entryCount);

    template <typename T,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable = true>
    extern uint8_t menu(char const * menuTitle, __FlashStringHelper const * templateStr, T const * menuEntries, uint8_t entryCount);

    //! @cond
    template class IntegerTemplateMenu<uint8_t>;
    template class IntegerTemplateMenu<uint16_t>;
    //! @endcond

    /**
     * Prompt a user to select from an array of choices and return the index of the selection.
     *
     * @param menuTitle A flash string to use for the menu title.
     * @param menuEntries An instance of MenuOptions ready for use.
     */
    template<uint8_t optionCount, uint8_t optionLength>
    extern uint8_t menu(__FlashStringHelper const * menuTitle, MenuOptions<optionCount, optionLength> & menuEntries);

    template<uint8_t optionCount, uint8_t optionLength>
    extern uint8_t menu(char const * menuTitle, MenuOptions<optionCount, optionLength> & menuEntries);

    //! @cond
    template class MenuOptionsMenu< 7, 20>;
    template class MenuOptionsMenu<10, 20>;
    //! @endcond

    /**
     * Print a string using the header format.
     */
    extern void printHeader(char const * title);
    extern void printHeader(__FlashStringHelper const * title);

    /**
     * Print a string using the notification header format.
     */
    extern void printNotificationHeader(char const * title);
    extern void printNotificationHeader(__FlashStringHelper const * title);

    /**
     * Print a string using the error header format.
     */
    extern void printErrorHeader(char const * title);
    extern void printErrorHeader(__FlashStringHelper const * title);

    /**
     * Print a string using the fatal error header format.
     */
    extern void printFatalErrorHeader(char const * title);
    extern void printFatalErrorHeader(__FlashStringHelper const * title);

    /**
     * Print a number as hex.
     *
     * @param number The number to print.
     */
    template <bool prfx = true,
              typename T,
              OSCR::Util::if_is_any_unsigned_t<T> Enable = true>
    extern void printHex(T number);

    /**
     * Print a number as hex, followed by a new line.
     *
     * @param number The number to print.
     */
    template <bool prfx = true,
              typename T,
              OSCR::Util::if_is_any_unsigned_t<T> Enable = true>
    extern void printHexLine(T number);

    /**
     * Output a string to the UI.
     *
     * @param string  The string to output.
     *
     * @tparam T      A type of string
     */
    template <bool sync = false,
              typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
    extern void print(T string);

    /**
     * Output a string and a newline character to the UI.
     *
     * @param string  The string to output.
     *
     * @tparam T      A type of string
     */
    template <bool sync = false,
              typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
    extern void printLine(T string);

    /**
     * Output a string to the UI.
     *
     * @param string  The string to output.
     *
     * @tparam T      A type of string
     */
    template <typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
    inline void printSync(T string)
    {
      OSCR::UI::print<true>(string);
    }

    /**
     * Output a string and a newline character to the UI.
     *
     * @param string  The string to output.
     *
     * @tparam T      A type of string
     */
    template <typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable = true>
    inline void printLineSync(T string)
    {
      OSCR::UI::printLine<true>(string);
    }

    /**
     * Output a newline character to the UI.
     *
     * @param string  The string to output.
     */
    template <bool sync = false>
    extern void printLine(void);

    /**
     * Output a newline character to the UI.
     *
     * @param string  The string to output.
     */
    inline void printLineSync(void)
    {
      OSCR::UI::printLine<true>();
    }

    /**
     * Output a number to the UI.
     *
     * @param number  The number to output.
     * @param base    The base to display the number in.
     *
     * @tparam Tint   A type of interger
     */
    template <bool sync = false,
              typename Tint,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
    extern void print(Tint number, int base = DEC);

    /**
     * Output a number and a newline character to the UI.
     *
     * @param number  The number to output.
     * @param base    The base to display the number in.
     *
     * @tparam Tint   A type of interger
     */
    template <bool sync = false,
              typename Tint,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
    extern void printLine(Tint number, int base = DEC);

    /**
     * Output a number to the UI.
     *
     * @param number  The number to output.
     * @param base    The base to display the number in.
     *
     * @tparam Tint   A type of interger
     */
    template <bool sync = false,
              typename Tint,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
    inline void printSync(Tint number, int base = DEC)
    {
      OSCR::UI::print<true>(number, base);
    }

    /**
     * Output a number and a newline character to the UI.
     *
     * @param number  The number to output.
     * @param base    The base to display the number in.
     *
     * @tparam Tint   A type of interger
     */
    template <bool sync = false,
              typename Tint,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable = true>
    inline void printLineSync(Tint number, int base = DEC)
    {
      OSCR::UI::printLine<true>(number, base);
    }

    /**
     * Printing doubles is expensive (~1KB), so we don't do it.
     *
     * These functions are defined as deleted so the compiler error
     *  makes more sense. Without this, the compiler complains about
     *  the other templates not being a match.
     *
     * @warning DO NOT USE. THE COMPILER *WILL* ERROR.
     *
     * @deprecated
     */
    //inline void print(double number, int digits = 1) = delete;
    //inline void printSync(double number, int digits = 1) = delete;

    /**
     * Printing doubles is expensive (~1KB), so we don't do it.
     *
     * These functions are defined as deleted so the compiler error
     *  makes more sense. Without this, the compiler complains about
     *  the other templates not being a match.
     *
     * @warning DO NOT USE. THE COMPILER *WILL* ERROR.
     *
     * @deprecated
     */
    //inline void printLine(double number, int digits = 1) = delete;
    //inline void printLineSync(double number, int digits = 1) = delete;

    /**
     * Move the cursor to the start of a line relative to the current
     * line.
     *
     * @param numLines  The number of lines.
     */
    extern void setLineRel(int8_t numLines);

    /**
     * Go to the last line of the display. For outputs without a
     * fixed number of lines, such as serial, this will output one
     * new line instead.
     */
    extern void gotoLast();

    /**
     * Go to the specified line of the display. For outputs without a
     * way to change lines, such as ASCII serial, this does nothing.
     *
     * @param lineNumber  The line number.
     */
    extern void gotoLine(uint8_t lineNumber);

    /**
     * Go to the specified line of the display, counting from the
     * bottom. For outputs without a way to change lines, such as
     * ASCII serial, this prints a new line instead.
     *
     * @param lineNumber  The line number.
     */
    extern void gotoLineBottom(uint8_t lineNumber);

    /**
     * Checks if the specified line number will overlap the current
     * line. For outputs without a way to change lines, such as ASCII
     * serial, this is always `false`.
     *
     * @param lineNumber  The line number.
     */
    extern bool overlapsCurrentLine(uint8_t lineNumber);

    /**
     * Checks if the specified line number, counting from the bottom,
     * will overlap the current line. For outputs without a way to
     * change lines, such as ASCII serial, this is always `false`.
     *
     * @param lineNumber  The line number.
     */
    extern bool overlapsCurrentLineBottom(uint8_t lineNumber);

    /**
     * Internal function implemented by the UI output implementation
     * which is called when a notification occurs, before any output.
     *
     *
     * @sa notification()
     */
    extern void _notificationFormat();

    /**
     * Internal function implemented by the UI output implementation
     * which is called when a notification occurs.
     *
     * @sa notification()
     */
    extern void _notification(void);

    /**
     * Internal function implemented by the UI output implementation
     * which is called when an error occurs, before any output.
     *
     * @sa error()
     */
    extern void _errorFormat();

    /**
     * Internal function implemented by the UI output implementation
     * which is called when an error occurs.
     *
     * @sa error()
     */
    extern void _error();

    /**
     * Internal function implemented by the UI output implementation
     * which is called when a fatal error occurs, before any output.
     *
     * @sa fatalError()
     */
    extern void _fatalErrorFormat();

    /**
     * Internal function implemented by the UI output implementation
     * which is called when a fatal error occurs.
     *
     * @sa fatalError()
     */
    [[ noreturn ]] extern void _fatalError();

    /**
     * Set the notification format but don't output anything.
     */
    extern void notification(void);

    /**
     * Output a notification message and wait for input to continue.
     *
     * @param message   The message to output.
     *
     * @tparam T  Any non-numeric type supported by printLine()
     */
    template <typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true>
    extern void notification(T message);

    /**
     * Set the error format but don't output anything.
     */
    extern void error(void);

    /**
     * Output an error message and wait for input to continue.
     *
     * @param message   The message to output.
     *
     * @tparam T  Any non-numeric type supported by printLine()
     */
    template <typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true>
    extern void error(T message);

    /**
     * Set the fatal error format but don't output anything.
     */
    extern void fatalError(void);

    /**
     * Output an error message and wait for input to reset.
     *
     * @param message   The message to output.
     *
     * @returns never
     *
     * @tparam T  Any non-numeric type supported by printLine()
     */
    template <typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable = true>
    [[ noreturn ]] extern void fatalError(T message);

    /**
     * Output a fatal SD error message.
     *
     * @returns never
     */
    [[ noreturn ]] extern void ncFatalErrorStorage();

    /**
     * Clear and output a fatal SD error message.
     *
     * @returns never
     */
    [[ noreturn ]] extern void fatalErrorStorage();

    /**
     * Clear and output a fatal buffer overflow message.
     *
     * @returns never
     */
    [[ noreturn ]] extern void fatalErrorBufferOverflow();

    /**
     * Clear and output a fatal buffer overflow (name) message.
     *
     * @returns never
     */
    [[ noreturn ]] extern void fatalErrorNameOverflow();

    /**
     * Clear and output a fatal no buffer message.
     *
     * @returns never
     */
    [[ noreturn ]] extern void fatalErrorNoBuffer();

    /**
     * Clear and output a fatal invalid data message.
     *
     * @returns never
     */
    [[ noreturn ]] extern void fatalErrorInvalidData();

    /**
     * @brief Progress bar methods
     */
    namespace ProgressBar
    {
      /**
       * Current progress.
       */
      extern __constinit uint32_t current;

      /**
       * The total of the progress bar or 0 if not known.
       */
      extern __constinit uint32_t total;

      /**
       * Initilize the progress bar with the provided max/target value.
       *
       * @param maxProgress   The maximum the progress bar will reach.
       */
      extern void init(uint32_t maxProgress);

      /**
       * Initilize the progress bar offset by `lineOffset` lines with
       * the provided max/target value.
       *
       * @param maxProgress   The maximum the progress bar will reach.
       * @param maxProgress   Number of lines to offset the bar by.
       *
       * @note Not all interfaces support changing lines.
       */
      extern void init(uint32_t maxProgress, uint8_t lineOffset);

      /**
       * Advance the progress bar.
       *
       * @param steps         The amount to advance by (default: 1).
       */
      extern void advance(uint32_t steps = 1);

      /**
       * Finish the progress bar (set `current` to `total` and render it).
       */
      extern void finish();

      /**
       * Render an updated progress bar.
       */
      extern void render(bool end = false);
    }
  }
}

#endif /* OSCR_INTERFACES_H_ */
