#pragma once
#if !defined(OSCR_CORE_ATARI5200_H_)
# define OSCR_CORE_ATARI5200_H_

# include "config.h"

# if HAS_5200
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Atari 5200
 */
namespace OSCR::Cores::Atari5200
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint16_t startaddr, uint16_t endaddr);
  void readBankBountyBob(uint16_t startaddr);
  void readROM();
  void setROMSize();
  void checkStatus();
  void setMapperMenu();
  void setCart();
  void cycleDelay(uint8_t cycleCount);
} /* namespace OSCR::Cores::Atari5200 */

# endif /* HAS_5200 */
#endif /* OSCR_CORE_ATARI5200_H_ */
