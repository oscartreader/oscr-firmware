#include "config.h"

#if HAS_SELFTEST
# include "common.h"
# include "api.h"
# include "hardware.h"
# include "ui.h"
# include "cores/SelfTest.h"

namespace OSCR::Cores::SelfTest
{
  constexpr char const PROGMEM PinTemplate[] = "%S%.2" PRIu8;
  constexpr char const PROGMEM PinShortTestTemplate[] = "Pins %s + %s";
  constexpr char const PROGMEM PinShortTestingTemplate[] = "Pin %s + ...";
  constexpr char const PROGMEM PinD[] = "D";
  constexpr char const PROGMEM PinA[] = "A";

  uint8_t pinIdxToNum(uint8_t idx)
  {
    if (idx <  8) return idx +  2; //  0 -  7 =  2 -  9 || +  2
    if (idx < 12) return idx +  6; //  8 - 11 = 14 - 17 || +  6
    if (idx < 28) return idx + 10; // 12 - 27 = 22 - 37 || + 10
    if (idx < 36) return idx + 14; // 28 - 35 = 42 - 49 || + 14
    if (idx < 52) return idx + 18; // 36 - 51 = 54 - 69 || + 18

    return 0; // invalid
  }

  // Floating pins can flip between states, this looks for a consistent HIGH (1) state.
  bool __multiRead(uint8_t pinNumber, uint16_t msDelay = 10, uint8_t cycles = 3)
  {
    uint8_t read = 0;

    for (uint8_t i = 0; i < cycles; i++)
    {
      if (digitalRead(pinNumber)) read++;

      delay(msDelay);
    }

    return (read == cycles);
  }

  inline bool isGNDPin(uint8_t pinNumber)
  {
    return (pinNumber == 0);
  }

  inline bool isDigitalPin(uint8_t pinNumber)
  {
    return (pinNumber > 53);
  }

  inline uint8_t adjPinNumber(uint8_t pinNumber)
  {
    return isDigitalPin(pinNumber) ? (pinNumber - 54) : pinNumber;
  }

  void printHeader()
  {
    OSCR::UI::printHeader(OSCR::Lang::formatSubmenuTitle(OSCR::Strings::Cores::SelfTest));
  }

  void printErrorHeader()
  {
    OSCR::UI::printErrorHeader(OSCR::Lang::formatSubmenuTitle(OSCR::Strings::Cores::SelfTest));
  }

  void printShortedPinMessage(uint8_t pin1, int8_t pin2)
  {
    char pin1str[4];
    char pin2str[4];
    char bufferStr[30];

    snprintf_P(pin1str, sizeof(pin1str), PinTemplate, (isDigitalPin(pin1) ? PinD : PinA), adjPinNumber(pin1));

    if (pin2 > 0)
    {
      snprintf_P(pin2str, sizeof(pin2str), PinTemplate, (isDigitalPin(pin2) ? PinD : PinA), adjPinNumber(pin2));
    }

    if (pin2 < 0)
      snprintf_P(bufferStr, sizeof(bufferStr), PinShortTestingTemplate, pin1str);
    else
      snprintf_P(bufferStr, sizeof(bufferStr), PinShortTestTemplate, pin1str, (isGNDPin(pin2) ? "GND" : pin2str));

    OSCR::UI::printLineSync(bufferStr);
  }

  void printShortedPinError(uint8_t pin1, int8_t pin2 = 0)
  {
    printErrorHeader();
    printShortedPinMessage(pin1, pin2);
    OSCR::UI::error(F(" are shorted!"));
  }

  void printShortedPinTest(uint8_t pin1, int8_t pin2 = 0)
  {
    printHeader();
    OSCR::UI::printLine(F("Testing..."));
    printShortedPinMessage(pin1, pin2);
  }

