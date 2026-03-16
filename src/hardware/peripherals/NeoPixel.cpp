#include "common.h"

#if defined(ENABLE_NEOPIXEL)

namespace OSCR
{
  namespace UI
  {
    namespace NeoPixel
    {
      /**
       * The NeoPixel instance for controlling the color of the NeoPixel.
       */
      Adafruit_NeoPixel pixels(3, 13, NEO_GRB + NEO_KHZ800);

      /**
       * The current colors.
       */
      class {
      public:
        rgb_t off = {
          0,
          0,
          0,
        };

        rgb_t normal = {
          OPTION_LCD_BG_RED,
          OPTION_LCD_BG_GREEN,
          OPTION_LCD_BG_BLUE,
        };

        rgb_t notification = {
          OPTION_LCD_NOTIF_RED,
          OPTION_LCD_NOTIF_GREEN,
          OPTION_LCD_NOTIF_BLUE,
        };

        rgb_t error = {
          OPTION_LCD_ERROR_RED,
          OPTION_LCD_ERROR_GREEN,
          OPTION_LCD_ERROR_BLUE,
        };

        rgb_t background = {
          OPTION_LCD_BG_RED,
          OPTION_LCD_BG_GREEN,
          OPTION_LCD_BG_BLUE,
        };

        rgb_t button = {
          OPTION_LCD_BG_RED,
          OPTION_LCD_BG_GREEN,
          OPTION_LCD_BG_BLUE,
        };

        NeoPixelState buttonState = NeoPixelState::Normal;
        NeoPixelState backgroundState = NeoPixelState::Normal;

        NeoPixelState buttonOffState = NeoPixelState::Normal;
        NeoPixelState backgroundOffState = NeoPixelState::Normal;

        void setStates(NeoPixelState state)
        {
          buttonState = state;
          backgroundState = state;
        }

        NeoPixelState getStates()
        {
          if (buttonState != backgroundState) return NeoPixelState::Mixed;

          return buttonState;
        }
      } pixelColors;

      void config()
      {
#if defined(ENABLE_CONFIG)
        // Check if config is ready
        if (!OSCR::Configuration::enabled())
        {
          // Register this function to be called when config is ready
          OSCR::Configuration::registerCallback(config);
          return;
        }

        bool confNormalColor = false;
        bool confNotifColor = false;
        bool confErrorColor = false;

        OSCR::Configuration::getBool(F("lcd.normal.color"), confNormalColor);
        OSCR::Configuration::getBool(F("lcd.notif.color"), confNotifColor);
        OSCR::Configuration::getBool(F("lcd.error.color"), confErrorColor);

        if (confNormalColor)
        {
          if (!OSCR::Configuration::getInteger(F("lcd.normal.red"),   pixelColors.normal.red))
            pixelColors.normal.red = 0;

          if (!OSCR::Configuration::getInteger(F("lcd.normal.green"), pixelColors.normal.green))
            pixelColors.normal.green = 0;

          if (!OSCR::Configuration::getInteger(F("lcd.normal.blue"),  pixelColors.normal.blue))
            pixelColors.normal.blue = 0;
        }

        if (confNotifColor)
        {
          if (!OSCR::Configuration::getInteger(F("lcd.notif.red"),    pixelColors.notification.red))
            pixelColors.notification.red = 0;

          if (!OSCR::Configuration::getInteger(F("lcd.notif.green"),  pixelColors.notification.green))
            pixelColors.notification.green = 0;

          if (!OSCR::Configuration::getInteger(F("lcd.notif.blue"),   pixelColors.notification.blue))
            pixelColors.notification.blue = 0;
        }

        if (confErrorColor)
        {
          if (!OSCR::Configuration::getInteger(F("lcd.error.red"),    pixelColors.error.red))
            pixelColors.error.red = 0;

          if (!OSCR::Configuration::getInteger(F("lcd.error.green"),  pixelColors.error.green))
            pixelColors.error.green = 0;

          if (!OSCR::Configuration::getInteger(F("lcd.error.blue"),   pixelColors.error.blue))
            pixelColors.error.blue = 0;
        }
#endif /* ENABLE_CONFIG */
      }

      void setup()
      {
#if defined(ENABLE_3V3FIX)
        // Set clock high during setup
        OSCR::Clock::setClockSpeed(ClockSpeed::k16MHz);
        delay(10);
#endif /* ENABLE_3V3FIX */

        pixels.begin();
        pixels.setBrightness(255);

        updateColors();
      }

      void off()
      {
        bool needsUpdate = false;

        switch (pixelColors.backgroundState)
        {
        default:
          pixelColors.backgroundOffState = pixelColors.backgroundState;
          /* fall through */
        case NeoPixelState::Dim:
          needsUpdate = true;
          pixelColors.backgroundState = NeoPixelState::Off;
          break;

        case NeoPixelState::Off:
          break;
        }

        switch (pixelColors.buttonState)
        {
        default:
          pixelColors.buttonOffState = pixelColors.buttonState;
          /* fall through */
        case NeoPixelState::Dim:
          needsUpdate = true;
          pixelColors.buttonState = NeoPixelState::Off;
          break;

        case NeoPixelState::Off:
          break;
        }

        if (needsUpdate) updateColors();
      }

