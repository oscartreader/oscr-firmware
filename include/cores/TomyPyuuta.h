#pragma once
#if !defined(OSCR_CORE_TOMYPYUUTA_H_)
# define OSCR_CORE_TOMYPYUUTA_H_

# include "config.h"

# if HAS_PYUUTA
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Tomy Pyuuta
 */
namespace OSCR::Cores::TomyPyuuta
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint32_t startaddr, uint32_t endaddr);
  void readROM();
  void setROMSize();
  void setCart();
} /* namespace OSCR::Cores::TomyPyuuta */

# endif /* HAS_PYUUTA */
#endif /* OSCR_CORE_TOMYPYUUTA_H_ */
