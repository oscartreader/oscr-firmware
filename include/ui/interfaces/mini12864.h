/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#if !defined(OSCR_MINI12864_H_)
# define OSCR_MINI12864_H_

# include "config.h"

# if (HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864)
#   include <U8g2lib.h>
#   include "common/Util.h"
#   include "hardware/peripherals/NeoPixel.h"

#   define HAS_OUTPUT_LINE_ADJUSTMENTS true
#   define HAS_OUTPUT_LINE_ALIGNMENT true

namespace OSCR
{
  namespace UI
  {
    /**
     * The U8G2 instance for communicating with the MINI12864
     *
     * @note
     * Requires MINI12864
     */
    extern U8G2_ST7567_OS12864_F_4W_HW_SPI display;

    //! @cond

    inline constexpr bool const kSupportsLineAdjustments = true;
    inline constexpr bool const kSupportsLineAlignment = true;

    inline constexpr uint8_t const kDisplayWidth = 128;
    inline constexpr uint8_t const kDisplayHeight = 64;

    inline constexpr uint8_t const kLineHeight = 8;

    inline constexpr uint8_t const kDisplayLines = 8;
    inline constexpr uint8_t const kDisplayLineStart = 8;

    extern void update();
    extern void clear();

    extern void printCenter(char const * text);
    extern void printCenter_P(char const * flashText);
    extern void printRight(char const * text);
    extern void printRight_P(char const * flashText);

    template <bool sync,
              typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
    extern void print(T string);

    template <bool sync,
              typename T,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_number<T>::value), bool> Enable,
              OSCR::Util::enable_if_t<!(OSCR::Util::is_printable<T>::value), bool> NonPrintable>
    extern void printLine(T string);

    template <bool sync,
              typename Tint,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
    extern void print(Tint number, int base);

    template <bool sync,
              typename Tint,
              OSCR::Util::enable_if_t<OSCR::Util::is_integer<Tint>::value, bool> Enable>
    extern void printLine(Tint number, int base);

    //! @endcond
  }
}

# endif /* OUTPUT_OS12864 */
#endif /* !OSCR_MINI12864_H_ */
