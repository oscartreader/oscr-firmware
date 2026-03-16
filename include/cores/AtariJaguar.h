#pragma once
#if !defined(OSCR_CORE_ATARIJAGUAR_H_)
# define OSCR_CORE_ATARIJAGUAR_H_

# include "config.h"

# if HAS_JAGUAR
#   include "syslibinc.h"
#   include "common/Types.h"
/**
 * @brief System core for the Atari Jaguar
 */
namespace OSCR::Cores::AtariJaguar
{
  void menu();

  void cartOn();
  void cartOff();

  void printHeader();

  void setCart();
  void sizeMenu();
  void eepMenu();
  void getCartInfo();
  void shiftOutFAST(uint8_t addr);
  void readData(uint32_t myAddress);
  void dataOut();
  void dataIn();
  void readROM();
  void readEEP();
  void writeEEP();
  void eepromDisplayClear();
  void eepromReset();
  void eeprom0();
  void eeprom1();
  void eepromRead(uint16_t addr);
  void eepromWrite(uint16_t addr);
  void eepromreadData();
  void eepromWriteData();
  void eepromSetAddress(uint16_t addr);
  void eepromStatus();
  void eepromEWEN();
  void eepromERAL();
  void eepromEWDS();
  void readID();
  void eraseFLASH();
  void resetFLASH();
  void busyCheck();
  uint8_t readBYTE_FLASH(uint32_t myAddress);
  uint8_t readBYTE_MEMROM(uint32_t myAddress);
  void writeBYTE_FLASH(uint32_t myAddress, uint8_t myData);
  void writeSECTOR_FLASH(uint32_t myAddress);
  void readMEMORY();
  void readFLASH();
  void writeFLASH();
  uint32_t verifyFLASH();
} /* namespace OSCR::Cores::AtariJaguar */

# endif /* HAS_JAGUAR */
#endif /* OSCR_CORE_ATARIJAGUAR_H_ */
