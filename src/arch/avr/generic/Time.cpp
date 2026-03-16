/******************************************
  RTC Module
 *****************************************/

#include "arch/avr/syslibinc.h"

# if defined(OSCR_ARCH_AVR)

#include "macros.h"
#include "ui.h"
#include "api/Time.h"

namespace OSCR::Time
{
  constexpr uint8_t kBCDSecond  = 0;
  constexpr uint8_t kBCDMinute  = 1;
  constexpr uint8_t kBCDHour	  = 2;
  constexpr uint8_t kBCDWkd	    = 3;
  constexpr uint8_t kBCDDay	    = 4;
  constexpr uint8_t kBCDMonth	  = 5;
  constexpr uint8_t kBCDYear	  = 6;

  constexpr char const PROGMEM menuOptionSetDate[] = "Date";
  constexpr char const PROGMEM menuOptionSetTime[] = "Time";

#if defined(ENABLE_RTC)
  bool rtcStarted = false;
  DateTime eventStarted;
#else
  constexpr bool const rtcStarted = false;
  uint32_t eventStarted = 0;
#endif

#if defined(ENABLE_RTC)
  enum class MenuOption : uint8_t
  {
    SetDate,
    SetTime,
    Back,
  };

  constexpr char const * const PROGMEM menuOptions[] = {
    menuOptionSetDate,
    menuOptionSetTime,
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr uint8_t const kMenuOptionCount = sizeofarray(menuOptions);

# if (RTC_TYPE == RTCOPT_DS3231)
  RTC_DS3231 rtc;
# elif (RTC_TYPE == RTCOPT_DS1307)
  RTC_DS1307 rtc;
# endif

#endif /* ENABLE_RTC */

  // Setup RTC
  bool setup()
  {
#if defined(ENABLE_RTC)
    // Start RTC
    if (!rtc.begin())
    {
      // When RTC fails, print error and return. No reason to prevent boot.
      OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::OSCR));
      OSCR::UI::error(FS(OSCR::Strings::Errors::RTCMissing));
      return false;
    }

    rtcStarted = true;

    // RTC_DS1307 does not have lostPower()
# if (RTC_TYPE == RTCOPT_DS3231)
    // Set RTC Date/Time of Sketch Build if it lost battery power
    // After initial setup it would have lost battery power ;)
    if (rtc.lostPower())
    {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
# endif
#endif /* ENABLE_RTC */

    return rtcStarted;
  }

#if defined(ENABLE_RTC)
  bool exists()
  {
    return rtcStarted;
  }
#else
  // Implemented in the header
#endif

#if defined(ENABLE_RTC)
  DateTime now()
  {
    return rtc.now();
  }

  // Callback for file timestamps
  void dateTime(uint16_t * date, uint16_t * time)
  {
    DateTime now = rtc.now();

    // Return date using FAT_DATE macro to format fields
    *date = FAT_DATE(now.year(), now.month(), now.day());

    // Return time using FAT_TIME macro to format fields
    *time = FAT_TIME(now.hour(), now.minute(), now.second());
  }

  bool setDateTime(DateTime const & dt)
  {
    if (!dt.isValid()) return false;

    rtc.adjust(dt);

    return true;
  }

  bool setDateTime(uint16_t const year, uint8_t const month, uint8_t const day, uint8_t const hour, uint8_t const min, uint8_t const sec)
  {
    DateTime const dt(year, month, day, hour, min, sec);

    return setDateTime(dt);
  }

  bool setTime(uint8_t const hour, uint8_t const min, uint8_t const sec)
  {
    DateTime const now = rtc.now();

    DateTime const dt(now.year(), now.month(), now.day(), hour, min, sec);

    return setDateTime(dt);
  }

  bool setDate(uint16_t const year, uint8_t const month, uint8_t const day)
  {
    DateTime const now = rtc.now();

    DateTime const dt(year, month, day, now.hour(), now.minute(), now.second());

    return setDateTime(dt);
  }

  /******************************************
    RTC Time Stamp Setup
    Call in any other script
  *****************************************/
  // Format a Date/Time stamp
  char * RTCStamp(char * time, uint8_t size)
  {
    // Set a format
    OSCR::Util::copyStr_P(time, size, F("DDMMMYYYY hh:mm:ssAP"));

    // Get current Date/Time
    return rtc.now().toString(time);
  }

  uint8_t daysInMonth(uint8_t month)
  {
    return 28 + ( ( 0xEEFBB3 >> ( ( month - 1 ) * 2 ) ) & 0b11 );
  }

  void startMeasure()
  {
    eventStarted = rtc.now();
  }

  char * getDifference(char * diffStr, size_t length)
  {
    if (!eventStarted.isValid())
    {
      strncpy_P(diffStr, PSTR("? sec"), length);
      return diffStr;
    }

    TimeSpan difference = rtc.now() - eventStarted;

    uint8_t days = difference.days();
    uint8_t hours = difference.hours();
    uint8_t minutes = difference.minutes();
    uint8_t seconds = difference.seconds();

    if (days > 0)
    {
      snprintf_P(diffStr, length, PSTR("%" PRIu8 " dy, %" PRIu8 " hr, %" PRIu8 " min, %" PRIu8 " sec"), days, hours, minutes, seconds);
    }
    else if (hours > 0)
    {
      snprintf_P(diffStr, length, PSTR("%" PRIu8 " hr, %" PRIu8 " min, %" PRIu8 " sec"), hours, minutes, seconds);
    }
    else if (minutes > 0)
    {
      snprintf_P(diffStr, length, PSTR("%" PRIu8 " min, %" PRIu8 " sec"), minutes, seconds);
    }
    else
    {
      snprintf_P(diffStr, length, PSTR("%" PRIu8 " sec"), seconds);
    }

    return diffStr;
  }

