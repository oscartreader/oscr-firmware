/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#if !defined(OSCR_NEOPIXEL_H_)
# define OSCR_NEOPIXEL_H_

# include "syslibinc.h"
# include "config.h"

# if defined(ENABLE_NEOPIXEL)

#   include <Adafruit_NeoPixel.h>
#   include "common/Types.h"

namespace OSCR
{
  namespace UI
  {
    /**
     * @brief NeoPixel-specific methods and variables.
     *
     * Methods and variables for the Adafruit %NeoPixel which is
     * usually used by the %MINI12864
     */
    namespace NeoPixel
    {
      enum class NeoPixelState : uint8_t
      {
        Off,
        Dim,
        Normal,
        Notification,
        Error,
        Custom,
        Mixed,
      };

      extern Adafruit_NeoPixel pixels;

      extern void config();

      /**
       * Setup the NeoPixel
       *
       * @note
       * Requires a NeoPixel.
       */
      extern void setup();

      /**
       * Turn off the NeoPixels.
       */
      extern void off();

      /**
       * Dim the NeoPixels.
       */
      extern void dim();

      /**
       * Turn on the NeoPixels.
       */
      extern void on();

      /**
       * Set the current NeoPixel background color.
       */
      extern void setBackgroundColor(rgb_t rgb);

      /**
       * Reset the LCD background color back to default.
       *
       * @note
       * Requires NeoPixel
       */
      extern void resetBackgroundColor();

      /**
       * Set the LCD button color.
       *
       * @note
       * Requires NeoPixel
       */
      extern void setButtonColor(rgb_t rgb);

      /**
       * Set the LCD button color.
       *
       * @note
       * Requires NeoPixel
       */
      extern void setColors(rgb_t background, rgb_t button);

      /**
       * Reset the LCD button color back to default.
       *
       * @note
       * Requires MINI12864
       */
      extern void resetButtonColor();

      /**
       * Use the notification colors.
       *
       * @note
       * Requires MINI12864
       */
      extern void useNotificationColor();

      /**
       * Use the error colors.
       *
       * @note
       * Requires MINI12864
       */
      extern void useErrorColor(bool fatal = false);

      /**
       * Use the default colors.
       *
       * @note
       * Requires MINI12864
       */
      extern void useNormalColor();

      /**
       * Reset colors back to default.
       *
       * @note
       * Requires MINI12864
       */
      extern void resetColors();

      /**
       * Update/refresh the LCD's colors.
       *
       * @note
       * Requires MINI12864
       */
      extern void updateColors();

      /**
       * Check if the colors are off.
       */
      extern bool isOff();

      /**
       * Check if the colors are dimmed.
       */
      extern bool isDim();

      /**
       * Check if the colors are normal colors.
       */
      extern bool isNormal();

      /**
       * Check if the colors are notification colors.
       */
      extern bool isNotification();

      /**
       * Check if the colors are error colors.
       */
      extern bool isError();

      /**
       * Check if the colors are custom colors.
       */
      extern bool isCustom();

      /**
       * Get current color state.
       */
      extern NeoPixelState getState();
    }
  }
}

# endif /* ENABLE_NEOPIXEL */

#endif /* OSCR_NEOPIXEL_H_ */
