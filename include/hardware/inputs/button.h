/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#if !defined(OSCR_BUTTON_H_)
# define OSCR_BUTTON_H_

# include "common/Types.h"
# include "common/OSCR.h"
# include "common/PinControl.h"

namespace OSCR::UI
{
  // ms required for a long press
  constexpr uint32_t const kLongHold = 5000;
  // ms required for a short press
  constexpr uint32_t const kShortHold = 2000;
  // max ms between clicks for a double click event
  constexpr uint32_t const kDoubleClick = 250;
  // ms debounce period to prevent flickering when pressing or releasing the button
  constexpr uint32_t const kDebounceDelay = 20;

  class Button
  {
  protected:
    bool current = false;
    bool clicked = false;

    InterfaceInput lastEvent = InterfaceInput::kInterfaceInputIgnore;

    uint32_t buttonTime = 0;
    uint32_t clickedTime = 0;
    uint32_t lastButtonEventTime = 0;

    volatile uint8_t const & pinBank;
    uint8_t const pinNumber;

  public:
    Button(OSCR::Hardware::PinBank buttonPinBank, uint8_t buttonPinNumber);
    Button(volatile uint8_t& buttonPinBank, uint8_t buttonPinNumber);

    bool read();

    InterfaceInput check();

    uint32_t lastEventTime();
    uint32_t time();

    bool state();
  };
}

#endif /* !OSCR_BUTTON_H_ */