  void run()
  {
    // Set to 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);

    printHeader();
    OSCR::UI::printLine(F("Remove all Cartridges"));
    OSCR::UI::printLine(F(" before continuing!"));

#if (defined(HW3) || defined(HW2))
    OSCR::UI::printLine(F("And turn the EEP switch on."));
#endif
    OSCR::UI::waitButton();

    OSCR::Power::enableCartridge();

    testVoltages();

    // Test if pin 7 is held high by 1K resistor
    pinMode(7, INPUT);

    printHeader();
    OSCR::UI::printLineSync(F("Testing 1K resistor..."));

    if (!__multiRead(7, 200, 20))
    {
      printErrorHeader();
      OSCR::UI::error(F("Error: 1K resistor not found."));
    }

    for (uint8_t pinIdx = 0; uint8_t pinNumber = pinIdxToNum(pinIdx); pinIdx++)
    {
      pinMode(pinNumber, INPUT_PULLUP);
    }

    for (uint8_t pinIdx = 0; uint8_t pinNumber = pinIdxToNum(pinIdx); pinIdx++)
    {
      printShortedPinTest(pinNumber);

      if (!digitalRead(pinNumber))
      {
        printShortedPinError(pinNumber);
      }
    }

    // Test for short between pins 2-9, 14-17, 22-37, 42-49, 54-69
    for (uint8_t pinIdx = 0; uint8_t pinNumber = pinIdxToNum(pinIdx); pinIdx++)
    {
      pinMode(pinNumber, OUTPUT);
      digitalWrite(pinNumber, LOW);
      printShortedPinTest(pinNumber, -1);

      /**
       * We start at `pinNumber + 1` instead of `2` because we've
       * already tested the prior numbers against all pin combinations.
       */
      for (uint8_t pinIdx2 = pinIdx + 1; uint8_t pinNumber2 = pinIdxToNum(pinIdx2); pinIdx2++)
      {
        //printShortedPinTest(pinNumber, pinNumber2);

        pinMode(pinNumber2, INPUT_PULLUP);

        if (!__multiRead(pinNumber2))
        {
          printShortedPinError(pinNumber, pinNumber2);
        }
      }

      pinMode(pinNumber, INPUT_PULLUP);
    }

#if defined(ENABLE_CLOCKGEN)
    printHeader();
    OSCR::UI::printLine(F("Testing clock generator..."));

    if (!OSCR::ClockGen::initialize())
    {
      printErrorHeader();
      OSCR::UI::error(FS(OSCR::Strings::Errors::ClockGenMissing));
    }
    else
    {
      //clockgen.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
      OSCR::ClockGen::clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
      OSCR::ClockGen::clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
      //OSCR::ClockGen::clockgen.pll_reset(SI5351_PLLA);
      //OSCR::ClockGen::clockgen.pll_reset(SI5351_PLLB);
      OSCR::ClockGen::clockgen.set_freq(400000000ULL, SI5351_CLK0);
      OSCR::ClockGen::clockgen.set_freq(100000000ULL, SI5351_CLK1);
      OSCR::ClockGen::clockgen.set_freq(307200000ULL, SI5351_CLK2);
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 1);
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK2, 1);
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK0, 1);
    }
#endif /* ENABLE_CLOCKGEN */

    OSCR::Power::disableCartridge();

    printHeader();
    OSCR::UI::printLine(F("All tests finished."));
    OSCR::UI::waitButton();
  }

  void testVoltages()
  {
#ifdef ENABLE_ONBOARD_ATMEGA
    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Power::VCC));
    OSCR::UI::print(FS(OSCR::Strings::Symbol::LabelEnd));
    if (OSCR::Power::getVoltage() == OSCR::Voltage::k5V)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Power::Voltage5));
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Power::Voltage3V3));
    }

    OSCR::UI::print(FS(OSCR::Strings::Power::Voltage3V3));
    OSCR::UI::print(FS(OSCR::Strings::Symbol::LabelEnd));
    if (OSCR::Power::checkVoltage(OSCR::Voltage::k3V3))
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
    }

    OSCR::UI::print(FS(OSCR::Strings::Power::Voltage5));
    OSCR::UI::print(FS(OSCR::Strings::Symbol::LabelEnd));
    if (OSCR::Power::checkVoltage(OSCR::Voltage::k5V))
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
    }

    OSCR::UI::print(FS(OSCR::Strings::Power::VSELECT));
    OSCR::UI::print(FS(OSCR::Strings::Symbol::LabelEnd));
    if (OSCR::Power::getVoltageSelect() == OSCR::Voltage::k5V)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Power::Voltage5));
    } else {
      OSCR::UI::printLine(FS(OSCR::Strings::Power::Voltage3V3));
    }

    OSCR::UI::waitButton();
#endif
  }

} /* namespace OSCR::Cores::SelfTest */

#endif
