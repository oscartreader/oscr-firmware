#include "config.h"

#if defined(ENABLE_SERIAL_OUTPUT) || defined(ENABLE_UPDATER)
# include "arch.h"
# include "syslibinc.h"
# include "macros.h"
# include "hardware/outputs/Serial.h"
# include "api/Time.h"
# include "common/Power.h"
# include "ui/l10n.h"
# include "common/OSCR.h"
# include "cores/GameBoyAdvance.h"

namespace OSCR
{
  namespace Updater
  {
    using OSCR::Serial::command;
    using OSCR::Lang::Updater::oscrInfo;
    using OSCR::Lang::Updater::flagValue;
    using OSCR::Lang::Updater::flagValueV;

    void printVersionToSerial()
    {
      //
      // Preamble
      //

      oscrInfo();


      //
      // Output Method
      //

# if (HARDWARE_OUTPUT_TYPE == OUTPUT_SERIAL)
#   if (OPTION_SERIAL_OUTPUT == SERIAL_ASCII)
      flagValue(PSTR("OUT"), PSTR("ASCII"));
#   elif (OPTION_SERIAL_OUTPUT == SERIAL_ANSI)
      flagValue(PSTR("OUT"), PSTR("ANSI"));
#   endif
# elif (HARDWARE_OUTPUT_TYPE == OUTPUT_SSD1306)
      flagValue(PSTR("OUT"), PSTR("OLED"));
# elif (HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864)
#   if (OPTION_LCD_TYPE == LCD_MKS)
      flagValue(PSTR("OUT"), PSTR("MKS"));
#   elif (OPTION_LCD_TYPE == LCD_SSRETRO)
      flagValue(PSTR("OUT"), PSTR("SSRETRO"));
#   endif
# endif
      // --

      //
      // Localization
      //

      flagValue(PSTR("LANG"), (uint8_t)OSCR_REGION);
      flagValue(PSTR("REGN"), (uint8_t)OSCR_LANGUAGE);

      // --

      //
      // Cores
      //

      uint64_t coreFlag = 0;

      for (uint8_t i = 0; i < OSCR::Menus::kMainMenuCoreCount; i++)
      {
        coreFlag |= ((uint64_t)1) << (((uint8_t)OSCR::Menus::MainMenuOptions[i].id) - 1);
      }

      flagValue(PSTR("CORES"), coreFlag);

      // --

      //
      // Features
      //

      flagValue(PSTR("FEATURES"), kFirmwareFeatures);

      // --

      //
      // Features with values

# if OPTION_SMS_ADAPTER || OPTION_GG_ADAPTER || OPTION_SG1000_ADAPTER
      uint8_t SMSValues[] = {
        OPTION_SMS_ADAPTER,
        OPTION_GG_ADAPTER,
        OPTION_SMS_ADAPTER,
      };

      flagValueV(PSTR("SMS"), BUFFN(SMSValues));
# endif

# if OPTION_PERFORMANCE_FLAGS
      flagValue(PSTR("PERF"), static_cast<uint8_t>(OPTION_PERFORMANCE_FLAGS));
# endif

# if OPTION_VOLTAGE_MONITOR_METHOD
      flagValue(PSTR("VLTMON"), (uint8_t)OPTION_VOLTAGE_MONITOR_METHOD);
# endif
      // --

      OSCR::Serial::printLine();
      OSCR::Serial::flush();
    }

