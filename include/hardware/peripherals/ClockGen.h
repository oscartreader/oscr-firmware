/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#if !defined(OSCR_CLOCKGEN_H_)
# define OSCR_CLOCKGEN_H_

# include <si5351.h>
# include "syslibinc.h"

// Clockgen Calibration
# ifdef OPTION_CLOCKGEN_CALIBRATION
#   include <FreqCount.h>
# endif

namespace OSCR
{
  namespace ClockGen
  {
#ifdef ENABLE_CLOCKGEN
    extern Si5351 clockgen;
#endif

    /**
     * Initilize the clock generator.
     *
     * @return `true` if present, `false` if missing or disabled.
     */
    extern bool initialize();

    /**
     * Stop all clock outputs.
     */
    extern void stop();

    /**
     * Check if the clock generator is present. If it is missing or
     * disabled, this will return `false`. Use this when the clock
     * generator is required.
     */
    extern bool exists();

    /**
     * Check if the clock generator is present if it's enabled. If it
     * is not enabled, this always returns `true`. Use this when the
     * clock generator being enabled is optional.
     */
    extern bool existsIfEnabled();
  }
}

#endif /* OSCR_CLOCKGEN_H_ */
