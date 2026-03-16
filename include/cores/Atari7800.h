#pragma once
#if !defined(OSCR_CORE_ATARI7800_H_)
# define OSCR_CORE_ATARI7800_H_

# include "config.h"

# if HAS_7800
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Atari 7800
 */
namespace OSCR::Cores::Atari7800
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint16_t startaddr, uint32_t endaddr);
  void readSegmentBank(uint8_t startbank, uint8_t endbank);
  void readStandard();
  void readSupergame();
  void writeData(uint16_t addr, uint8_t data);
  void bankSwitch(uint16_t addr);
  void readROM();
  void setHalt(uint8_t on);
  void setROMSize();
  void checkStatus();
  void setMapperMenu();
  void setCart();
} /* namespace OSCR::Cores::Atari7800 */

# endif /* HAS_7800 */
#endif /* OSCR_CORE_ATARI7800_H_ */
