#include "common.h"

#if (HARDWARE_OUTPUT_TYPE == OUTPUT_SSD1306)

# include <U8g2lib.h>
# include "common/Updater.h"
# include "ui/interfaces.h"
# include "ui/l10n.h"
# include "ui/interfaces/ssd1306.h"
# include "hardware/inputs/button.h"

namespace OSCR
{
  namespace UI
  {
    //! @cond
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

# if (HARDWARE_INPUT_TYPE == INPUT_2BUTTON)
    Button selButton(PING, PING2);
# endif /* INPUT_2BUTTON */
    Button navButton(PIND, PIND7);
    //! @endcond

    void setup()
    {
      display.begin();
      display.setFont(u8g2_font_haxrcorp4089_tr);
    }

    template <uint8_t y = 0>
    void setCursorX(uint8_t x)
    {
      display.setCursor(x, y);
    }

    template <uint8_t x = 0>
    void setCursorY(uint8_t y)
    {
      display.setCursor(x, y);
    }

    template <uint8_t x = 0, uint8_t y = 0>
    void setCursorXY(void)
    {
      display.setCursor(x, y);
    }

    uint8_t _nextSpace(char const * const text)
    {
      char * spaceAt = strchr(text, ' ');

      if (spaceAt == nullptr) return strlen(text);

      return (spaceAt - text);
    }

    uint8_t printText(char const * const text)
    {
      constexpr uint8_t const kBuffSize = 20;
      constexpr uint8_t const kBuffMaxLen = kBuffSize - 1;
      char pbuff[kBuffSize] = " ";

      size_t len = strlen(text);
      size_t remainingLen = len;

      uint8_t lines = 0;
      uint8_t remainingWidth = kDisplayWidth - display.tx;
      uint8_t const spaceWidth = OSCR::UI::display.getStrWidth(pbuff);

      for (size_t i = 0; i < len;)
      {
        uint8_t wordLen = _nextSpace(text + i);

        if (wordLen > kBuffMaxLen)
        {
          lines++;
          printLine(pbuff);
          return lines;
        }

        strncpy(pbuff, text + i, wordLen);
        pbuff[wordLen] = '\0';

        uint8_t wordWidth = OSCR::UI::display.getStrWidth(pbuff);

        if (wordWidth > kDisplayWidth)
        {
          if (remainingWidth != kDisplayWidth)
          {
            lines++;
            printLine();
            remainingWidth = kDisplayWidth;
          }
        }
        else if (wordWidth > remainingWidth)
        {
          lines++;
          printLine();
          remainingWidth = kDisplayWidth;
        }

        print(pbuff);

        remainingLen -= (wordLen);
        remainingWidth -= (wordWidth);

        i += wordLen + 1;

        if (spaceWidth >= remainingWidth)
        {
          lines++;
          printLine();
          remainingWidth = kDisplayWidth;
          if (remainingLen > 0) remainingLen -= 1;
          continue;
        }

        if (remainingLen > 0)
        {
          remainingWidth -= spaceWidth;
          remainingLen -= 1;
          print(FS(OSCR::Strings::Symbol::Space));
        }
      }

      lines++;
      printLine();

      return lines;
    }

    void printCenter(char const * text)
    {
      uint8_t currentY = display.ty;
      uint8_t centeredX = ((kDisplayWidth - (OSCR::UI::display.getStrWidth(text))) / 2);
      display.setCursor(centeredX, currentY);
      print(text);
    }

    void printCenter_P(char const * flashText)
    {
      char text[50];
      OSCR::Util::copyStr_P(text, 50, flashText);
      printCenter(text);
    }

    void printRight(char const * text)
    {
      uint8_t currentY = display.ty;
      uint8_t centeredX = (kDisplayWidth - OSCR::UI::display.getStrWidth(text));
      display.setCursor(centeredX, currentY);
      print(text);
    }

    void printRight_P(char const * flashText)
    {
      char text[50];
      OSCR::Util::copyStr_P(text, 50, flashText);
      printRight(text);
    }

    void setLineRel(int8_t numLines)
    {
      uint8_t prevLine = OSCR::Util::maxOf<uint8_t>((uint8_t)0, (uint8_t)(display.ty + (numLines * kLineHeight)));
      setCursorY<0>(prevLine);
    }

    /**
     * Display
     */

    void gotoLast()
    {
      setCursorY<0>(kDisplayHeight);
    }

    void gotoLine(uint8_t lineNumber)
    {
      setCursorY<0>(lineNumber * kLineHeight);
    }

