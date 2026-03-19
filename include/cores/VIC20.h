#pragma once
#if !defined(OSCR_CORE_VIC20_H_)
# define OSCR_CORE_VIC20_H_

# include "config.h"

# if HAS_VIC20
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Commodore VIC-20
 */
namespace OSCR::Cores::VIC20
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readROM();
  void setROMSize();
  void checkStatus();
  void setROMMap();
  void setCart();
} /* namespace OSCR::Cores::VIC20 */

# endif /* HAS_VIC20 */
#endif /* OSCR_CORE_VIC20_H_ */
