#include "common.h"
#include "ui.h"

namespace OSCR
{
  namespace UI
  {
    void notification(void)
    {
      OSCR::UI::_notificationFormat();
    }

    template <typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable>
    void notification(T message)
    {
      OSCR::UI::_notificationFormat();
      OSCR::UI::printLine(message);
      OSCR::UI::_notification();
    }

    void error(void)
    {
      OSCR::UI::_errorFormat();
    }

    template <typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable>
    void error(T message)
    {
      OSCR::UI::_errorFormat();
      OSCR::UI::printLine(message);
      OSCR::UI::_error();
    }

    void fatalError(void)
    {
      OSCR::UI::_fatalErrorFormat();
    }

    template <typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable>
    [[ noreturn ]] void fatalError(T message)
    {
      OSCR::UI::_fatalErrorFormat();
      OSCR::UI::printLine(message);
      OSCR::UI::_fatalError();
    }

    [[ noreturn ]] void ncFatalErrorStorage()
    {
      fatalError(FS(OSCR::Strings::Errors::StorageError));
    }

    [[ noreturn ]] void fatalErrorStorage()
    {
      printFatalErrorHeader(FS(OSCR::Strings::Headings::FatalError));
      ncFatalErrorStorage();
    }

    [[ noreturn ]] void fatalErrorBufferOverflow()
    {
      printFatalErrorHeader(FS(OSCR::Strings::Headings::FatalError));
      printLine(FS(OSCR::Strings::Errors::BuffereOverflow));
      _fatalError();
    }

    [[ noreturn ]] void fatalErrorNameOverflow()
    {
      printFatalErrorHeader(FS(OSCR::Strings::Headings::FatalError));
      printLine(FS(OSCR::Strings::Errors::BuffereOverflow));
      printLine(FS(OSCR::Strings::Errors::NameOverflow));
      _fatalError();
    }

    [[ noreturn ]] void fatalErrorNoBuffer()
    {
      printFatalErrorHeader(FS(OSCR::Strings::Headings::FatalError));
      printLine(F("No buffer assigned."));
      _fatalError();
    }

    [[ noreturn ]] void fatalErrorInvalidData()
    {
      printFatalErrorHeader(FS(OSCR::Strings::Headings::FatalError));
      printLine(F("Invalid data encountered."));
      _fatalError();
    }

    template <bool prfx,
              typename T,
              OSCR::Util::if_is_any_unsigned_t<T> Enable>
    void printHex(T number)
    {
      uint8_t const kBuffSize = (sizeof(T) * 2) + (prfx * 2) + 1;
      char buff[kBuffSize];

      OSCR::Util::toHex<prfx>(buff, kBuffSize, number);

      print(buff);
    }

    template <bool prfx,
              typename T,
              OSCR::Util::if_is_any_unsigned_t<T> Enable>
    void printHexLine(T number)
    {
      printHex<prfx>(number);
      printLine();
    }

    RangeSelect::RangeSelect(__FlashStringHelper const * menuTitle, uint16_t const rangeMin, uint16_t const rangeMax): title(menuTitle), minValue(rangeMin), maxValue(rangeMax)
    {
      _setup();
    }

    uint8_t RangeSelect::getPositionsFor(uint16_t value)
    {
      if (value >= 10000) return 5;
      else if (value >= 1000) return 4;
      else if (value >= 100) return 3;
      else if (value >= 10) return 2;
      else return 1;
    }

    void RangeSelect::_setup()
    {
      positions = getPositionsFor(maxValue);
    }

    int8_t RangeSelect::getNumberAt(uint16_t value, uint8_t index)
    {
      if (value == 0) return (index == 4) ? 0 : -1;

      uint32_t tens = OSCR::Util::power<10>((uint32_t)(4 - index));

      if (value < tens) return -1;

      int number = (value % (tens * 10)) / tens;

      return number;
    }