    void gotoLineBottom(uint8_t lineNumber)
    {
      setCursorY<0>((kDisplayLines - (lineNumber - 1)) * kLineHeight);
    }

    bool overlapsPreviousLine(uint8_t lineNumber)
    {
      // If current position is the first or second line, it's always true.
      if (display.ty <= (kLineHeight * 2)) return true;

      uint8_t testY = lineNumber * kLineHeight;

      return (testY <= display.ty);
    }

    bool overlapsPreviousLineBottom(uint8_t lineNumber)
    {
      uint8_t prevLine = OSCR::Util::maxOf<uint8_t>((uint8_t)0, (uint8_t)(display.ty - kLineHeight));
      uint8_t newY = (kDisplayLines - lineNumber) * kLineHeight;
      return (newY <= prevLine);
    }

    bool overlapsCurrentLine(uint8_t lineNumber)
    {
      uint8_t newY = (lineNumber + 1) * kLineHeight;
      return (newY <= display.ty);
    }

    bool overlapsCurrentLineBottom(uint8_t lineNumber)
    {
      uint8_t newY = (kDisplayLines - lineNumber) * kLineHeight;
      return (newY <= display.ty);
    }

    void update()
    {
      display.updateDisplay();
      OSCR::busy();
    }

    /**
     * @cond
     * Clear the display using a slower but sometimes more reliable method.
     */
    void clearSlow()
    {
      display.setDrawColor(0);
      display.drawBox(0, 0, kDisplayWidth, kDisplayHeight);
      display.setDrawColor(1);
      setCursorXY<0, 8>();
    }

    void clear()
    {
      display.clearDisplay();
      setCursorXY<0, 8>();
    }

    void clearLine()
    {
      u8g2_uint_t curY = display.ty;

      display.setDrawColor(0);
      display.drawBox(0, curY - kLineHeight, kDisplayWidth, kLineHeight);
      display.setDrawColor(1);

      setCursorY<0>(curY);
    }

    /**
     * Input
     */

    UserInput checkButton()
    {
      OSCR::tick();

# if (HARDWARE_INPUT_TYPE == INPUT_2BUTTON)
      InterfaceInput const selection = selButton.check();
      switch (selection)
      {
        case InterfaceInput::kInterfaceInputPress: return UserInput::kUserInputConfirm;
        case InterfaceInput::kInterfaceInputPressShort: return UserInput::kUserInputConfirmShort;
        case InterfaceInput::kInterfaceInputPressLong: return UserInput::kUserInputConfirmLong;
        default: break;
      }
# endif /* INPUT_2BUTTON */

      InterfaceInput const navigation = navButton.check();
      switch (navigation)
      {
        case InterfaceInput::kInterfaceInputPress: return UserInput::kUserInputNext;
        case InterfaceInput::kInterfaceInputPressDouble: return UserInput::kUserInputBack;
        case InterfaceInput::kInterfaceInputPressShort: return UserInput::kUserInputConfirm;
        case InterfaceInput::kInterfaceInputPressLong: return UserInput::kUserInputConfirmLong;
        default: break;
      }

      return UserInput::kUserInputIgnore;
    }

    /**
     * Menus/Prompts
     */

    uint16_t RangeSelect::select(uint16_t const startAt)
    {
      this->currentValue = OSCR::Util::clamp(startAt, minValue, maxValue);

      if (!rendered)
        this->render();

      bool confirmed = false;

      while (!confirmed)
      {
        UserInput userInput = waitInput();

        switch (userInput)
        {
        case UserInput::kUserInputBack:
          // Decrease selected number by 1
          valueDecrement();

          //if (!rendered)
          this->render();

          continue;

        case UserInput::kUserInputNext:
          // Increase selected number by 1
          valueIncrement();

          //if (!rendered)
          this->render();

          continue;

        case UserInput::kUserInputConfirmShort:
        case UserInput::kUserInputConfirmLong:
          // Go back to previous selection
          if (currentPosition > 0) this->currentPosition -= 1;

          this->render();

          continue;

        case UserInput::kUserInputConfirm:
          // Go to next selection unless at end
          this->currentPosition += 1;

          // If this position is valid continue with selection
          if (this->currentPosition < this->positions)
          {
            this->render();
            continue;
          }

          // Set position back to 0 and mark confirmed
          this->currentPosition = 0;

          confirmed = true;

          continue;

        case UserInput::kUserInputIgnore:
        case UserInput::kUserInputUnknown:
          continue;
        }
      }

      clear();

      return this->currentValue;
    }

    uint16_t RangeSelect::select()
    {
      return this->select(minValue);
    }

