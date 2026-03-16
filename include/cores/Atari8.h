#pragma once
#if !defined(OSCR_CORE_ATARI8_H_)
# define OSCR_CORE_ATARI8_H_

# include "config.h"

# if HAS_ATARI8
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for 8-bit Atari
 */
namespace OSCR::Cores::Atari8
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint16_t startaddr, uint16_t endaddr);
  void bankSwitch(uint8_t bank);
  void readBountyBobBank(uint16_t startaddr);
  void readROM();
  void setROMSize();
  void checkStatus();
  void setCart();
} /* namespace OSCR::Cores::Atari8 */

# endif /* HAS_ATARI8 */
#endif /* OSCR_CORE_ATARI8_H_ */