    int32_t RangeSelect::diffValue(bool willRollover, int32_t tens, int8_t rolloverNumber, int8_t step)
    {
      if (currentPosition == 0) return willRollover ? rolloverNumber : step;
      return willRollover ? tens * rolloverNumber : tens * step;
    }

    bool RangeSelect::valueInBounds(uint32_t newValue)
    {
      if (newValue < minValue) return false;
      if (newValue > maxValue) return false;
      return true;
    }

    bool RangeSelect::adjustValue(bool willRollover, int32_t tens, int8_t rolloverNumber, int8_t step)
    {
      int32_t newValue = currentValue + diffValue(willRollover, tens, rolloverNumber, step);

      if (!valueInBounds(newValue)) return false;

      currentValue = newValue;

      return true;
    }

    void RangeSelect::valueDecrement()
    {
      uint8_t const index = 4 - currentPosition;
      int32_t const tens = OSCR::Util::power<10>(currentPosition);

      int8_t originalNumber = getNumberAt(currentValue, index);
      if (originalNumber < 0) originalNumber = 0;

      bool const willRollover = ((originalNumber == 0) || ((currentValue - tens) < minValue));

      if (adjustValue(willRollover, tens, 9, -1)) return;

      if (willRollover)
      {
        int8_t minNumberAt = getNumberAt(minValue, index);
        int8_t maxNumberAt = getNumberAt(maxValue, index);

        if ((minNumberAt < 1) && (maxNumberAt < 1)) return;

        int8_t minNumber = min(minNumberAt, maxNumberAt);
        int8_t maxNumber = max(minNumberAt, maxNumberAt);

        if (maxNumber > 0)
        {
          if (adjustValue(willRollover, tens, maxNumber, -1)) return;

          maxNumber = maxNumber - 1;

          if ((maxNumber > 0) && (adjustValue(willRollover, tens, maxNumber, -1))) return;
        }

        if ((minNumber < 1) || (minNumber < maxNumber)) return;
        if (adjustValue(willRollover, tens, minNumber, -1) || (minNumber == 1)) return;

        minNumber -= 1;

        adjustValue(willRollover, tens, minNumber, -1);
      }
    }

    void RangeSelect::valueIncrement()
    {
      uint8_t const index = 4 - currentPosition;
      int32_t const tens = OSCR::Util::power<10>(currentPosition);

      int8_t originalNumber = getNumberAt(currentValue, index);
      if (originalNumber < 0) originalNumber = 0;

      bool willRollover = ((originalNumber == 9) || ((currentValue + tens) > maxValue));

      if (adjustValue(willRollover, tens, -originalNumber, 1)) return;

      if (willRollover)
      {
        int8_t minNumberAt = getNumberAt(minValue, index);
        int8_t maxNumberAt = getNumberAt(maxValue, index);

        if ((minNumberAt < 0) && (maxNumberAt < 0)) return;

        if (minNumberAt < 0) minNumberAt = 0;

        int8_t minNumber = min(minNumberAt, maxNumberAt);
        int8_t maxNumber = max(minNumberAt, maxNumberAt);

        minNumber = ((originalNumber > 0) ? (originalNumber - minNumber) : minNumber);
        maxNumber = ((originalNumber > 0) ? (originalNumber - maxNumber) : maxNumber);

        if (minNumber > 0)
        {
          if (adjustValue(willRollover, tens, -minNumber, 1)) return;

          minNumber -= 1;

          if ((minNumber > 0) && adjustValue(willRollover, tens, -minNumber, 1)) return;
        }

        if ((maxNumber < 1) || (maxNumber > minNumber)) return;
        if (adjustValue(willRollover, tens, -maxNumber, 1) || (maxNumber == 1)) return;

        maxNumber -= 1;

        adjustValue(willRollover, tens, -maxNumber, 1);
      }
    }

    uint16_t rangeSelect(__FlashStringHelper const * menuTitle, uint16_t const rangeMin, uint16_t const rangeMax, uint16_t const startAt)
    {
      OSCR::UI::RangeSelect selector = OSCR::UI::RangeSelect(menuTitle, rangeMin, rangeMax);
      uint16_t selection = selector.select(startAt);
      return selection;
    }