    void RangeSelect::render()
    {

      printHeader(this->title);

      print(FS(OSCR::Strings::Symbol::Space));

      char drawStr[2] = "0";
      uint8_t const strFixedWidth = display.getStrWidth(drawStr) + 2;
      uint8_t strOffset = 0;

      for (uint8_t drawPos = (5 - this->positions); drawPos < 5; drawPos++)
      {
        uint8_t drawIndex = 4 - drawPos;
        int8_t drawDigit = getNumberAt(this->currentValue, drawPos);

        if (drawDigit < 0) drawDigit = 0;

        snprintf_P(drawStr, sizeof(drawStr), PSTR("%ld"), (long)drawDigit);

        display.setDrawColor(1);

        if (drawIndex == currentPosition)
        {
          display.drawBox(
            display.tx,
            display.ty - display.getAscent(),
            strFixedWidth,
            kLineHeight
          );
          display.setDrawColor(0);
        }

        strOffset = OSCR::Util::maxOf<uint8_t>((uint8_t)1, (uint8_t)((strFixedWidth - display.getStrWidth(drawStr)) / 2));

        display.setCursor(display.tx + strOffset, display.ty);

        print(drawStr);

        display.setCursor(display.tx + strOffset, display.ty);

        display.setDrawColor(1);
      }

      printLine();

      display.updateDisplay();

      this->rendered = true;
    }

    void MenuRenderer::render()
    {
      printHeader(this->title);

      for (uint_fast8_t i = 0; i < getPageEntryCount(); i++)
      {
        // Add space for the selection dot
        print(FS(OSCR::Strings::Symbol::MenuSpaces));

        // Print menu item
        printLine(pageEntries[i]);
      }

      // Draw selection indicator
      display.drawBox(1, 8 * selection + 11, 3, 3);

      // Draw a scroll bar when there are multiple pages
      if (totalPages > 1)
      {
        // Calculate scroll bar height/position
        u8g2_uint_t barPageHeight = OSCR::Util::maxOf<uint16_t>((uint16_t)1, (uint16_t)((kDisplayHeight - 8) / totalPages));
        u8g2_uint_t barPagePosition = ((currentPage - 1) * ((kDisplayHeight - 8) / totalPages)) + 8;

        // Draw scroll bar
        display.drawBox(kDisplayWidth - 1, barPagePosition, kDisplayWidth, barPageHeight);
      }

      display.updateDisplay();

      this->rendered = true;
    }

    uint16_t MenuRenderer::select()
    {
      if (!rendered) this->render();

      bool confirmed = false;

      do
      {
        switch (waitInput())
        {
        case UserInput::kUserInputBack:
          this->navigate(NavDir::kNavDirBackward);
          if (!rendered) this->render();
          continue;

        case UserInput::kUserInputNext:
          this->navigate(NavDir::kNavDirForward);
          if (!rendered) this->render();
          continue;

        case UserInput::kUserInputConfirm:
        case UserInput::kUserInputConfirmShort:
        case UserInput::kUserInputConfirmLong:
          if (!onConfirm()) continue;

          confirmed = true;

          continue;

        case UserInput::kUserInputIgnore:
        case UserInput::kUserInputUnknown:
          continue;
        }
      }
      while (!confirmed);

      clear();

      return this->getEntryIndex();
    }

    void MenuRenderer::navigate(NavDir direction)
    {
      // Remove selection indicator
      display.setDrawColor(0);
      display.drawBox(1, 8 * selection + 11, 3, 3);
      display.setDrawColor(1);
      display.updateDisplay();

      switch (direction)
      {
      case NavDir::kNavDirBackward: return navPrev();
      case NavDir::kNavDirForward: return navNext();
      }
    }

    void MenuRenderer::onSelectionChange()
    {
      // Add selection indicator
      display.drawBox(1, 8 * selection + 11, 3, 3);
      display.updateDisplay();
    }

    void MenuBase::setup()
    {
      onPageChange();
    }

    bool MenuBase::onConfirm()
    {
      return true;
    }

    template <typename T,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<T>::value, bool> Enable>
    void IntegerTemplateMenu<T, Enable>::onPageChange()
    {
      uint8_t indexOffset = getPageEntryOffset();

      for (uint_fast8_t menuIndex = 0; menuIndex < getPageEntryCount(); ++menuIndex)
      {
        uint8_t entryIndex = indexOffset + menuIndex;

        OSCR::Util::applyTemplate_P(pageEntries[menuIndex], UI_PAGE_ENTRY_LENTH_MAX, templateString, (int32_t)entries[entryIndex]);
      }
    }

