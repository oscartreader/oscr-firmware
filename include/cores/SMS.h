#pragma once
#if !defined(OSCR_CORE_SMS_H_)
# define OSCR_CORE_SMS_H_

# include "config.h"

# if HAS_SMS
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the %SMS
 */
namespace OSCR::Cores::SMS
{
  void menu();
  void operationsMenu();
  void cartOn();
  void cartOff();
  void writeByte(uint16_t myAddress, uint8_t myData);
  uint8_t readByte(uint16_t myAddress);
  uint8_t readNibble(uint8_t data, uint8_t number);
  void getCartInfo();
  void manual_selectRomSize();
  void readROM();
  void readSRAM();
  void writeSRAM();
} /* namespace OSCR::Cores::SMS */

# endif /* HAS_SMS */
#endif /* OSCR_CORE_SMS_H_ */
