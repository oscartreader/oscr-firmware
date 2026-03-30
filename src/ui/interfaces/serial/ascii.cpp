#include "common.h"

#if (HARDWARE_INPUT_TYPE == INPUT_SERIAL) && (OPTION_SERIAL_OUTPUT == SERIAL_ASCII)
# include "common/Updater.h"
# include "ui/interfaces.h"
# include "ui/l10n.h"
# include "ui/interfaces/serial.h"

namespace OSCR
{
  namespace UI
  {
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
    using OSCR::Serial::getCommand;

    void setup()
    {
      OSCR::Serial::setup();

      OSCR::UI::printLine();
      OSCR::UI::printLine(F("Cartridge Reader"));
      OSCR::UI::printLine(F("2025 github.com/sanni"));
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

    void setLineRel(int8_t numLines __attribute__((unused)))
    {
      // Unsupported
    }

    void update()
    {
      OSCR::Serial::flush();
      OSCR::busy();
    }

    void clear()
    {
      OSCR::UI::printLine();
      OSCR::UI::printLine();
      OSCR::UI::printLine();
    }

    void clearLine()
    {
      OSCR::UI::printLine();
    }

    UserInput checkButton()
    {
      OSCR::tick();

      while (OSCR::Serial::available() > 0)
      {
        if (!getCommand()) continue;

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
      // @todo Implement a way for people to just type it

      this->rendered = true;
    }

    void MenuRenderer::render()
    {
      OSCR::UI::printHeader(this->title);

      for (uint_fast8_t i = 0; i < getPageEntryCount(); i++)
      {
        // Print menu option
        OSCR::UI::print(FS((i == selection) ? OSCR::Strings::Symbol::MenuSelection : OSCR::Strings::Symbol::MenuSpaces));
        OSCR::UI::printLine(pageEntries[i]);
      }

      OSCR::UI::update();

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
          break;

        case UserInput::kUserInputNext:
          navigate(NavDir::kNavDirForward);
          break;

        case UserInput::kUserInputConfirm:
        case UserInput::kUserInputConfirmShort:
        case UserInput::kUserInputConfirmLong:
          if (!onConfirm()) break;
          selected = true;
          break;

        case UserInput::kUserInputUnknown:
          OSCR::UI::printLine(F("Unknown Option"));
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
      switch (direction)
      {
      case NavDir::kNavDirBackward: return navPrev();
      case NavDir::kNavDirForward:  return navNext();
      }
    }

    void MenuRenderer::onSelectionChange()
    {
      render();
      OSCR::UI::update();
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

        strlcpy(pageEntries[menuIndex], entries->options[entryIndex], entryLengthMax);
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
    }

    void __printHeader_afterPrint()
    {
      print(FS(OSCR::Strings::Symbol::Space));

      for (uint16_t i = 0, printLines = 50 - 2; i < printLines; i++)
      {
        print(FS(OSCR::Strings::Symbol::Asterisk));
      }

      printLineSync(FS(OSCR::Strings::Symbol::Space));
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

    void wait()
    {
      bool keepWaiting = true;

      OSCR::UI::update();

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
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::PressButton));
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
      uint32_t lastUpdate = 0;

      void init(uint32_t maxProgress)
      {
        total = maxProgress;
        current = 0;
      }

      void init(uint32_t maxProgress, uint8_t lineOffset __attribute__((unused)))
      {
        //OSCR::UI::setLineRel(lineOffset); // Unsupported
        init(maxProgress);
        //OSCR::UI::setLineRel(-lineOffset); // Unsupported
      }

      void advance(uint32_t steps)
      {
        current += steps;

        // Progress bar with unknown target
        if (!total)
        {
          return render();
        }

        uint8_t progress = floor(current / total);

        if (progress > lastUpdate) render();
      }

      void finish()
      {
        current = total;
        render(true);
      }

      void render(bool end)
      {
        // Progress bar with unknown target
        if (!total)
        {
          if (end)
          {
            OSCR::UI::printLine(FS(OSCR::Strings::Symbol::FullStop));
          }
          else
          {
            OSCR::UI::print(FS(OSCR::Strings::Symbol::FullStop));
          }

          lastUpdate = current;

          return OSCR::UI::update();
        }

        int8_t progress = floor(current / total);

        OSCR::UI::print(progress);

        if (end) OSCR::UI::printLine(FS(OSCR::Strings::Symbol::Percent));
        else
        {
          OSCR::UI::print(FS(OSCR::Strings::Symbol::Percent));
          OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));
          OSCR::UI::print(FS(OSCR::Strings::Symbol::Ellipsis));
          OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));
        }

        lastUpdate = progress;

        OSCR::UI::update();
      }
    }
  }
}

#endif /* (HARDWARE_INPUT_TYPE == 1) && (OPTION_SERIAL_OUTPUT == 1) */