    void FlashTemplateMenu::onPageChange()
    {
      uint8_t indexOffset = getPageEntryOffset();

      for (uint_fast8_t menuIndex = 0; menuIndex < getPageEntryCount(); ++menuIndex)
      {
        uint8_t entryIndex = indexOffset + menuIndex;

        OSCR::Util::applyTemplate_P(pageEntries[menuIndex], UI_PAGE_ENTRY_LENTH_MAX, templateString, entries[entryIndex]);
      }
    }

    template<uint8_t optionCount, uint8_t optionLength>
    void MenuOptionsMenu<optionCount, optionLength>::onPageChange()
    {
      uint8_t indexOffset = getPageEntryOffset();

      for (uint_fast8_t menuIndex = 0; menuIndex < getPageEntryCount(); ++menuIndex)
      {
        uint8_t entryIndex = indexOffset + menuIndex;

        strlcpy(pageEntries[menuIndex], entries->options[entryIndex], min(entryLengthMax, UI_PAGE_ENTRY_LENTH_MAX));
      }
    }

    void Menu::onPageChange()
    {
      uint8_t indexOffset = getPageEntryOffset();

      for (uint_fast8_t menuIndex = 0; menuIndex < getPageEntryCount(); ++menuIndex)
      {
        uint8_t entryIndex = indexOffset + menuIndex;

        switch (menuMode)
        {
        case MenuMode::kMenuFlashString:
          strlcpy_P(pageEntries[menuIndex], FP(entries[entryIndex]), UI_PAGE_ENTRY_LENTH_MAX);
          break;

        case MenuMode::kMenuRamFlashString:
          strlcpy_P(pageEntries[menuIndex], entries[entryIndex], UI_PAGE_ENTRY_LENTH_MAX);
          break;

        case MenuMode::kMenuNullChar:
          strlcpy(pageEntries[menuIndex], entries[entryIndex], UI_PAGE_ENTRY_LENTH_MAX);
          break;

        default:
          break;
        }
      }
    }

    /**
     * Messages
     */
    void __printHeader_beforePrint()
    {
      clear();

      display.setDrawColor(1);
      display.drawBox(0, 0, kDisplayWidth, kLineHeight);
      display.setDrawColor(0);

      setCursorXY<0, 8>();
    }

    void __printHeader_afterPrint()
    {
      display.setDrawColor(1);
      update();
    }

    void printHeader(char const * title)
    {
      __printHeader_beforePrint();

      OSCR::Lang::printUpperLine(title);

      __printHeader_afterPrint();
    }

    void printHeader(__FlashStringHelper const * title)
    {
      __printHeader_beforePrint();

      OSCR::Lang::printUpperLine(title);

      __printHeader_afterPrint();
    }

    void printNotificationHeader(char const * title)
    {
      __printHeader_beforePrint();

      notification();

      OSCR::Lang::printUpperLine(title);

      __printHeader_afterPrint();
    }

    void printNotificationHeader(__FlashStringHelper const * title)
    {
      __printHeader_beforePrint();

      notification();

      OSCR::Lang::printUpperLine(title);

      __printHeader_afterPrint();
    }

    void printErrorHeader(char const * title)
    {
      __printHeader_beforePrint();

      error();

      OSCR::Lang::printUpperLine(title);

      __printHeader_afterPrint();
    }

    void printErrorHeader(__FlashStringHelper const * title)
    {
      __printHeader_beforePrint();

      error();

      OSCR::Lang::printUpperLine(title);

      __printHeader_afterPrint();
    }

    void printFatalErrorHeader(char const * title)
    {
      __printHeader_beforePrint();

      fatalError();

      OSCR::Lang::printUpperLine(title);

      __printHeader_afterPrint();
    }

    void printFatalErrorHeader(__FlashStringHelper const * title)
    {
      __printHeader_beforePrint();

      fatalError();

      OSCR::Lang::printUpperLine(title);

      __printHeader_afterPrint();
    }

    template <bool sync>
    void printLine(void)
    {
      setCursorY<0>(OSCR::UI::display.ty + 8);
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
      OSCR::UI::display.print(string);
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
      OSCR::UI::display.print(string);
      OSCR::UI::setCursorY<0>(OSCR::UI::display.ty + 8);
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
      OSCR::UI::display.print(number, base);
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
      OSCR::UI::display.print(number, base);
      OSCR::UI::setCursorY<0>(OSCR::UI::display.ty + 8);
    }

    void _errorFormat() {}

    void _error(void)
    {
      waitButton();
    }

    void _fatalErrorFormat() {}

