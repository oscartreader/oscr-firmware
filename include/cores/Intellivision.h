#pragma once
#if !defined(OSCR_CORE_INTELLIVISION_H_)
# define OSCR_CORE_INTELLIVISION_H_

# include "config.h"

# if HAS_INTV
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Mattel %Intellivision
 */
namespace OSCR::Cores::Intellivision
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint16_t readData(uint32_t addr);
  void readSegment(uint32_t startaddr, uint32_t endaddr);
  void readROM();
  void ecsBank(uint32_t addr, uint8_t bank);
  void setMapper();
  void setCart();
} /* namespace OSCR::Cores::Intellivision */

# endif /* HAS_INTV */
#endif /* OSCR_CORE_INTELLIVISION_H_ */