    uint16_t rangeSelect(__FlashStringHelper const * menuTitle, uint16_t const rangeMin, uint16_t const rangeMax)
    {
      return rangeSelect(menuTitle, rangeMin, rangeMax, rangeMin);
    }

    uint16_t rangeSelect(__FlashStringHelper const * menuTitle, uint16_t const rangeMax)
    {
      return rangeSelect(menuTitle, 0, rangeMax);
    }

    MenuBase::MenuBase(char const * menuTitle, uint8_t entryCount, MenuMode menuBaseMode, uint8_t entryLength)
      : MenuRenderer(menuTitle, entryCount),
        menuMode(menuBaseMode),
        entryLengthMax(OSCR::Util::clamp(entryLength, UI_PAGE_ENTRY_LENTH_MIN, UI_PAGE_ENTRY_LENTH_MAX))
    {
      // ...
    }

    MenuBase::MenuBase(__FlashStringHelper const * menuTitle, uint8_t entryCount, MenuMode menuBaseMode, uint8_t entryLength)
      : MenuRenderer(menuTitle, entryCount),
        menuMode(menuBaseMode),
        entryLengthMax(OSCR::Util::clamp(entryLength, UI_PAGE_ENTRY_LENTH_MIN, UI_PAGE_ENTRY_LENTH_MAX))
    {
      // ...
    }

    Menu::Menu(char const * menuTitle, char * menuEntries[], uint8_t entryCount, uint8_t entryLength)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuNullChar, entryLength),
        entries(&*menuEntries)
    {
      setup();
    }

