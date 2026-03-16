/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#ifndef CLOCKEDSERIAL_H_
#define CLOCKEDSERIAL_H_
#pragma once

# include "arch/avr/syslibinc.h"

# if defined(OSCR_ARCH_AVR)

#   include <HardwareSerial.h>
#   include <HardwareSerial_private.h>

/**
 * @class DynamicClockSerial
 *
 * @brief Serial interface that supports a dynamic clock speed
 *
 * This function is unchanged, including comments, from HardwareSerial. Comments not from
 * the original function are denoted with a prefix of "(ClockedSerial)".
 *
 * The parameter `sclock` is used to let it know the clockspeed. It replaces the usage of
 * the F_CPU preprocessor variable. Unlike `clock_prescale_set` this parameter is the
 * speed in MHz, i.e. 16000000 (16MHz).
 *
 * @note
 * If this class isn't used by anything then the compiler will optimize it out. So, there
 * is no harm in leaving it.
 *
 * @sa https://docs.arduino.cc/language-reference/en/functions/communication/serial/
 */
class DynamicClockSerial : public HardwareSerial
{
  using HardwareSerial::HardwareSerial;

public:
  // Various functions to allow parameter omission and automatic handling.
  void begin(uint32_t baud);
  void begin(uint32_t baud, uint8_t config);
  void begin(uint32_t baud, uint32_t sclock);
  void begin(uint32_t baud, uint8_t config, uint32_t sclock);

  void clockSkew(uint32_t sclock);
  void clockSkewAtomic(uint32_t sclock);

protected:
  uint32_t clockSpeed = 0;
  uint32_t baudRate;
};

extern DynamicClockSerial ClockedSerial;

# endif /* OSCR_ARCH_AVR  */

#endif /* CLOCKEDSERIAL_H_ */
