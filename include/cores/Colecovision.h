#pragma once
#if !defined(OSCR_CORE_COLECOVISION_H_)
# define OSCR_CORE_COLECOVISION_H_

# include "config.h"

# if HAS_COLV
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for %Colecovision
 */
namespace OSCR::Cores::Colecovision
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint32_t addr);
  void readSegment(uint16_t startaddr, uint32_t endaddr);
  void readROM();
  void setROMSize();
  void checkStatus();
  void setCart();
} /* namespace OSCR::Cores::Colecovision */

# endif /* HAS_COLV */
#endif /* OSCR_CORE_COLECOVISION_H_ */
