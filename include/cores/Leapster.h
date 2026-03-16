#pragma once
#if !defined(OSCR_CORE_LEAPSTER_H_)
# define OSCR_CORE_LEAPSTER_H_

# include "config.h"

# if HAS_LEAP
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the %Leapster
 */
namespace OSCR::Cores::Leapster
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint16_t read_rom_word(uint32_t address);
  uint8_t read_flash_byte(uint32_t address);
  void write_flash_byte(uint32_t address, uint8_t data);
  void checkStart();
  void findTable(uint32_t startAddr, uint32_t endAddr);
  void readTable(uint32_t startAddr, uint32_t endAddr);
  void readROM();
  void setROMSize();
  void dataOut();
  void dataIn();
  void idFLASH();
  void resetFLASH();
  void eraseFLASH();
  void programFLASH();
  void statusFLASH();
  void readFLASH();
  void writeFLASH();
  void eepromStart();
  void eepromSet0();
  void eepromSet1();
  void eepromDevice();
  void eepromSetDeviceAddress(uint32_t addrhi);
  void eepromStatus();
  void eepromReadMode();
  void eepromWriteMode();
  void eepromReadData();
  void eepromWriteData(uint8_t data);
  void eepromStop();
  void eepromSetAddress(uint16_t address);
  void readEepromByte(uint16_t address);
  void writeEepromByte(uint16_t address);
  void readEEP();
  void writeEEP();
  void setCart();
} /* namespace OSCR::Cores::Leapster */

# endif /* HAS_LEAP */
#endif /* OSCR_CORE_LEAPSTER_H_ */
