#include "common.h"

#if defined(ENABLE_SERIAL_ANSI)
# include "common/Updater.h"
# include "ui/interfaces.h"
# include "ui/l10n.h"
# include "ui/interfaces/serial/ansi.h"
# include "hardware/outputs/Serial.h"

namespace OSCR
{
  namespace UI
  {
    constexpr bool const kSupportsLineAdjustments = true;

    constexpr uint8_t const kDisplayWidth = 0;
    constexpr uint8_t const kDisplayHeight = 0;

    constexpr uint8_t const kLineHeight = 0;

    constexpr uint8_t const kDisplayLines = UI_PAGE_SIZE;
    constexpr uint8_t const kDisplayLineStart = 0;

    /**
     * Define if a menu is active.
     */
    bool menuActive = false;

    /**
     * The user's selection from a menu.
     */
    uint8_t menuSelection = 0;

    /**
     * The user's selection from a non-menu. This allows non-menu methods to ask for numerical
     * input, such as the size of a cartridge, etc. It's split out so they don't need to store
     * the result.
     */
    uint8_t selection = 0;

    using OSCR::Serial::command;
    using OSCR::Serial::Style;
    using OSCR::Serial::Foreground;
    using OSCR::Serial::Background;
    using OSCR::Serial::ANSI::inputEsc;
    using OSCR::Serial::ANSI::format;

    void setup()
    {
      Serial::setup();
      Serial::ANSI::hideCursor();
    }

    void setWindowTitle(char const * title)
    {
      char windowTitle[100];
      snprintf_P(windowTitle, 100, OSCR::Serial::ANSI::TITLE, title);

      OSCR::Serial::print(windowTitle);
    }

    void setWindowTitle_P(__FlashStringHelper const * flashTitle)
    {
      char title[80];
      strlcpy_P(title, reinterpret_cast<char const *>(flashTitle), sizeof(title));

      char windowTitle[100];
      snprintf_P(windowTitle, sizeof(windowTitle), OSCR::Serial::ANSI::TITLE, title);

      OSCR::Serial::print(windowTitle);

      update();
    }

    void setLineRel(int8_t numLines)
    {
      if (numLines == 0) return;

      if (numLines < 0)
      {
        OSCR::Serial::ANSI::moveCursorUp(-numLines);
        return;
      }

      OSCR::Serial::ANSI::moveCursorDown(numLines);
    }

    void update()
    {
      OSCR::Serial::flush();
      OSCR::busy();
    }

    void clear()
    {
      format(Style::Reset);
      print(FS(OSCR::Serial::ANSI::CLEAR));
    }

    void clearLine()
    {
      OSCR::Serial::ANSI::eraseLine();
    }

