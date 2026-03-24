/********************************************************************
*                   Open Source Cartridge Reader                    */
/*H******************************************************************
* FILENAME :        Voltage.cpp
*
* DESCRIPTION :
*       Contains the control functions for voltage and clock speed.
*
* PUBLIC FUNCTIONS :
*       void    setClockScale( ClockSpeed )
*       Voltage setVoltage( Voltage )
*
* NOTES :
*       This file is a WIP, I've been moving things into it on my local working
*       copy, but they are not ready yet. Rather than put this in the main file
*       only to move them back again, I decided to commit it early. If you need
*       to add new globals, enums, defines, etc, please use this file!
*
* LICENSE :
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation, either version 3 of the License, or
*       (at your option) any later version.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License
*       along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
* CHANGES :
*
* REF NO    VERSION  DATE        WHO            DETAIL
*           20.0     2025-05-24  Ancyker        Initial version
*
*H*/

#include "arch/avr/syslibinc.h"

#if defined(ARDUINO_AVR_MEGA2560)

# include "common/Types.h"
# include "common/Power.h"
# include "ui.h"
# include "hardware/peripherals/ClockGen.h"

// Local globals that should be unavailable outside this file.
namespace
{
  // MCU clock speeds
  constexpr uint32_t const kClock16M = 16000000UL;  /*!< 16MHz clock speed */
  constexpr uint32_t const kClock8M = 8000000UL;    /*!< 8MHz clock speed */

# if defined(ENABLE_VSELECT)

  /**
   * Voltage lock toggle -- allows forcing a voltage.
   */
  __constinit bool voltageLocked = false;

# endif /* ENABLE_VSELECT */

  /**
   * The current voltage.
   */
  __constinit OSCR::Voltage currentVoltage = OSCR::Voltage::Unknown;

  /**
   * The current clock speed.
   */
  __constinit OSCR::ClockSpeed currentClockSpeed = OSCR::ClockSpeed::k16MHz;

# if defined(ENABLE_POWERSAVING)

  __constinit uint32_t lastInput = 0;
  __constinit OSCR::SleepState sleepState = OSCR::SleepState::Awake;

#   if (OPTION_POWERSAVING_METHOD & POWERSAVING_CARTBUS_OFF)
  __constinit bool wasCartBusOn = false;
#   endif

#   if defined(ENABLE_VSELECT) && (OPTION_POWERSAVING_METHOD & POWERSAVING_SLOWCLOCK)
  __constinit OSCR::Voltage wasVoltage = OSCR::Voltage::Unknown;
#   endif /* ENABLE_VSELECT */

# endif /* ENABLE_POWERSAVING */
}

# define currentClockRate ((currentClockSpeed == ClockSpeed::k16MHz) ? kClock16M : kClock8M)
# define scaleToClockRate(SCALE) ((SCALE == ClockSpeed::k16MHz) ? kClock16M : kClock8M)

namespace OSCR
{
  namespace Clock
  {
    /**
     * @internal
     *
     * Set the ATMEGA2560 clock prescaler. This shouldn't normally be called directly.
     *
     * @param clockScale  Clock scale
     *
     * @cond PROCESS
     *   [1]  Begin an atomic operation
     *   [2]  Adjust the UART clock
     *   [3]  Enable clock prescaler changes
     *   [4]  Apply new clock prescaler
     * @endcond
     *
     * @note
     * Changing the clock prescaler to a value other than F_CPU (1 by default)
     * can/will result in some clock-based functions not working, including
     * timers and most communication protocols.
     *
     * @sa [ATmega640/V-1280/V-1281/V-2560/V-2561/V § 10.13.2](https://rmy.pw/atmega2560)
     * @sa setClockSpeed
     *
     * @endinternal
     */
    void setClockScale(ClockSpeed clockScale)
    {
      if (clockScale == currentClockSpeed) return;

      ATOMIC_BLOCK(ATOMIC_FORCEON) /*[1]*/
      {
# if defined(ENABLE_SERIAL_OUTPUT) || defined(ENABLE_UPDATER)
        ClockedSerial.clockSkew(scaleToClockRate(clockScale)); /*[2]*/
# endif /* ENABLE_SERIAL_OUTPUT || ENABLE_UPDATER */

        currentClockSpeed = (clockScale == ClockSpeed::k16MHz ? ClockSpeed::k16MHz : ClockSpeed::k8MHz);

        uint8_t __tmp = _BV(CLKPCE); /*[3]*/
        __asm__ __volatile__ (
          "in __tmp_reg__,__SREG__" "\n\t"
          "cli" "\n\t"
          "sts %1, %0" "\n\t"
          "sts %1, %2" "\n\t"
          "out __SREG__, __tmp_reg__"
          : /* no outputs */
          : "d" (__tmp),
          "M" (_SFR_MEM_ADDR(CLKPR)),
          "d" (clockScale)
          : "r0"
        ); /*[4]*/
      }
    }