    Menu::Menu(__FlashStringHelper const * menuTitle, char * menuEntries[], uint8_t entryCount, uint8_t entryLength)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuNullChar, entryLength),
        entries(&*menuEntries)
    {
      setup();
    }

    Menu::Menu(char const * menuTitle, char const * const menuEntries[], uint8_t entryCount)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuFlashString),
        entries(&*menuEntries)
    {
      setup();
    }

    Menu::Menu(__FlashStringHelper const * menuTitle, char const * const menuEntries[], uint8_t entryCount)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuFlashString),
        entries(&*menuEntries)
    {
      setup();
    }

    Menu::Menu(char const * menuTitle, __FlashStringHelper const * const * menuEntries, uint8_t entryCount)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuRamFlashString),
        entries(reinterpret_cast<char const * const *>(&*menuEntries))
    {
      setup();
    }

    Menu::Menu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * const * menuEntries, uint8_t entryCount)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuRamFlashString),
        entries(reinterpret_cast<char const * const *>(&*menuEntries))
    {
      setup();
    }

    FlashTemplateMenu::FlashTemplateMenu(char const * menuTitle, __FlashStringHelper const * templateStr, char const * const menuEntries[], uint8_t entryCount)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuFlashStringTemplate),
        templateString(templateStr),
        entries(&*menuEntries)
    {
      setup();
    }

    FlashTemplateMenu::FlashTemplateMenu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * templateStr, char const * const menuEntries[], uint8_t entryCount)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuFlashStringTemplate),
        templateString(templateStr),
        entries(&*menuEntries)
    {
      setup();
    }

    template <typename T,
              OSCR::Util::if_is_integer_t<T> Enable>
    IntegerTemplateMenu<T, Enable>::IntegerTemplateMenu(char const * menuTitle, __FlashStringHelper const * templateStr, T const * menuEntries, uint8_t entryCount)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuIntegerTemplate),
        templateString(templateStr),
        entries(menuEntries)
    {
      setup();
    }

    template <typename T,
              OSCR::Util::if_is_integer_t<T> Enable>
    IntegerTemplateMenu<T, Enable>::IntegerTemplateMenu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * templateStr, T const * menuEntries, uint8_t entryCount)
      : MenuBase(menuTitle, entryCount, MenuMode::kMenuIntegerTemplate),
        templateString(templateStr),
        entries(menuEntries)
    {
      setup();
    }

    template<uint8_t optionCount, uint8_t optionLength>
    MenuOptionsMenu<optionCount, optionLength>::MenuOptionsMenu(char const * menuTitle, MenuOptions<optionCount, optionLength> & menuEntries)
      : MenuBase(menuTitle, min((uint8_t)menuEntries.count, optionCount), MenuMode::kMenuOptionStruct, optionLength),
        entries(&menuEntries)
    {
      setup();
    }

    template<uint8_t optionCount, uint8_t optionLength>
    MenuOptionsMenu<optionCount, optionLength>::MenuOptionsMenu(__FlashStringHelper const * menuTitle, MenuOptions<optionCount, optionLength> & menuEntries)
      : MenuBase(menuTitle, min((uint8_t)menuEntries.count, optionCount), MenuMode::kMenuOptionStruct, optionLength),
        entries(&menuEntries)
    {
      setup();
    }

    uint8_t menu(__FlashStringHelper const * menuTitle, char * menuEntries[], uint8_t entryCount, uint8_t entryLength)
    {
      OSCR::UI::Menu UserMenu = OSCR::UI::Menu(menuTitle, menuEntries, entryCount, entryLength);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    uint8_t menu(char const * menuTitle, char * menuEntries[], uint8_t entryCount, uint8_t entryLength)
    {
      OSCR::UI::Menu UserMenu = OSCR::UI::Menu(menuTitle, menuEntries, entryCount, entryLength);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    uint8_t menu(__FlashStringHelper const * menuTitle, char const * const menuEntries[], uint8_t entryCount)
    {
      OSCR::UI::Menu UserMenu = OSCR::UI::Menu(menuTitle, menuEntries, entryCount);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    uint8_t menu(char const * menuTitle, char const * const menuEntries[], uint8_t entryCount)
    {
      OSCR::UI::Menu UserMenu = OSCR::UI::Menu(menuTitle, menuEntries, entryCount);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    uint8_t menu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * const * menuEntries, uint8_t entryCount)
    {
      OSCR::UI::Menu UserMenu = OSCR::UI::Menu(menuTitle, menuEntries, entryCount);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    uint8_t menu(char const * menuTitle, __FlashStringHelper const * const * menuEntries, uint8_t entryCount)
    {
      OSCR::UI::Menu UserMenu = OSCR::UI::Menu(menuTitle, menuEntries, entryCount);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    uint8_t menu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * templateStr, char const * const menuEntries[], uint8_t entryCount)
    {
      OSCR::UI::FlashTemplateMenu UserMenu = OSCR::UI::FlashTemplateMenu(menuTitle, templateStr, menuEntries, entryCount);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    uint8_t menu(char const * menuTitle, __FlashStringHelper const * templateStr, char const * const menuEntries[], uint8_t entryCount)
    {
      OSCR::UI::FlashTemplateMenu UserMenu = OSCR::UI::FlashTemplateMenu(menuTitle, templateStr, menuEntries, entryCount);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    template <typename T,
              OSCR::Util::if_is_integer_t<T> Enable>
    uint8_t menu(__FlashStringHelper const * menuTitle, __FlashStringHelper const * templateStr, T const * menuEntries, uint8_t entryCount)
    {
      OSCR::UI::IntegerTemplateMenu UserMenu = OSCR::UI::IntegerTemplateMenu(menuTitle, templateStr, menuEntries, entryCount);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    template <typename T,
              OSCR::Util::if_is_integer_t<T> Enable>
    uint8_t menu(char const * menuTitle, __FlashStringHelper const * templateStr, T const * menuEntries, uint8_t entryCount)
    {
      OSCR::UI::IntegerTemplateMenu UserMenu = OSCR::UI::IntegerTemplateMenu(menuTitle, templateStr, menuEntries, entryCount);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    template<uint8_t optionCount, uint8_t optionLength>
    uint8_t menu(__FlashStringHelper const * menuTitle, MenuOptions<optionCount, optionLength> & menuEntries)
    {
      OSCR::UI::MenuOptionsMenu UserMenu = OSCR::UI::MenuOptionsMenu<optionCount, optionLength>(menuTitle, menuEntries);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    template<uint8_t optionCount, uint8_t optionLength>
    uint8_t menu(char const * menuTitle, MenuOptions<optionCount, optionLength> & menuEntries)
    {
      OSCR::UI::MenuOptionsMenu UserMenu = OSCR::UI::MenuOptionsMenu<optionCount, optionLength>(menuTitle, menuEntries);

      uint8_t menuSelection = UserMenu.select();

      return menuSelection;
    }

    MenuRenderer::MenuRenderer(char const * menuTitle)
      : count(0),
        totalPages(0),
        pageEntriesLast(0)
    {
      OSCR::Util::copyStr(this->title, sizeof(title), menuTitle);
    }

    MenuRenderer::MenuRenderer(__FlashStringHelper const * menuTitle)
      : count(0),
        totalPages(0),
        pageEntriesLast(0)
    {
      OSCR::Util::copyStr_P(this->title, sizeof(title), menuTitle);
    }

    MenuRenderer::MenuRenderer(char const * menuTitle, uint16_t entryCount)
      : count(entryCount),
        totalPages(max(1, (entryCount / UI_PAGE_SIZE + (entryCount%UI_PAGE_SIZE != 0)))),
        pageEntriesLast(entryCount - ((totalPages - 1) * UI_PAGE_SIZE))
    {
      OSCR::Util::copyStr(this->title, sizeof(title), menuTitle);
    }

    MenuRenderer::MenuRenderer(__FlashStringHelper const * menuTitle, uint16_t entryCount)
      : count(entryCount),
        totalPages(max(1, (entryCount / UI_PAGE_SIZE + (entryCount%UI_PAGE_SIZE != 0)))),
        pageEntriesLast(entryCount - ((totalPages - 1) * UI_PAGE_SIZE))
    {
      OSCR::Util::copyStr_P(this->title, sizeof(title), menuTitle);
    }

    uint16_t MenuRenderer::getPage()
    {
      return currentPage;
    }

    uint16_t MenuRenderer::getPageCount()
    {
      return totalPages;
    }

    bool MenuRenderer::isLast()
    {
      return (currentPage == totalPages);
    }

    uint16_t MenuRenderer::getPageEntryCount()
    {
      return isLast() ? pageEntriesLast : UI_PAGE_SIZE;
    }

    uint16_t MenuRenderer::getPageEntryOffset()
    {
      return (getPage() - 1) * UI_PAGE_SIZE;
    }

    uint16_t MenuRenderer::getEntryIndex()
    {
      return ((currentPage - 1) * UI_PAGE_SIZE) + selection;
    }

    void MenuRenderer::navNext()
    {
      uint16_t pageSize = getPageEntryCount();

      ++selection;

      if (selection == pageSize)
      {
        selection = 0;
        nextPage();
      }

      onSelectionChange();
    }

    void MenuRenderer::nextPage()
    {
        if (currentPage == totalPages) return gotoPage(1);
        else return gotoPage(currentPage+1);
    }

    void MenuRenderer::navPrev()
    {
      if (selection == 0)
      {
        prevPage();
        selection = getPageEntryCount() - 1;
      }
      else
      {
        --selection;
      }

      onSelectionChange();
    }

    void MenuRenderer::prevPage()
    {
      if (currentPage == 1) return gotoPage(totalPages);
      else return gotoPage(currentPage-1);
    }

    void MenuRenderer::gotoPage(uint16_t page)
    {
      currentPage = page;
      rendered = false;
      onPageChange();
    }

    /**
     * Toggle the LED present on some hardware versions
     */
    void blinkLED()
    {
#if defined(ENABLE_VSELECT)
      // Nothing
#elif defined(HW5)
      // 3mm LED on D38, front of PCB
      PORTD ^= (1 << 7);
#elif defined(ENABLE_OLED)
      // 5mm LED on D10, above SD slot
      PORTB ^= (1 << 4);
#elif (HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864)  // HW4
      // TX LED on D1, build-in
      PORTE ^= (1 << 1);
#elif defined(ENABLE_SERIAL)
      // 5mm LED on D10, above SD slot (HW3)
      PORTB ^= (1 << 4);  //HW4/HW5 LCD RST connects there now too
      // 3mm LED on D38, front of PCB (HW5)
      PORTB ^= (1 << 7);
#endif
    }

    namespace ProgressBar
    {
      __constinit uint32_t current = 0;
      __constinit uint32_t total = 0;
    }
  }
}

//! @cond
template void OSCR::UI::notification<__FlashStringHelper const *>(__FlashStringHelper const *);
template void OSCR::UI::notification<String const &>(String const &);
template void OSCR::UI::notification<char const[]>(char const[]);
template void OSCR::UI::notification<char *>(char *);
template void OSCR::UI::notification<char const *>(char const *);

template void OSCR::UI::error<__FlashStringHelper const *>(__FlashStringHelper const *);
template void OSCR::UI::error<String const &>(String const &);
template void OSCR::UI::error<char const[]>(char const[]);
template void OSCR::UI::error<char *>(char *);
template void OSCR::UI::error<char const *>(char const *);

template void OSCR::UI::fatalError<__FlashStringHelper const *>(__FlashStringHelper const *);
template void OSCR::UI::fatalError<String const &>(String const &);
template void OSCR::UI::fatalError<char const[]>(char const[]);
template void OSCR::UI::fatalError<char *>(char *);
template void OSCR::UI::fatalError<char const *>(char const *);

template void OSCR::UI::printHex<true >(uint8_t number);
template void OSCR::UI::printHex<false>(uint8_t number);
template void OSCR::UI::printHex<true >(uint16_t number);
template void OSCR::UI::printHex<false>(uint16_t number);
template void OSCR::UI::printHex<true >(uint32_t number);
template void OSCR::UI::printHex<false>(uint32_t number);
template void OSCR::UI::printHex<true >(uint64_t number);
template void OSCR::UI::printHex<false>(uint64_t number);

template void OSCR::UI::printHexLine<true >(uint8_t number);
template void OSCR::UI::printHexLine<false>(uint8_t number);
template void OSCR::UI::printHexLine<true >(uint16_t number);
template void OSCR::UI::printHexLine<false>(uint16_t number);
template void OSCR::UI::printHexLine<true >(uint32_t number);
template void OSCR::UI::printHexLine<false>(uint32_t number);
template void OSCR::UI::printHexLine<true >(uint64_t number);
template void OSCR::UI::printHexLine<false>(uint64_t number);

template uint8_t OSCR::UI::menu(__FlashStringHelper const *, __FlashStringHelper const *, uint8_t const *, uint8_t);
template uint8_t OSCR::UI::menu(__FlashStringHelper const *, __FlashStringHelper const *, uint16_t const *, uint8_t);
template uint8_t OSCR::UI::menu(char const *, __FlashStringHelper const *, uint8_t const *, uint8_t);
template uint8_t OSCR::UI::menu(char const *, __FlashStringHelper const *, uint16_t const *, uint8_t);

template uint8_t OSCR::UI::menu(__FlashStringHelper const *, OSCR::UI::MenuOptions<10, 20>&);
template uint8_t OSCR::UI::menu(__FlashStringHelper const *, OSCR::UI::MenuOptions< 7, 20>&);
template uint8_t OSCR::UI::menu(char const *, OSCR::UI::MenuOptions<10, 20>&);
template uint8_t OSCR::UI::menu(char const *, OSCR::UI::MenuOptions< 7, 20>&);
//! @endcond
