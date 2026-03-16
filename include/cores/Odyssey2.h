#pragma once
#if !defined(OSCR_CORE_ODYSSEY2_H_)
# define OSCR_CORE_ODYSSEY2_H_

# include "config.h"

# if HAS_ODY2
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Magnavox Odyssey 2
 */
namespace OSCR::Cores::Odyssey2
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void bankSwitch(uint16_t addr, uint8_t data);
  void readROM();
  void setROMSize();
  void setCart();
} /* namespace OSCR::Cores::Odyssey2 */

# endif /* HAS_ODY2 */
#endif /* OSCR_CORE_ODYSSEY2_H_ */
