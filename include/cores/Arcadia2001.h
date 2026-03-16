#pragma once
#if !defined(OSCR_CORE_ARC_H_)
# define OSCR_CORE_ARC_H_

# include "config.h"

# if HAS_ARC

#include "syslibinc.h"
#include "common/Types.h"

/**
 * @brief System core for the Arcadia 2001
 */
namespace OSCR::Cores::Arcadia2001
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint16_t startaddr, uint16_t endaddr);
  void readROM();
  void setROMSize();
  void checkStatus();
  void setCart();
} /* namespace OSCR::Cores::Arcadia2001 */

# endif /* HAS_ARC */
#endif /* OSCR_CORE_ARC_H_ */
