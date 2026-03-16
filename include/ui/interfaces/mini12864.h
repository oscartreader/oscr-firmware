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

    extern void update();
    extern void clear();

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
  }
}

# endif /* OUTPUT_OS12864 */
#endif /* !OSCR_MINI12864_H_ */
