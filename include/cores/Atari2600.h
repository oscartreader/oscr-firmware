#pragma once
#if !defined(OSCR_CORE_ATARI2600_H_)
# define OSCR_CORE_ATARI2600_H_

# include "config.h"

# if HAS_2600
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Atari 2600
 */
namespace OSCR::Cores::Atari2600
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint16_t startaddr, uint16_t endaddr);
  void readDataArray(uint16_t addr, uint16_t size);
  void readSegmentF8(uint16_t startaddr, uint16_t endaddr, uint16_t bankaddr);
  void readSegmentE7(uint8_t start, uint8_t end);
  void readSegmentFx(bool hasRAM, uint16_t size);
  void readSegmentTigervision(uint8_t banks);
  void outputFF(uint16_t size);
  void writeData(uint16_t addr, uint8_t data);
  void writeData3F(uint16_t addr, uint8_t data);
  bool checkE7(uint16_t bank);
  void readROM();
  void checkStatus();
  void setMapperMenu();
  void setCart();
} /* namespace OSCR::Cores::Atari2600 */

# endif /* HAS_2600 */
#endif /* OSCR_CORE_ATARI2600_H_ */