    [[ noreturn ]] void _fatalError(void)
    {
      OSCR::Power::disableCartridge();
      waitButton();
      resetArduino();
    }

    void sleep()
    {
      // ...
    }

    void drowse()
    {
      // ...
    }

    void wake()
    {
      // ...
    }

    bool __wait_keepWaiting(UserInput buttonState)
    {
      switch (buttonState)
      {
      case UserInput::kUserInputConfirm:
      case UserInput::kUserInputConfirmShort:
      case UserInput::kUserInputConfirmLong:
        return false;
      default:
        return true;
      }
    }

    void wait()
    {
      bool keepWaiting = true;

      update();

      do
      {
        switch (checkButton())
        {
        case UserInput::kUserInputConfirm:
        case UserInput::kUserInputConfirmShort:
        case UserInput::kUserInputConfirmLong:
          if (OSCR::wakeEvent()) break; // Ignore input that awakens
          OSCR::busy();
          keepWaiting = false;
          break;

        case UserInput::kUserInputIgnore:
        case UserInput::kUserInputUnknown:
          OSCR::idle();
          break;

        default: // Non-confirm events can still wake/trigger busy
          OSCR::wakeEvent();
          OSCR::busy();
          break;
        }
      }
      while (keepWaiting);

      clear();
    }

    void waitButton()
    {
      gotoLast();
      printLine(FS(OSCR::Strings::Status::PressButton));
      wait();
    }

    UserInput waitInput()
    {
      bool keepWaiting = true;
      UserInput inputState;

      update();

      do
      {
        inputState = checkButton();

        switch (inputState)
        {
        case UserInput::kUserInputIgnore:
        case UserInput::kUserInputUnknown:
          OSCR::idle();
          break;

        default:
          if (OSCR::wakeEvent()) break; // Ignore input that awakens
          keepWaiting = false;
          OSCR::busy();
          break;
        }
      }
      while (keepWaiting);

      return inputState;
    }

    /**
     * Colors/LEDs
     */

    namespace ProgressBar
    {
      constexpr uint16_t const kSteps = 20 - 1;
      uint16_t stepSize = 1;
      uint32_t progressUnit = 0;
      uint32_t displayed = 0;
      uint32_t nextUpdate = 0;
      u8g2_uint_t positionY = 0;
      u8g2_uint_t positionX = 0;

      void init(uint32_t maxProgress)
      {
        total = maxProgress;

        displayed = 0;
        current = 0;
        nextUpdate = 0;

        positionY = OSCR::UI::display.ty;
        positionX = OSCR::UI::display.tx;

        if (total <= kSteps)
        {
          stepSize = OSCR::Util::clamp(floor(kSteps / total), 1, kSteps);
          progressUnit = 1;
        }
        else
        {
          stepSize = 1;
          progressUnit = OSCR::Util::clamp(floor(total / kSteps), stepSize, (uint32_t)UINT32_MAX);
        }

        render();
      }

      void init(uint32_t maxProgress, uint8_t lineOffset)
      {
        OSCR::UI::setLineRel(lineOffset);
        init(maxProgress);
        OSCR::UI::setLineRel(-lineOffset);
      }

      void advance(uint32_t steps)
      {
        current += steps;

        if (total < 1)
        {
          if (current > kSteps) current = 0;
          render();
          return;
        }

        if (current >= nextUpdate) render();
      }

      void finish()
      {
        current = total;
        render(true);
      }

      void render(bool end)
      {
        u8g2_uint_t curX = display.tx, curY = display.ty;
        uint32_t progress = floor(current / progressUnit) * stepSize;

        setCursorY<0>(positionY);

        print(FS(OSCR::Strings::Symbol::ProgressBarOpen));

        for (uint8_t i = 0; i < kSteps; i++)
        {
          if (!total && (i == progress))
          {
            print(FS(OSCR::Strings::Symbol::ProgressBarUnknown));
            continue;
          }

          if (total && ((i <= progress) || end))
          {
            print(FS(OSCR::Strings::Symbol::ProgressBarFilled));
            continue;
          }

          print(FS(OSCR::Strings::Symbol::ProgressBarEmpty));
        }

        if (end)
        {
          printLine(FS(OSCR::Strings::Symbol::ProgressBarClose));

          nextUpdate = current + 1;
        }
        else
        {
          print(FS(OSCR::Strings::Symbol::ProgressBarClose));

          display.setCursor(curX, curY);

          nextUpdate = current + progressUnit;
        }

        displayed = current;

        update();
      }
    }
  }
}

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

#endif /* OUTPUT_SSD1306 */
