#pragma once
#if !defined(OSCR_CORE_TRS80_H_)
# define OSCR_CORE_TRS80_H_

# include "config.h"

# if HAS_TRS80
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the TRS-80
 */
namespace OSCR::Cores::TRS80
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint32_t startaddr, uint32_t endaddr);
  void bankSwitch(uint16_t addr, uint8_t data);
  void readROM();
  void setROMSize();
  void setCart();
} /* namespace OSCR::Cores::TRS80 */

# endif /* HAS_TRS80 */
#endif /* OSCR_CORE_TRS80_H_ */
