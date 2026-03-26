/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_TIME_H_
#define OSCR_TIME_H_

#include "config.h"

# if defined(ENABLE_RTC)
#   include "syslibinc.h"
#   include <RTClib.h>
# endif /* ENABLE_RTC */

namespace OSCR
{
  namespace Time
  {
# if defined(ENABLE_RTC)

#   if (RTC_TYPE == RTCOPT_DS3231)
    extern RTC_DS3231 rtc;
#   elif (RTC_TYPE == RTCOPT_DS1307)
    extern RTC_DS1307 rtc;
#   endif

    extern const uint8_t kBCDYear;
    extern const uint8_t kBCDMonth;
    extern const uint8_t kBCDDay;
    extern const uint8_t kBCDWkd;
    extern const uint8_t kBCDHour;
    extern const uint8_t kBCDMinute;
    extern const uint8_t kBCDSecond;

# endif /* ENABLE_RTC */

    extern bool setup();

# if defined(ENABLE_RTC)

    extern DateTime now();

    extern bool setDateTime(DateTime const & dt);
    extern bool setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);

    extern char * RTCStamp(char * time, uint8_t size = 21);

    extern uint8_t daysInMonth(uint8_t month);

    extern void menu();
    extern void setTimeMenu();
    extern void setDateMenu();

    extern bool exists();

# else /* !ENABLE_RTC */

    inline constexpr bool exists()
    {
      return false;
    }

# endif /* ENABLE_RTC */

    extern void startMeasure();
    extern void endMeasure();
    extern char * getDifference(char * diffStr, size_t length);
    extern void printDifference();
  } /* namespace Time */
} /* namespace OSCR */

#endif /* OSCR_TIME_H_ */
