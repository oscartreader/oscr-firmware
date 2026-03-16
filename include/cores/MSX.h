#pragma once
#if !defined(OSCR_CORE_MSX_H_)
# define OSCR_CORE_MSX_H_

# include "config.h"

# if HAS_MSX
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the %MSX
 */
namespace OSCR::Cores::MSX
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint32_t startaddr, uint32_t endaddr);
  void writeData(uint16_t addr, uint8_t data);
  void setCS();
  void checkCS();
  void enableCS();
  void disableCS();
  void readROM();
  bool testRAM(uint8_t enable1, uint8_t enable2);
  bool checkRAM();
  void readRAM();
  void writeRAM();
  void setMapper();
  void setCart();
} /* namespace OSCR::Cores::MSX */

# endif /* HAS_MSX */
#endif /* OSCR_CORE_MSX_H_ */
