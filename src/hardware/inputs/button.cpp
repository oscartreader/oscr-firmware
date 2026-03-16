#include "hardware/inputs/button.h"

namespace OSCR::UI
{
  Button::Button(OSCR::Hardware::PinBank buttonPinBank, uint8_t buttonPinNumber) : pinBank(OSCR::Hardware::getPinFromBank(buttonPinBank)), pinNumber(buttonPinNumber)
  {
    // ...
  }

  Button::Button(volatile uint8_t& buttonPinBank, uint8_t buttonPinNumber) : pinBank(buttonPinBank), pinNumber(buttonPinNumber)
  {
    // ...
  }

  bool Button::read()
  {
    if (!state())
    {
      current = false;
      return false;
    }

    uint32_t buttonDown = millis();
    while (state()) nop;
    uint32_t buttonDownTime = millis() - buttonDown;

    if (buttonDownTime > kDebounceDelay)
    {
      buttonTime = buttonDownTime;
      current = true;
      lastButtonEventTime = millis();
      return true;
    }

    return false;
  }

  InterfaceInput Button::check()
  {
    if (read())
    {
      if (buttonTime >= kLongHold)
      {
        clicked = false;
        lastEvent = InterfaceInput::kInterfaceInputPressLong;
        return lastEvent;
      }

      if (buttonTime >= kShortHold)
      {
        clicked = false;
        lastEvent = InterfaceInput::kInterfaceInputPressShort;
        return lastEvent;
      }

      if (clicked)
      {
        uint32_t const sinceLastClick = lastButtonEventTime - clickedTime;

        clicked = false;
        clickedTime = 0;

        if (sinceLastClick < kDoubleClick)
        {
          return InterfaceInput::kInterfaceInputPressDouble;
        }
      }

      clicked = true;
      clickedTime = millis();

      return InterfaceInput::kInterfaceInputIgnore;
    }

    if (clicked)
    {
      uint32_t const sinceLastClick = millis() - clickedTime;

      if (sinceLastClick > kDoubleClick)
      {
        clicked = false;
        clickedTime = 0;
        return InterfaceInput::kInterfaceInputPress;
      }
    }

    return InterfaceInput::kInterfaceInputIgnore;
  }

  uint32_t Button::lastEventTime()
  {
    return lastButtonEventTime;
  }

  uint32_t Button::time()
  {
    return buttonTime;
  }

  bool Button::state()
  {
    return (
      (pinBank & (1 << pinNumber)) >> pinNumber
    ) == 0;
  }
}