    UserInput checkButton()
    {
      OSCR::tick();

      while (OSCR::Serial::available() > 0)
      {
        if (!Serial::getCommand()) continue;

        // Check if command is an escape sequence
        if (command[0] == inputEsc[0])
        {
          if (command[1] == inputEsc[1])
          {
            switch(command[2])
            {
            case UI_INPUT_SERIAL_ANSI_NEXT:
              return UserInput::kUserInputNext;

            case UI_INPUT_SERIAL_ANSI_BACK:
              return UserInput::kUserInputBack;

            case UI_INPUT_SERIAL_ANSI_CONFIRM:
              return UserInput::kUserInputConfirm;

            default:
              return UserInput::kUserInputUnknown;
            }
          }

          return UserInput::kUserInputUnknown;
        }
        // Check if command is a single character
        if (command[1] == '\0')
        {
          switch (command[0])
          {
          case UI_INPUT_SERIAL_NEXT:
            return UserInput::kUserInputNext;

          case UI_INPUT_SERIAL_BACK:
            return UserInput::kUserInputBack;

          case UI_INPUT_SERIAL_CONFIRM:
            // Clear current selection in case we were expecting a number
            if (menuActive) menuSelection = 0;
            else selection = 0;

            return UserInput::kUserInputConfirm;

          case UI_INPUT_SERIAL_CONFIRM_SHORT:
            return UserInput::kUserInputConfirmShort;

          case UI_INPUT_SERIAL_CONFIRM_LONG:
            return UserInput::kUserInputConfirmLong;

          default:
            // Check if input is a number...
            if ((command[0] >= '0') && (command[0] <= '9')) break; // '0'(0x30) - '9'(0x39)

            // There are no single-character updater commands, so we don't need to check that.

            // Unknown input.
            return UserInput::kUserInputUnknown;
          }
        }
        // Check if the input is an updater command and execute it if so...
        else if (OSCR::Updater::execCommand())
        {
          // Ignore updater commands
          return UserInput::kUserInputIgnore;
        }

        // Check if the input is an integer (>0)
        uint8_t choice = OSCR::Util::clamp(atoi(command), 0, UINT8_MAX);

        // If choice is 0 the input was invalid.
        if (!choice) return UserInput::kUserInputUnknown;

        // Set the appropriate selection.
        if (menuActive) menuSelection = choice;
        else selection = choice;

        // Return a confirm.
        return UserInput::kUserInputConfirm;
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

      for (uint8_t drawPos = (5 - this->positions); drawPos < 5; drawPos++)
      {
        uint8_t drawIndex = 4 - drawPos;
        int8_t drawDigit = getNumberAt(this->currentValue, drawPos);

        if (drawDigit < 0) drawDigit = 0;

        if (drawIndex == currentPosition)
        {
          // ...
        }

        print(drawDigit);
        print(FS(OSCR::Strings::Symbol::Space));
      }

      update();

      this->rendered = true;
    }

    void MenuRenderer::render()
    {
      printHeader(title);

      for (uint_fast8_t i = 0; i < getPageEntryCount(); i++)
      {
        // Print menu option
        Serial::ANSI::bold(FS((i == selection) ? OSCR::Strings::Symbol::MenuSelection : OSCR::Strings::Symbol::MenuSpaces));
        printLine(pageEntries[i]);
      }

      update();

      rendered = true;
    }

    uint16_t MenuRenderer::select()
    {
      if (!rendered) render();

      bool selected = false;

      do
      {
        switch (waitInput())
        {
        case UserInput::kUserInputBack:
          navigate(NavDir::kNavDirBackward);
          if (!rendered) this->render();
          break;

        case UserInput::kUserInputNext:
          navigate(NavDir::kNavDirForward);
          if (!rendered) this->render();
          break;

        case UserInput::kUserInputConfirm:
        case UserInput::kUserInputConfirmShort:
        case UserInput::kUserInputConfirmLong:
          if (!onConfirm()) break;
          selected = true;
          break;

        case UserInput::kUserInputUnknown:
          printLine(F("Unknown Option"));
          break;

        default:
          break;
        }
      }
      while (!selected);

      return getEntryIndex();
    }

    void MenuRenderer::navigate(NavDir direction)
    {
      Serial::ANSI::saveCursorPos();
      Serial::ANSI::moveCursor(2 + selection);
      Serial::ANSI::bold(FS(OSCR::Strings::Symbol::MenuSpaces));
      Serial::ANSI::restoreCursorPos();

      switch (direction)
      {
      case NavDir::kNavDirBackward: return navPrev();
      case NavDir::kNavDirForward:  return navNext();
      }
    }

    void MenuRenderer::onSelectionChange()
    {
      Serial::ANSI::saveCursorPos();
      Serial::ANSI::moveCursor(2 + selection);
      Serial::ANSI::bold(FS(OSCR::Strings::Symbol::MenuSelection));
      Serial::ANSI::restoreCursorPos();

      update();
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
          strlcpy(pageEntries[menuIndex], entries[entryIndex], min(entryLengthMax, UI_PAGE_ENTRY_LENTH_MAX));
          break;

        default:
          break;
        }
      }
    }

    /**
     * Messages
     */

    void __printHeader_beforePrint(char const * title)
    {
      setWindowTitle(title);
      clear();
    }

    void __printHeader_beforePrint(__FlashStringHelper const * title)
    {
      setWindowTitle_P(title);
      clear();
    }

    void printHeader(char const * title)
    {
      __printHeader_beforePrint(title);

      Serial::ANSI::boldUpperLine(title);
    }

    void printHeader(__FlashStringHelper const * title)
    {
      __printHeader_beforePrint(title);

      Serial::ANSI::boldUpperLine(title);
    }

    void printNotificationHeader(char const * title)
    {
      __printHeader_beforePrint(title);

      notification();

      Serial::ANSI::boldUpperLine(title);
    }

