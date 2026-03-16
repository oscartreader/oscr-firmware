/**********************************************************************************
                    Cartridge Reader for Arduino Mega2560

   This project represents a community-driven effort to provide
   an easy to build and easy to modify cartridge dumper.

   SD lib: https://github.com/greiman/SdFat
   LCD lib: https://github.com/olikraus/u8g2
   Neopixel lib: https://github.com/adafruit/Adafruit_NeoPixel
   Rotary Enc lib: https://github.com/mathertel/RotaryEncoder
   SI5351 lib: https://github.com/etherkit/Si5351Arduino
   RTC lib: https://github.com/adafruit/RTClib
   Frequency lib: https://github.com/PaulStoffregen/FreqCount

   Thanks to:
   MichlK - ROM Reader for Super Nintendo
   Jeff Saltzman - 4-Way Button
   Wayne and Layne - Video Game Shield menu
   skaman - Cart ROM READER SNES ENHANCED, Famicom Cart Dumper, 2600, 5200, 7800, ARC, ATARI8, BALLY, C64, COLV, FAIRCHILD,
   INTV, LEAP, LJ, LJPRO, MSX, ODY2, PCW, POKEMINI, PV1000, PYUUTA, RCA, TI99, TRS80, VBOY, VECTREX, WSV, VIC20, VSMILE modules
   Tamanegi_taro - PCE and Satellaview modules
   splash5 - GBSmart, Wonderswan, NGP and Super A'can modules
   partlyhuman - Casio Loopy & Atari Lynx module
   hkz & themanbehindthecurtain - N64 flashram commands
   Andrew Brown & Peter Den Hartog - N64 controller protocol
   libdragon - N64 controller checksum functions
   Angus Gratton - CRC32
   Snes9x - SuperFX sram fix
   insidegadgets - GBCartRead
   RobinTheHood - GameboyAdvanceRomDumper
   Gens-gs - Megadrive checksum
   fceux - iNes header
   PsyK0p4T - Sufami Turbo module
   LuigiBlood - SNES Game Processor RAM Cassette module
   herzmx - CPS3 module

   And a special Thank You to all coders and contributors on Github and the Arduino forum:
   Ancyker, Andy-miles, BacteriaMage, Borti4938, Breyell, ButThouMust, CaitSith2, Chomemel, Dakkaron, Ducky92, Fakkuyuu, Gemarcano,
   Hxlnt, InvalidInterrupt, Jiyunomegami, Joshman196, Karimhadjsalem, Kreeblah, Lesserkuma, LuigiBlood, Majorpbx, Mattiacci, Modman,
   Niklasweber, Nsx0r, Partlyhuman, Philenotfound, Pickle, Plaidpants, PsychoFox11, PsyK0p4T, Qufb, RWeick, Ramapcsx2, Sakman55,
   Scrap-a, Sdhizumi, Smesgr9000, Splash5, Tombo89, Uzlopak, Vogelfreiheit, Vpelletier, Wfmarques

   And to nocash for figuring out the secrets of the SFC Nintendo Power cartridge.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

**********************************************************************************/

#include "syslibinc.h"
#include "hardware.h"
#include "api.h"
#include "common.h"
#include "ui.h"
#include "apps.h"

#include "common/crdb/basic.h"
#include "common/crdb/standard.h"
#include "common/crdb/extended.h"
#include "common/crdb/mapper.h"
#include "common/crdb/nes.h"
#include "common/crdb/snes.h"
#include "common/crdb/gba.h"
#include "common/crdb/ti99.h"

#include "cores/include.h"

/******************************************
  End of inclusions and forward declarations
 *****************************************/

#pragma region Setup

void setup()
{
  // Set Button Pin PG2 to Input
  DDRG &= ~(1 << 2);

#if defined(OPTION_BTN_PULLUP)
  PORTG = (1 << 2);
#else
  PORTG &= ~(1 << 2);
#endif

// Setup extra pins from the on-board ATmega
#if (HW_VERSION == 5) && defined(ENABLE_ONBOARD_ATMEGA)
  for (uint8_t pin = 4; pin < 7; ++pin)
  {
    // Pins for the voltage monitors
    DDRJ &= ~(1 << pin);
    //PORTJ |= (1 << pin);
    PORTJ &= ~(1 << pin);

    // Pins for STAT RGB LED
    DDRD |= (1 << pin);
    PORTD |= (1 << pin);
  }

  // Pin for VBUS toggle
  DDRJ |= (1 << 7);
  PORTJ &= ~(1 << 7);

  // VSELECT
  DDRD |= (1 << 7);

  // Pin for VSELECT status
  DDRE &= ~(1 << 6);
  PORTE |= (1 << 6);
#elif (HW_VERSION == 5) && !defined(ENABLE_VSELECT)
  /**
   * HW5 has status LED connected to PD7
   * Set LED Pin PD7 to Output
   */
  DDRD |= (1 << 7);
  PORTD |= (1 << 7);
#elif defined(ENABLE_VSELECT)
  /**
   * VSELECT uses pin PD7
   * Set LED Pin PD7 to Output
   */
  DDRD |= (1 << 7);
#else  /* !defined(HW5) && !defined(ENABLE_VSELECT) */
  /**
   * HW1-3 have button connected to PD7
   * Set pin PD7 to input for button
   */
  DDRD &= ~(1 << 7);
#endif /* HW5 &| ENABLE_VSELECT */

  // Reset pin states
  OSCR::Power::disableCartridge();

  OSCR::Power::setVoltage(OSCR::Voltage::Default);

  // ClockGen init
  OSCR::ClockGen::initialize();

#if !defined(ENABLE_SERIAL_OUTPUT) && defined(ENABLE_UPDATER)
  OSCR::Serial::setup();
#endif /* !ENABLE_SERIAL_OUTPUT && ENABLE_UPDATER */

  OSCR::UI::setup();

# ifndef ENABLE_LCD
#   ifdef ENABLE_CA_LED
  // Turn LED off
  digitalWrite(12, 1);
  digitalWrite(11, 1);
  digitalWrite(10, 1);
#   endif /* ENABLE_CA_LED */
  // Configure 4 Pin RGB LED pins as output
  DDRB |= (1 << DDB6);  // Red LED (pin 12)
  DDRB |= (1 << DDB5);  // Green LED (pin 11)
  DDRB |= (1 << DDB4);  // Blue LED (pin 10)
# endif /* ENABLE_LCD */

# ifdef ENABLE_NEOPIXEL
  OSCR::UI::NeoPixel::setup();
# endif /* ENABLE_NEOPIXEL */

  // Start RTC
  OSCR::Time::setup();

  // SD Card
  OSCR::Storage::setup();

#if defined(ENABLE_CONFIG)
  OSCR::Configuration::init();
#endif /* ENABLE_CONFIG */

  OSCR::Logger::setup();

#if defined(ENABLE_3V3FIX)
  OSCR::Clock::setClockSpeed(OSCR::ClockSpeed::k8MHz);  // Set clock back to low after setup
#endif /* ENABLE_3V3FIX */

#if defined(ENABLE_ONBOARD_ATMEGA)
  if (!OSCR::Power::voltagesOk())
  {
    OSCR::UI::printErrorHeader(FS(OSCR::Strings::Features::CheckVoltage));
    OSCR::UI::error(FS(OSCR::Strings::Errors::IncorrectVoltage));
  }
#endif
}

#pragma region Loop

void loop()
{
  // Start menu system
  OSCR::Menus::main();
}
