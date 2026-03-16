
#include "common.h"
#include "hardware/peripherals/ClockGen.h"

namespace OSCR
{
  namespace ClockGen
  {
#ifdef ENABLE_CLOCKGEN
    Si5351 clockgen;
    __constinit bool i2c_found = false;
    __constinit int32_t clock_offset = 0;
#endif

    bool initialize()
    {
#if defined(ENABLE_CLOCKGEN)
      if (i2c_found) return true;

# ifdef OPTION_CLOCKGEN_USE_CALIBRATION
      if (!Calibration::readClockOffset(clock_offset))
      {
        FsFile clock_file;

        if (clock_file.open("/snes_clk.txt", O_WRITE | O_CREAT | O_TRUNC))
        {
          clock_file.write("0", 1);
          clock_file.close();
        }
      }
# endif

      i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, clock_offset);

      if (i2c_found) stop();

      return i2c_found;
#else
      return false;
#endif
    }

    void stop()
    {
#if defined(ENABLE_CLOCKGEN)
      if (!i2c_found) return;

      clockgen.output_enable(SI5351_CLK0, 0);
      clockgen.output_enable(SI5351_CLK1, 0);
      clockgen.output_enable(SI5351_CLK2, 0);

      clockgen.update_status();
#endif
    }

    bool exists()
    {
#if defined(ENABLE_CLOCKGEN)
      return i2c_found;
#else
      return false;
#endif
    }

    bool existsIfEnabled()
    {
#if defined(ENABLE_CLOCKGEN)
      return i2c_found;
#else
      return true;
#endif
    }

# if defined(OPTION_CLOCKGEN_CALIBRATION) || defined(OPTION_CLOCKGEN_USE_CALIBRATION)
    /******************************************
      Clockgen Calibration
     *****************************************/
    namespace Calibration
    {
      int32_t cal_factor = 0;
      int32_t old_cal = 0;
      int32_t cal_offset = 100;

      bool readClockOffset(int32_t & clock_offset)
      {
        FsFile clock_file;

        if (!clock_file.open("/snes_clk.txt", O_READ))
        {
          return false;
        }

        char clock_buf[12] = {};

        int8_t read = clock_file.read(clock_buf, 11);

        clock_file.close();

        if ((read == -1) || ((read == 11) && (clock_buf[0] != '-')))
        {
          return false;
        }

        for (uint8_t i = 0; i < 12; i++)
        {
          if (clock_buf[i] != '-' && clock_buf[i] < '0' && clock_buf[i] > '9')
          {
            if ((i == 0) || ((i == 1) && (clock_buf[0] == '-')))
            {
              return false;
            }
            else
            {
              clock_buf[i] = 0;
              break;
            }
          }
        }

        clock_offset = atol(clock_buf);

        return true;
      }

#   ifdef OPTION_CLOCKGEN_CALIBRATION
      void clkcal()
      {
        // Adafruit Clock Generator
        // last number is the clock correction factor which is custom for each clock generator
        cal_factor = readClockOffset();

        OSCR::UI::clear();
        OSCR::UI::print(F("Read correction: "));
        OSCR::UI::printLineSync(cal_factor);

        delay(500);

        if (cal_factor > INT32_MIN)
        {
          i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, cal_factor);
        }
        else
        {
          i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
          cal_factor = 0;
        }

        if (!i2c_found)
        {
          OSCR::UI::clear();
          OSCR::UI::fatalError(FS(OSCR::Strings::Errors::ClockGenMissing));
        }

        //clockgen.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
        clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
        clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
        //clockgen.pll_reset(SI5351_PLLA);
        //clockgen.pll_reset(SI5351_PLLB);
        clockgen.set_freq(400000000ULL, SI5351_CLK0);
        clockgen.set_freq(100000000ULL, SI5351_CLK1);
        clockgen.set_freq(307200000ULL, SI5351_CLK2);
        clockgen.output_enable(SI5351_CLK1, 1);
        clockgen.output_enable(SI5351_CLK2, 1);
        clockgen.output_enable(SI5351_CLK0, 1);

        // Frequency Counter
        delay(500);
        FreqCount.begin(1000);

