#pragma once
#if !defined(OSCR_CORE_BALLYASTROCADE_H_)
# define OSCR_CORE_BALLYASTROCADE_H_

# include "config.h"

# if HAS_BALLY
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Bally Astrocade
 */
namespace OSCR::Cores::BallyAstrocade
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  uint8_t readData(uint16_t addr);
  void readSegment(uint16_t startaddr, uint16_t endaddr);
  void readROM();
  void setROMSize();
  void checkStatus();
  void setCart();
} /* namespace OSCR::Cores::BallyAstrocade */

# endif /* HAS_BALLY */
#endif /* OSCR_CORE_BALLYASTROCADE_H_ */
