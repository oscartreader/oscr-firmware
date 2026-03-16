#pragma once
#if !defined(OSCR_CORE_COMMODORE64_H_)
# define OSCR_CORE_COMMODORE64_H_

# include "config.h"

# if HAS_C64
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Commodore 64
 */
namespace OSCR::Cores::Commodore64
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint16_t startaddr, uint32_t endaddr, uint16_t size = 512);
  void readSegmentEnableDisable(uint16_t startaddr, uint32_t endaddr, uint8_t romLow, uint16_t size = 512);
  void readSegment16k();
  void readSegmentBankD0D5(uint16_t banks, uint16_t address, uint8_t romLow);
  void readSegmentBankA0A4(uint16_t banks);
  void writeData(uint16_t addr, uint8_t data);
  void bankSwitch(uint16_t addr, uint8_t data);
  uint8_t readPorts();
  void readROM();
  void setMapper();
  void setROMSize();
  void checkStatus();
  void setCart();
} /* namespace OSCR::Cores::Commodore64 */

# endif /* HAS_C64 */
#endif /* OSCR_CORE_COMMODORE64_H_ */
