#pragma once
#if !defined(OSCR_CORE_RCASTUDIO2_H_)
# define OSCR_CORE_RCASTUDIO2_H_

# include "config.h"

# if HAS_RCA
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the RCA Studio II
 */
namespace OSCR::Cores::RCAStudio2
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
  void setCart();
} /* namespace OSCR::Cores::RCAStudio2 */

# endif /* HAS_RCA */
#endif /* OSCR_CORE_RCASTUDIO2_H_ */
