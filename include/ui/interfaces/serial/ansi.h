/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#if !defined(OSCR_SERIAL_ANSI_H_)
# define OSCR_SERIAL_ANSI_H_

# include "config.h"

# if defined(ENABLE_SERIAL_ANSI)
#   include "hardware/outputs/SerialANSI.h"

#   define HAS_OUTPUT_LINE_ADJUSTMENTS true
#   define HAS_OUTPUT_LINE_ALIGNMENT false

namespace OSCR
{
  namespace UI
  {
    //! @cond

    inline constexpr bool const kSupportsLineAdjustments = true;
    inline constexpr bool const kSupportsLineAlignment = false;

    inline constexpr uint8_t const kDisplayWidth = 0;
    inline constexpr uint8_t const kDisplayHeight = 0;

    inline constexpr uint8_t const kLineHeight = 0;

    inline constexpr uint8_t const kDisplayCols = 57;
    inline constexpr uint8_t const kDisplayRows = 20;

    inline constexpr uint8_t const kDisplayLines = UI_PAGE_SIZE;
    inline constexpr uint8_t const kDisplayLineStart = 0;

    inline constexpr uint8_t const kPageRowMax = (UI_PAGE_SIZE > kDisplayRows) ? kDisplayRows : UI_PAGE_SIZE;

    /**
     * Define if a menu is active.
     */
    extern bool menuActive;

    /**
     * The user's selection from a menu.
     */
    extern uint8_t menuSelection;

    /**
     * The user's selection from a non-menu. This allows non-menu methods to ask for numerical
     * input, such as the size of a cartridge, etc. It's split out so they don't need to store
     * the result.
     */
    extern uint8_t selection;

    extern void update();
    extern void clear();

    //! @endcond
  }
}

# endif /* ENABLE_SERIAL_ANSI */

#endif /* !OSCR_SERIAL_ANSI_H_ */