    ModuleResult setClockSpeed(ClockSpeed newClock __attribute__((unused)))
    {
      switch (newClock)
      {
      case ClockSpeed::k8MHz: /* 8MHz */
        setClockScale(ClockSpeed::k8MHz);
        return ModuleResult::Success;

      case ClockSpeed::k16MHz: /* 16MHz */
        setClockScale(ClockSpeed::k16MHz);
        return ModuleResult::Success;
      }
    }

    ClockSpeed getClockSpeed()
    {
      return currentClockSpeed;
    }

    uint32_t getClock()
    {
      return (currentClockSpeed == ClockSpeed::k16MHz ? kClock16M : kClock8M);
    }
  }

  namespace Power
  {
    constexpr uint16_t const kIdleDrowsy  = OPTION_POWERSAVING_IDLE_DIM;
    constexpr uint16_t const kIdleSleep   = OPTION_POWERSAVING_IDLE_SLEEP;

# if defined(ENABLE_ONBOARD_ATMEGA)
    Voltage getVoltage()
    {
#   if !defined(OPTION_VOLTAGE_MONITOR_METHOD) || OPTION_VOLTAGE_MONITOR_METHOD == 0
      return currentVoltage;
#   elif OPTION_VOLTAGE_MONITOR_METHOD == 1
      return ((PINJ & (1 << 4)) ? Voltage::k5V : Voltage::k3V3);
#   elif OPTION_VOLTAGE_MONITOR_METHOD == 2
      return ((PINE & (1 << 6)) ? Voltage::k5V : Voltage::k3V3);
#   endif
    }

    Voltage getVoltageSelect()
    {
      return ((PINE & (1 << 6)) ? Voltage::k5V : Voltage::k3V3);
    }

    bool checkVoltage(Voltage voltage)
    {
      return (PINJ & (1 << (voltage == Voltage::k5V ? 5 : 6)));
    }

    bool voltagesOk()
    {
#   if !defined(OPTION_VOLTAGE_MONITOR_METHOD) || (OPTION_VOLTAGE_MONITOR_METHOD == 0)
      return true;
#   else
      return checkVoltage(Voltage::k5V) && checkVoltage(Voltage::k3V3) &&
#     if (OPTION_VOLTAGE_MONITOR_METHOD == 1) || (OPTION_VOLTAGE_MONITOR_METHOD == 3)
              (((PINJ & (1 << 4)) ? Voltage::k5V : Voltage::k3V3) == currentVoltage)
#     endif
#     if (OPTION_VOLTAGE_MONITOR_METHOD == 3)
              &&
#     endif
#     if (OPTION_VOLTAGE_MONITOR_METHOD == 2) || (OPTION_VOLTAGE_MONITOR_METHOD == 3)
              (((PINE & (1 << 6)) ? Voltage::k5V : Voltage::k3V3) == currentVoltage)
#     endif
#   endif
      ;
    }

    bool enableCartridge(bool abortIncorrectVoltage)
    {
      if (getVoltage() != currentVoltage)
      {
        if (!abortIncorrectVoltage) return false;
        OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::FatalError));
        OSCR::UI::fatalError(FS(OSCR::Strings::Errors::IncorrectVoltage));
      }
      else
      {
        // Enable CBUS
        PORTJ |= (1 << 7);
        return true;
      }
    }

    bool cartridgeEnabled()
    {
      return (PINJ & (1 << 7));
    }
# else /* !ENABLE_ONBOARD_ATMEGA */
    //! @cond
    Voltage getVoltage()
    {
      return currentVoltage;
    }

    Voltage getVoltageSelect()
    {
      return currentVoltage;
    }

    bool voltagesOk() { return true; }
    bool checkVoltage(Voltage voltage __attribute__((unused))) { return true; }
    bool enableCartridge(bool abortIncorrectVoltage __attribute__((unused))) { return true; }
    bool cartridgeEnabled() { return true; }
    //! @endcond