        while (1)
        {
          if (old_cal != cal_factor)
          {
            OSCR::UI::clear();
            OSCR::UI::printLineSync(F("Adjusting..."));

            clockgen.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
            clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
            clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
            clockgen.pll_reset(SI5351_PLLA);
            clockgen.pll_reset(SI5351_PLLB);
            clockgen.set_freq(400000000ULL, SI5351_CLK0);
            clockgen.set_freq(100000000ULL, SI5351_CLK1);
            clockgen.set_freq(307200000ULL, SI5351_CLK2);

            old_cal = cal_factor;

            delay(500);
          }
          else
          {
            clockgen.update_status();

            while (clockgen.dev_status.SYS_INIT == 1);

            if (FreqCount.available())
            {
              float count = FreqCount.read();
              OSCR::UI::clear();

              OSCR::UI::printLine(F("Clock Calibration"));

              OSCR::UI::print(F("Freq:   "));
              OSCR::UI::print(count);
              OSCR::UI::printLine(F("Hz"));

              OSCR::UI::print(F("Correction:"));
              OSCR::UI::printLine(String(cal_factor));

              OSCR::UI::print(F("Step:"));
              OSCR::UI::print(cal_offset);
              OSCR::UI::printLine();

#     if (HARDWARE_INPUT_TYPE == INPUT_1BUTTON)
              OSCR::UI::printLineSync(F("Click/dbl to adjust"));

#     elif (HARDWARE_INPUT_TYPE == INPUT_2BUTTON)

              OSCR::UI::printLine(F("(Hold button to save)"));
              OSCR::UI::printLine();
              OSCR::UI::printLineSync(F("Decrease     Increase"));

#     elif (HARDWARE_INPUT_TYPE == INPUT_ROTARY)

              OSCR::UI::printLine(F("Rotate to adjust Frequency"));
              OSCR::UI::printLine(F("Press to change step width"));
              OSCR::UI::printLineSync(F("Hold to save"));

#     endif
            }

#     if (HARDWARE_INPUT_TYPE == INPUT_2BUTTON)
            // get input button
            uint8_t a = checkButton1();
            uint8_t b = checkButton2();

            // if the cart readers input button is pressed shortly
            if (a == 1) {
              old_cal = cal_factor;
              cal_factor -= cal_offset;
            }
            if (b == 1) {
              old_cal = cal_factor;
              cal_factor += cal_offset;
            }

            // if the cart readers input buttons is double clicked
            if (a == 2) {
              cal_offset /= 10L;
              if (cal_offset < 1) {
                cal_offset = 100000000L;
              }
            }
            if (b == 2) {
              cal_offset *= 10L;
              if (cal_offset > 100000000L) {
                cal_offset = 1;
              }
            }

            // if the cart readers input button is pressed long
            if (a == 3) {
              savetofile();
            }
            if (b == 3) {
              savetofile();
            }
#     else
            //Handle inputs for either rotary encoder or single button interface.
            uint8_t a = checkButton();

            if (a == 1) {  //clockwise rotation or single click
              old_cal = cal_factor;
              cal_factor += cal_offset;
            }

            if (a == 2) {  //counterclockwise rotation or double click
              old_cal = cal_factor;
              cal_factor -= cal_offset;
            }

            if (a == 3) {  //button short hold
              cal_offset *= 10L;
              if (cal_offset > 100000000L) {
                cal_offset = 1;
              }
            }

            if (a == 4) {  //button long hold
              savetofile();
            }
#     endif
          }
        }
      }

      void savetofile()
      {
        OSCR::UI::clear();

        OSCR::UI::printLine(F("Saving..."));
        OSCR::UI::printLineSync(cal_factor);

        delay(2000);

        if (!sharedFile.open("/snes_clk.txt", O_WRITE | O_CREAT | O_TRUNC))
        {
          OSCR::UI::fatalError(FS(OSCR::Strings::Errors::StorageError));
        }

        sharedFile.print(cal_factor);

        sharedFile.close();

        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::DONE));

        delay(1000);

        resetArduino();
      }
#   endif /* OPTION_CLOCKGEN_CALIBRATION */
    } /* namespace Calibration */
# endif /* OPTION_CLOCKGEN_CALIBRATION || OPTION_CLOCKGEN_USE_CALIBRATION */
  } /* namespace ClockGen */
} /* namespace OSCR */