      void dim()
      {
        bool needsUpdate = false;

        switch (pixelColors.backgroundState)
        {
        default:
          pixelColors.backgroundOffState = pixelColors.backgroundState;
          /* fall through */
        case NeoPixelState::Off:
          needsUpdate = true;
          pixelColors.backgroundState = NeoPixelState::Dim;
          break;

        case NeoPixelState::Dim:
          break;
        }

        switch (pixelColors.buttonState)
        {
        default:
          pixelColors.buttonOffState = pixelColors.buttonState;
          /* fall through */
        case NeoPixelState::Off:
          needsUpdate = true;
          pixelColors.buttonState = NeoPixelState::Dim;
          break;

        case NeoPixelState::Dim:
          break;
        }

        if (needsUpdate) updateColors();
      }

      void on()
      {
        bool needsUpdate = false;

        switch (pixelColors.backgroundState)
        {
        case NeoPixelState::Dim:
        case NeoPixelState::Off:
          pixelColors.backgroundState = pixelColors.backgroundOffState;
          needsUpdate = true;
          break;

        default:
          break;
        }

        switch (pixelColors.buttonState)
        {
        case NeoPixelState::Dim:
        case NeoPixelState::Off:
          pixelColors.buttonState = pixelColors.buttonOffState;
          needsUpdate = true;
          break;

        default:
          break;
        }

        if (needsUpdate) updateColors();
      }

      void setBackgroundColor(rgb_t rgb)
      {
        pixelColors.background = rgb;
        pixelColors.backgroundState = NeoPixelState::Custom;
      }

      void resetBackgroundColor()
      {
        pixelColors.background = pixelColors.normal;
        pixelColors.backgroundState = NeoPixelState::Normal;
      }

      void setButtonColor(rgb_t rgb)
      {
        pixelColors.button = rgb;
        pixelColors.buttonState = NeoPixelState::Custom;
      }

      void resetButtonColor()
      {
        pixelColors.button = pixelColors.normal;
        pixelColors.buttonState = NeoPixelState::Normal;
      }

      void setColors(rgb_t rgb)
      {
        pixelColors.background = rgb;
        pixelColors.button = rgb;
        pixelColors.setStates(NeoPixelState::Custom);
      }

      void setColors(rgb_t background, rgb_t button)
      {
        pixelColors.background = background;
        pixelColors.button = button;
        pixelColors.setStates(NeoPixelState::Custom);
      }

      void resetColors()
      {
        pixelColors.background = pixelColors.normal;
        pixelColors.button = pixelColors.normal;
        pixelColors.setStates(NeoPixelState::Normal);
      }

      void useNotificationColor()
      {
        pixelColors.background = pixelColors.normal;
        pixelColors.button = pixelColors.notification;
        pixelColors.setStates(NeoPixelState::Notification);
        updateColors();
      }

      void useErrorColor(bool fatal)
      {
        pixelColors.button = pixelColors.error;
        pixelColors.background = (fatal) ? pixelColors.error : pixelColors.normal;
        pixelColors.setStates(NeoPixelState::Error);
        updateColors();
      }

      void useNormalColor()
      {
        pixelColors.button = pixelColors.normal;
        pixelColors.background = pixelColors.normal;
        pixelColors.setStates(NeoPixelState::Normal);
        updateColors();
      }

      void updateColors()
      {
        rgb_t updatedBackground = pixelColors.background;
        rgb_t updatedButton = pixelColors.button;

        if (pixelColors.backgroundState == NeoPixelState::Off)
        {
          updatedBackground = pixelColors.off;
        }
        else if (pixelColors.backgroundState == NeoPixelState::Dim)
        {
          updatedBackground.green /= 2;
          updatedBackground.red /= 2;
          updatedBackground.blue /= 2;
        }

        if (pixelColors.buttonState == NeoPixelState::Off)
        {
          updatedButton = pixelColors.off;
        }
        else if (pixelColors.buttonState == NeoPixelState::Dim)
        {
          updatedButton.green /= 2;
          updatedButton.red /= 2;
          updatedButton.blue /= 2;
        }

        pixels.clear();

        pixels.setPixelColor(PIXEL_BG, updatedBackground.green, updatedBackground.red, updatedBackground.blue);
        pixels.setPixelColor(PIXEL_BTN1, updatedButton.green, updatedButton.red, updatedButton.blue);
        pixels.setPixelColor(PIXEL_BTN2, updatedButton.green, updatedButton.red, updatedButton.blue);

        pixels.show();
      }

      bool isOff()
      {
        return (pixelColors.getStates() == NeoPixelState::Off);
      }

      bool isDim()
      {
        return (pixelColors.getStates() == NeoPixelState::Dim);
      }

      bool isNormal()
      {
        return (pixelColors.getStates() == NeoPixelState::Normal);
      }

      bool isNotification()
      {
        return (pixelColors.getStates() == NeoPixelState::Notification);
      }

      bool isError()
      {
        return (pixelColors.getStates() == NeoPixelState::Error);
      }

      bool isCustom()
      {
        return (pixelColors.getStates() == NeoPixelState::Custom);
      }

      NeoPixelState getState()
      {
        return pixelColors.getStates();
      }
    } /* namespace NeoPixel */
  } /* namespace UI */
} /* namespace OSCR */

#endif /* ENABLE_NEOPIXEL */