# endif /* ENABLE_ONBOARD_ATMEGA */

    void disableCartridge()
    {
      //
      // Disable all outputs
      //

      OSCR::ClockGen::stop();

      DDRA = 0;
      PORTA = 0;

      // Don't turn off B

      DDRC = 0;
      PORTC = 0;

      // Don't turn off D

      // VSELECT STAT (6) + UART (0-1)
      DDRE = 0b00000001;
      PORTE = 0b01000000;

      DDRF = 0;
      PORTF = 0;

      // Pins for:
      // - Button w/PU (2)
      // - CIC (1)
      DDRG = 0b00000000;
      PORTG = 0b00000100;

      DDRH = 0;
      PORTH = 0;

      // Pins for:
      //  - Voltage Monitors (4-6)
      //  - CBUS (7)
      DDRJ = 0b10000000;
      PORTJ = 0b00000000;

      DDRK = 0;
      PORTK = 0;

      DDRL = 0;
      PORTL = 0;

# if defined(ENABLE_ONBOARD_ATMEGA)
      // Disable CIC output on G
      DDRG &= ~(1 << 1);
      PORTG &= ~(1 << 1);

      // Disable CBUS
      PORTJ &= ~(1 << 7);
# else
      // Reset CIC
      DDRG |= (1 << 1);
      PORTG |= (1 << 1);
# endif /* ENABLE_ONBOARD_ATMEGA */

# if defined(ENABLE_VSELECT)
      OSCR::Power::setVoltage(OSCR::Voltage::Default);
# endif
    }

# if !defined(ENABLE_VSELECT)
    void changeVoltagePrompt(Voltage newVoltage)
    {
      // Don't prompt if voltage is the same
      if (currentVoltage == newVoltage) return;

      // Probably shouldn't happen...
      if (newVoltage == Voltage::Unknown)
      {
        currentVoltage = Voltage::Unknown;
        return;
      }

      // Prompt user to set voltage manually
      OSCR::UI::printHeader(FS(OSCR::Strings::Headings::ChangeVoltage));
      OSCR::Lang::printSwitchVoltage(newVoltage);
      OSCR::UI::waitButton();

      // Save voltage
      currentVoltage = newVoltage;
    }
# else /* ENABLE_VSELECT */
    void changeVoltagePrompt(Voltage newVoltage __attribute__((unused))) {}
# endif /* !ENABLE_VSELECT */

