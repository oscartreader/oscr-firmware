/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#if !defined(OSCR_SERIAL_ASCII_H_)
# define OSCR_SERIAL_ASCII_H_

# include "config.h"

# if defined(ENABLE_SERIAL_ASCII)

namespace OSCR
{
  namespace UI
  {
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
  }
}

# endif /* ENABLE_SERIAL_ASCII */

#endif /* !OSCR_SERIAL_ASCII_H_ */