    bool execCommand()
    {
#if defined(ENABLE_UPDATER)
      // VERCHK: Gets OSCR version and features
      if (strcasecmp_P(command, PSTR("VERCHK")) == 0)
      {
        delay(500);
        printVersionToSerial();
      }
      // VLTCHK: Gets voltage statuses
      else if (strcasecmp_P(command, PSTR("VLTCHK")) == 0)
      {
        OSCR::Voltage statusVCC = ((PINJ & (1 << 4)) ? OSCR::Voltage::k5V : OSCR::Voltage::k3V3);
        bool status3V3 = (PINJ & (1 << 6));
        bool status5V = (PINJ & (1 << 5));
        OSCR::Voltage statusVselect = ((PINE & (1 << 6)) ? OSCR::Voltage::k5V : OSCR::Voltage::k3V3);

        OSCR::Serial::print(FS(OSCR::Strings::Power::VCC));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        if (statusVCC == OSCR::Voltage::k5V)
          OSCR::Serial::printLine(FS(OSCR::Strings::Power::Voltage5));
        else
          OSCR::Serial::printLine(FS(OSCR::Strings::Power::Voltage3V3));

        OSCR::Serial::print(FS(OSCR::Strings::Power::Voltage3V3));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        if (status3V3)
          OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
        else
          OSCR::Serial::printLine(FS(OSCR::Strings::Common::FAIL));

        OSCR::Serial::print(FS(OSCR::Strings::Power::Voltage5));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        if (status5V) OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
        else OSCR::Serial::printLine(FS(OSCR::Strings::Common::FAIL));

        OSCR::Serial::print(FS(OSCR::Strings::Power::VSELECT));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        if (statusVselect == OSCR::Voltage::k5V)
          OSCR::Serial::printLine(FS(OSCR::Strings::Power::Voltage5));
        else
          OSCR::Serial::printLine(FS(OSCR::Strings::Power::Voltage3V3));

      }
      // GETCLOCK: Gets the MEGA's current clock speed.
      else if (strcasecmp_P(command, PSTR("GETCLOCK")) == 0)
      {
        OSCR::Serial::print(F("Clock is running at "));
        OSCR::Serial::print((OSCR::Clock::getClockSpeed() == OSCR::ClockSpeed::k16MHz) ? 16UL : 8UL);
        OSCR::Serial::printLine(F("MHz"));
      }
#if defined(ENABLE_VSELECT)
      // (G/S)ETVOLTS: Get and set the voltage.
      else if (command[1] == 'E' && command[2] == 'T' && command[3] == 'V' && command[4] == 'O' && command[5] == 'L' && command[6] == 'T' && command[7] == 'S')
      {
        if (command[0] == 'S')
        {
          switch (command[9])
          {
            case '3': OSCR::Power::setVoltage(OSCR::Voltage::k3V3); break;
            case '5': OSCR::Power::setVoltage(OSCR::Voltage::k5V); break;
          }

          delay(100);
        }

        OSCR::Serial::print(F("Voltage is set to "));
        OSCR::Serial::printLine(FS((OSCR::Power::getVoltage() == OSCR::Voltage::k5V) ? OSCR::Strings::Power::Voltage5 : OSCR::Strings::Power::Voltage3V3));
      }
#endif /* ENABLE_VSELECT */
#if defined(ENABLE_RTC)
      // (G/S)ETTIME: Get and set the date/time.
      else if (command[1] == 'E' && command[2] == 'T' && command[3] == 'T' && command[4] == 'I' && command[5] == 'M' && command[6] == 'E')
      {
        char time[21];

        if (command[0] == 'S')
        {
          OSCR::Serial::printLine(F("Setting Time..."));
          OSCR::Time::rtc.adjust(DateTime(command + 8));
        }

        OSCR::Serial::print(F("Current Time: "));
        OSCR::Serial::printLine(OSCR::Time::RTCStamp(time));
      }
#endif /* ENABLE_RTC */
#if HAS_GBX
      else if (strcasecmp_P(command, PSTR("TESTCARTCLK")) == 0)
      {
        OSCR::Cores::GameBoyAdvance::rtcTest();
      }
#endif /* ENABLE_GBX */
      // Unknown command
      else
      {
        return false;
      }

      return true;
#else /* !ENABLE_UPDATER */
      return false;
#endif /* ENABLE_UPDATER */
    }

    void check()
    {
#if defined(ENABLE_UPDATER)
      while (OSCR::Serial::available() > 0)
      {
        if (!OSCR::Serial::getCommand()) continue;

        if (!execCommand())
        {
          OSCR::Serial::print(FS(OSCR::Strings::Common::OSCR));
          OSCR::Serial::printLine(F(": Unknown Command"));
        }
      }
#endif /* ENABLE_UPDATER */
    }
  } // namespace Updater
} // namespace OSCR

#endif