# if defined(ENABLE_VSELECT)
    bool lockVoltage()
    {
      return voltageLocked;
    }

    void lockVoltage(Voltage newVoltage)
    {
      voltageLocked = true;
      currentVoltage = newVoltage;
      setVoltage(newVoltage);
    }

    void unlockVoltage()
    {
      voltageLocked = false;
      currentVoltage = Voltage::Unknown;
      setVoltage(Voltage::k3V3);
    }

    void voltageLockMenu()
    {
      __FlashStringHelper const * menuOptionsVoltageLock[4] = {};
      uint8_t i, lock3V3, lock5V, unlock, back;

      do
      {
        i = 0;
        lock3V3 = 5;
        lock5V = 5;
        unlock = 5;
        back = 5;

        if (!voltageLocked || currentVoltage != Voltage::k3V3)
        {
          lock3V3 = i;
          menuOptionsVoltageLock[i++] = FS(OSCR::Strings::Power::Voltage3V3);
        }

        if (!voltageLocked || currentVoltage != Voltage::k5V)
        {
          lock5V = i;
          menuOptionsVoltageLock[i++] = FS(OSCR::Strings::Power::Voltage5);
        }

        if (voltageLocked)
        {
          unlock = i;
          menuOptionsVoltageLock[i++] = FS(OSCR::Strings::MenuOptions::Unlock);
        }

        // Back
        back = i;
        menuOptionsVoltageLock[i++] = FS(OSCR::Strings::MenuOptions::Back);

        // Menu
        uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::Headings::OverrideVoltage), menuOptionsVoltageLock, i);

        // Selected lock to 3.3V
        if (menuSelection == lock3V3)
        {
          lockVoltage(Voltage::k3V3);
          continue;
        }

        // Selected lock to 5V
        if (menuSelection == lock5V)
        {
          lockVoltage(Voltage::k5V);
          continue;
        }

        // Selected unlock
        if (menuSelection == unlock)
        {
          unlockVoltage();
          continue;
        }

        // Selected Back
        if (menuSelection == back)
        {
          return;
        }
      }
      while (true);
    }

    ModuleResult setVoltage(Voltage newVoltage)
    {
      switch(newVoltage)
      {
      case Voltage::k5V: // 5V
        if (getVoltageSelect() == Voltage::k5V) return ModuleResult::Success; // Just return if already as requested

        // Ignore voltage change requests when voltage is locked
        if (voltageLocked && (currentVoltage != Voltage::k5V))
        {
          return ModuleResult::Success;
        }

        // Set voltage to 5.0V
        PORTD &= ~(1 << 7); /*[1]*/
        currentVoltage = Voltage::k5V;

#   if defined(ENABLE_ONBOARD_ATMEGA)
        PORTD &= ~((1 << 4) | (1 << 5)); // +Red +Green (Yellow)
#   endif

#   if defined(ENABLE_3V3FIX)
        // Set clock speed
        OSCR::Clock::setClockSpeed(ClockSpeed::k16MHz);
#   endif /* ENABLE_3V3FIX */

        // Wait for voltage to stabilize.
        delay(100);

        // Done
        return checkVoltage(Voltage::k5V) ? ModuleResult::Success : ModuleResult::Error;

      case Voltage::k3V3: // 3V3
        if (getVoltageSelect() == Voltage::k3V3) return ModuleResult::Success; // Just return if already as requested

        // Ignore voltage change requests when voltage is locked
        if (voltageLocked && (currentVoltage != Voltage::k3V3))
        {
          return ModuleResult::Success;
        }

#   if defined(ENABLE_3V3FIX)
        // Set clock speed
        OSCR::Clock::setClockSpeed(ClockSpeed::k8MHz);
#   endif /* ENABLE_3V3FIX */

        // Set voltage to 3.3V
        PORTD |= (1 << 7); /*[1]*/
        currentVoltage = Voltage::k3V3;

#   if defined(ENABLE_ONBOARD_ATMEGA)
        PORTD |=  (1 << 4); // -Red
        PORTD &= ~(1 << 5); // +Green
#   endif

        // Wait for voltage to stabilize.
        delay(200);

        // Done
        return checkVoltage(Voltage::k3V3) ? ModuleResult::Success : ModuleResult::Error;

      default: // ??
        return ModuleResult::Error;
      }
    }
# elif defined(ENABLE_3V3FIX)
    ModuleResult setVoltage(Voltage newVoltage)
    {
      switch(newVoltage)
      {
        /* 5V */
        case Voltage::k5V:
          if (currentClockSpeed == ClockSpeed::k16MHz)
            return ModuleResult::Success; // Just return if already as requested

          changeVoltagePrompt(newVoltage);

          // Set clock speed
          OSCR::Clock::setClockSpeed(ClockSpeed::k16MHz);

          // Done
          return (currentClockSpeed == ClockSpeed::k16MHz) ? ModuleResult::Success : ModuleResult::Error;

        /* 3.3V */
        case Voltage::k3V3:
          if (currentClockSpeed == ClockSpeed::k8MHz)
            return ModuleResult::Success; // Just return if already as requested

          // Set clock speed
          OSCR::Clock::setClockSpeed(ClockSpeed::k8MHz);

          changeVoltagePrompt(newVoltage);

          // Done
          return (currentClockSpeed == ClockSpeed::k8MHz) ? ModuleResult::Success : ModuleResult::Error;

        /* ??? */
        default:
          return ModuleResult::Error;
      }
    }
# else /* !ENABLE_VSELECT && !ENABLE_3V3FIX */
    ModuleResult setVoltage(Voltage newVoltage)
    {
      changeVoltagePrompt(newVoltage);

      return ModuleResult::NotEnabled;
    }