  void printDifference()
  {
    char diffStr[32];
    OSCR::UI::printLine(getDifference(BUFFN(diffStr)));
  }

  uint8_t bcdBin(uint8_t val)
  {
    return val - 6 * (val >> 4);
  }

  uint8_t binBcd(uint8_t val)
  {
    return val + 6 * (val / 10);
  }

  void dtBcd(uint8_t * edit_bst, const DateTime & dt = nullptr)
  {
    const DateTime dateTime = (dt == nullptr) ? rtc.now() : dt;

    edit_bst[kBCDSecond] = binBcd(dateTime.second());
    edit_bst[kBCDMinute] = binBcd(dateTime.minute());
    edit_bst[kBCDHour] = binBcd(dateTime.hour());
    edit_bst[kBCDDay] = binBcd(dateTime.day());
    edit_bst[kBCDMonth] = binBcd(dateTime.month());
    edit_bst[kBCDYear] = binBcd(dateTime.year() - 2000U);
  }

  void menu()
  {
    char dtStr[21];

    do
    {
      OSCR::Util::copyStr_P(dtStr, sizeof(dtStr), F(" YYYY-MM-DD hh:mm:ss"));

      OSCR::UI::printHeader(OSCR::Lang::formatSubmenuTitle(OSCR::Strings::Features::Settings));
      OSCR::UI::printLine();
      OSCR::UI::printLine(F("Current Time:"));
      OSCR::UI::printLine(rtc.now().toString(dtStr));
      OSCR::UI::waitButton();

      MenuOption menuSelection = static_cast<MenuOption>(OSCR::UI::menu(OSCR::Lang::formatSubmenuTitle(OSCR::Strings::Features::Settings), menuOptions, kMenuOptionCount));

      switch (menuSelection)
      {
      case MenuOption::SetDate:
        setDateMenu();
        continue;

      case MenuOption::SetTime:
        setTimeMenu();
        continue;

      case MenuOption::Back:
        return;
      }
    }
    while (true);
  }

  void setTimeMenu()
  {
    uint8_t hour = OSCR::UI::rangeSelect(F("SET HOUR"), 0, 23, rtc.now().hour());
    uint8_t minute = OSCR::UI::rangeSelect(F("SET MINUTE"), 0, 59, rtc.now().minute());
    uint8_t second = OSCR::UI::rangeSelect(F("SET SECOND"), 0, 59, rtc.now().second());

    setTime(hour, minute, second);
  }

  void setDateMenu()
  {
    uint16_t year = OSCR::UI::rangeSelect(F("SET YEAR"), 2025, 2099, rtc.now().year());
    uint8_t month = OSCR::UI::rangeSelect(F("SET MONTH"), 1, 12, rtc.now().month());
    uint8_t day = OSCR::UI::rangeSelect(F("SET DAY"), 1, daysInMonth(month), rtc.now().day());

    setDate(year, month, day);
  }
# else

  constexpr uint32_t const kSecondsMinute = 60;
  constexpr uint32_t const kSecondsHour = 60 * kSecondsMinute;
  constexpr uint32_t const kSecondsDay = 24 * kSecondsHour;

  void startMeasure()
  {
    eventStarted = millis();
  }

  char * getDifference(char * diffStr, size_t length)
  {
    if (eventStarted == 0)
    {
      strncpy_P(diffStr, PSTR("? sec"), length);
      return diffStr;
    }

    uint32_t difference = millis() - eventStarted;

    uint8_t days = floor(difference / kSecondsDay);
    if (days > 0) difference -= days * kSecondsDay;

    uint8_t hours = floor(difference / kSecondsHour);
    if (hours > 0) difference -= hours * kSecondsHour;

    uint8_t minutes = floor(difference / kSecondsMinute);
    if (minutes > 0) difference -= minutes * kSecondsMinute;

    uint8_t seconds = difference;

    if (days > 0)
    {
      snprintf_P(diffStr, length, PSTR("%" PRIu8 " dy, %" PRIu8 " hr, %" PRIu8 " min, %" PRIu8 " sec"), days, hours, minutes, seconds);
    }
    else if (hours > 0)
    {
      snprintf_P(diffStr, length, PSTR("%" PRIu8 " hr, %" PRIu8 " min, %" PRIu8 " sec"), hours, minutes, seconds);
    }
    else if (minutes > 0)
    {
      snprintf_P(diffStr, length, PSTR("%" PRIu8 " min, %" PRIu8 " sec"), minutes, seconds);
    }
    else
    {
      snprintf_P(diffStr, length, PSTR("%" PRIu8 " sec"), seconds);
    }

    return diffStr;
  }

  void printDifference()
  {
    char diffStr[32];
    OSCR::UI::printLine(getDifference(BUFFN(diffStr)));
  }
#endif /* ENABLE_RTC */
} /* namespace OSCR::Time */

#endif /* OSCR_ARCH_AVR  */
