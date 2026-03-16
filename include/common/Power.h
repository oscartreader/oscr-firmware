/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_VOLTAGE_H_
#define OSCR_VOLTAGE_H_

#include "config.h"
#include "common/Types.h"

namespace OSCR
{
  /**
   * @brief Power/voltage methods.
   */
  namespace Power
  {
    /**
     * Adjust the voltage and/or clock prescaler, if enabled.
     *
     * @param newVoltage  The voltage to set.
     *
     * **Without VSELECT**
     * Without VSELECT, this will prompt the user to set the voltage
     * switch to the specified voltage. If the 3V3 fix is enabled,
     * the prompt happens before the clock change when switching to
     * 5V, or after the clock change when switching to 3.3V.
     *
     * **With VSELECT**
     * When changing to 5V the voltage is set first so that the MPU
     * will be stable at 16MHz. When going down to 3.3V the clock
     * is changed to 8MHz first so that the MPU will be stable when
     * the voltage is changed to 3.3V.
     *
     * @note
     * 3V3FIX works best with VSELECT as the firmware controls the
     * timing of all of this. If you are doing this manually, then
     * you'll need to start the %OSCR with 5V set and only switch to
     * 3.3V once prompted.
     */
    extern ModuleResult setVoltage(Voltage volts);

#if defined(ENABLE_VSELECT)
    extern bool lockVoltage();
    extern void lockVoltage(Voltage newVoltage);
    extern void unlockVoltage();
    extern void voltageLockMenu();
#endif

    /**
     * Get the current voltage reading.
     */
    extern Voltage getVoltage();

    /**
     * Get the current voltage output from VSELECT (TPS2113).
     */
    extern Voltage getVoltageSelect();

    /**
     * Get the current voltage using the voltage monitors.
     */
    extern bool checkVoltage(Voltage voltage);

    /**
     * Check if the voltages of the power rails are valid.
     */
    extern bool voltagesOk();

    /**
     * Enable the cartridge power rail.
     */
    extern bool enableCartridge(bool abortIncorrectVoltage = true);

    /**
     * Disable the cartridge power rail.
     */
    extern void disableCartridge();

    /**
     * Check if the cartridge power rail is enabled.
     */
    extern bool cartridgeEnabled();

    /**
     * Check if we are sleeping.
     */
    extern bool sleeping();

    /**
     * Enter sleep state.
     */
    extern void sleep();

    /**
     * Wake from sleep state.
     */
    extern void wake();

    /**
     * If sleeping, wake and return `true`, else `false`.
     */
    extern bool wakeEvent();

    /**
     * Reset the idle timer.
     */
    extern void busy();

    /**
     * Check the idle timer and enter sleep if idle for long enough.
     */
    extern void idle();
  }

  /**
   * @brief MCU clock speed methods
   */
  namespace Clock
  {
    /**
     * Set the clock speed of the ATmega2560.
     *
     * @param newClock  The ClockSpeed to change to.
     * @returns ModuleResult
     */
    extern ModuleResult setClockSpeed(ClockSpeed newClock);

     /**
     * Get the current clock speed.
     */
    extern ClockSpeed getClockSpeed();

    /**
     * Get the current clock speed as an integer.
     */
    extern uint32_t getClock();
  }
}

#endif /* OSCR_VOLTAGE_H_ */
