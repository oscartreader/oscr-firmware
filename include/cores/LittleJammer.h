#pragma once
#if !defined(OSCR_CORE_LITTLEJAMMER_H_)
# define OSCR_CORE_LITTLEJAMMER_H_

# include "config.h"

# if HAS_LJ
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Little Jammer
 */
namespace OSCR::Cores::LittleJammer
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint32_t addr);
  void readSegment(uint32_t startaddr, uint32_t endaddr);
  void readROM();
  void setROMSize();
  void checkStatus();
  void setCart();
} /* namespace OSCR::Cores::LittleJammer */

# endif /* HAS_LJ */
#endif /* OSCR_CORE_LITTLEJAMMER_H_ */