    void printNotificationHeader(__FlashStringHelper const * title)
    {
      __printHeader_beforePrint(title);

      notification();

      Serial::ANSI::boldUpperLine(title);
    }

    void printErrorHeader(char const * title)
    {
      __printHeader_beforePrint(title);

      error();

      Serial::ANSI::boldUpperLine(title);
    }

    void printErrorHeader(__FlashStringHelper const * title)
    {
      __printHeader_beforePrint(title);

      error();

      Serial::ANSI::boldUpperLine(title);
    }

    void printFatalErrorHeader(char const * title)
    {
      __printHeader_beforePrint(title);

      fatalError();

      Serial::ANSI::boldUpperLine(title);
    }

    void printFatalErrorHeader(__FlashStringHelper const * title)
    {
      __printHeader_beforePrint(title);

      fatalError();

      Serial::ANSI::boldUpperLine(title);
    }

    void _errorFormat()
    {
      Serial::ANSI::setForeground(Foreground::Red);
      Serial::ANSI::setBackground(Background::Default);
      Serial::ANSI::apply();
    }

    void _error(void)
    {
      waitButton();
    }

    void _fatalErrorFormat()
    {
      Serial::ANSI::setForeground(Foreground::White);
      Serial::ANSI::setBackground(Background::Red);
      Serial::ANSI::apply();
    }

    [[ noreturn ]] void _fatalError(void)
    {
      OSCR::Power::disableCartridge();
      waitButton();
      resetArduino();
    }

    void sleep()
    {
# if defined(ENABLE_NEOPIXEL)
      OSCR::UI::NeoPixel::off();
# endif
    }

    void drowse()
    {
# if defined(ENABLE_NEOPIXEL)
      OSCR::UI::NeoPixel::dim();
# endif
    }

    void wake()
    {
# if defined(ENABLE_NEOPIXEL)
      OSCR::UI::NeoPixel::on();
# endif
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
          keepWaiting = false;
          OSCR::busy();
          break;

        default:
          OSCR::idle();
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
      update();

      UserInput inputState;

      do
      {
        inputState = checkButton();
        OSCR::idle();
      }
      while ((UserInput::kUserInputIgnore == inputState) || (UserInput::kUserInputUnknown == inputState));

      OSCR::busy();

      return inputState;
    }

    namespace ProgressBar
    {
      constexpr uint16_t kSteps = 30;
      uint16_t stepSize = 1;
      uint32_t progressUnit = 0;
      uint32_t displayed = 0;
      uint32_t nextUpdate = 0;

      void init(uint32_t maxProgress)
      {
        total = maxProgress;
        current = 0;
        displayed = 0;

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

        // Progress bar with unknown target
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
        uint32_t progress = floor(current / progressUnit) * stepSize;

        OSCR::Serial::ANSI::saveCursorPos();
        OSCR::Serial::ANSI::eraseLine();

        OSCR::UI::print(FS(OSCR::Strings::Symbol::ProgressBarOpen));

        for (uint8_t i = 0; i < kSteps; i++)
        {
          if (!total && (i == progress))
          {
            OSCR::UI::print(FS(OSCR::Strings::Symbol::ProgressBarUnknown));
            continue;
          }

          if (total && ((i <= progress) || end))
          {
            OSCR::UI::print(FS(OSCR::Strings::Symbol::ProgressBarFilled));
            continue;
          }

          OSCR::UI::print(FS(OSCR::Strings::Symbol::ProgressBarEmpty));
        }

        OSCR::UI::print(FS(OSCR::Strings::Symbol::ProgressBarClose));

        if (total)
        {
          uint8_t progressPercent = floor((current * 100) / total);

          OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));

          if (progressPercent < 100) OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));
          if (progressPercent < 10) OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));

          OSCR::UI::print(progressPercent, DEC);

          OSCR::UI::print(FS(OSCR::Strings::Symbol::Percent));
        }

        if (end)
        {
          OSCR::UI::printLine();
        }
        else
        {
          OSCR::Serial::ANSI::restoreCursorPos();
        }

        displayed = current;
        nextUpdate = displayed + progressUnit;

        OSCR::UI::update();
      }
    }
  }
}

#endif /* ENABLE_SERIAL_ANSI */