# endif /* ENABLE_VSELECT || ENABLE_3V3FIX */

    bool drowsing()
    {
# if defined(ENABLE_POWERSAVING)
      switch (sleepState)
      {
      case SleepState::Awake:
        return false;

      case SleepState::Drowsy:
      case SleepState::Sleep:
      case SleepState::DeepSleep:
        return true;
      }

      OSCR::Util::unreachable();
# else /* ENABLE_POWERSAVING */
      return false;
# endif /* ENABLE_POWERSAVING */
    }

    bool sleeping()
    {
# if defined(ENABLE_POWERSAVING)
      switch (sleepState)
      {
      case SleepState::Awake:
      case SleepState::Drowsy:
        return false;

      case SleepState::Sleep:
      case SleepState::DeepSleep:
        return true;
      }

      OSCR::Util::unreachable();
# else /* ENABLE_POWERSAVING */
      return false;
# endif /* ENABLE_POWERSAVING */
    }

    void drowse()
    {
# if defined(ENABLE_POWERSAVING)
      if (drowsing()) return;

      sleepState = SleepState::Drowsy;

      //
      // Dim Display
#   if (OPTION_POWERSAVING_METHOD & POWERSAVING_DISPLAY_DIM)
      OSCR::UI::drowse();
#   endif
# endif /* ENABLE_POWERSAVING */
    }

    void sleep()
    {
# if defined(ENABLE_POWERSAVING)
      if (sleeping()) return;

      sleepState = SleepState::Sleep;

      //
      // Sleep Display
#   if (OPTION_POWERSAVING_METHOD & POWERSAVING_DISPLAY_OFF)
      OSCR::UI::sleep();
#   endif

      //
      // Sleep cartridge bus
#   if (OPTION_POWERSAVING_METHOD & POWERSAVING_CARTBUS_OFF)
      wasCartBusOn = cartridgeEnabled();
      disableCartridge();
#   endif

      //
      // Sleep MCU clock/power...
#   if (OPTION_POWERSAVING_METHOD & POWERSAVING_SLOWCLOCK)

#     if !defined(ENABLE_3V3FIX)
      // Set clockspeed low
      OSCR::Clock::setClockSpeed(ClockSpeed::k8MHz);
#     endif /* !ENABLE_3V3FIX */

#     if defined(ENABLE_VSELECT)
      wasVoltage = currentVoltage;

      // Set voltage low (+ set clock speed low if 3V3FIX)
      setVoltage(Voltage::k3V3);
#     endif /* ENABLE_VSELECT */

#   endif /* POWERSAVING_SLOWCLOCK */
# endif /* ENABLE_POWERSAVING */
    }

    void wake()
    {
# if defined(ENABLE_POWERSAVING)
      if (SleepState::Awake == sleepState) return;

      lastInput = millis();

      if (SleepState::Drowsy != sleepState)
      {
        //
        // Wake MCU clock/power...
#   if (OPTION_POWERSAVING_METHOD & POWERSAVING_SLOWCLOCK)

#     if defined(ENABLE_VSELECT)
        // Restore voltage
        setVoltage(wasVoltage);
#     endif /* ENABLE_VSELECT */

#     if !defined(ENABLE_3V3FIX)
        // ...set clockspeed high
        OSCR::Clock::setClockSpeed(ClockSpeed::k16MHz);
#     endif /* !ENABLE_3V3FIX */

#   endif /* POWERSAVING_SLOWCLOCK */

        //
        // Wake cartridge bus
#   if (OPTION_POWERSAVING_METHOD & POWERSAVING_CARTBUS_OFF)
        if (wasCartBusOn) enableCartridge();
#   endif
      }

      //
      // Wake display
#   if (OPTION_POWERSAVING_METHOD & POWERSAVING_DISPLAY_OFF)
      OSCR::UI::wake();
#   endif

      sleepState = SleepState::Awake;
# endif /* ENABLE_POWERSAVING */
    }

    bool wakeEvent()
    {
# if defined(ENABLE_POWERSAVING)
      if (SleepState::Awake == sleepState) return false;

      wake();

      return true;
# else
      return false;
# endif /* ENABLE_POWERSAVING */
    }

    void idle()
    {
# if defined(ENABLE_POWERSAVING)

      if (lastInput > millis()) // Overflow
      {
        lastInput = millis();
        return;
      }

      if (sleeping()) return;

      uint32_t diff = millis() - lastInput;

      if (diff > kIdleSleep)
      {
        sleep();
      }

      if (drowsing()) return;

      if (diff > kIdleDrowsy)
      {
        drowse();
      }

# endif /* ENABLE_POWERSAVING */
    }

    void busy()
    {
# if defined(ENABLE_POWERSAVING)
      lastInput = millis();
# endif /* ENABLE_POWERSAVING */
    }

  } /* namespace Power */
} /* namespace OSCR */

#endif /* OSCR_ARCH_AVR  */
