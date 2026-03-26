#pragma once
#if !defined(OSCR_CORE_SATELLAVIEW_H_)
# define OSCR_CORE_SATELLAVIEW_H_

# include "config.h"

# if HAS_SATELLAVIEW
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for %SNES %Satellaview memory packs
 */
namespace OSCR::Cores::Satellaview
{
  void menu();

  void cartOn();
  void cartOff();

  //void openCRDB();
  //void closeCRDB();

  void printHeader();

  uint8_t readBank(uint8_t myBank, uint16_t myAddress);
  void writeBank(uint8_t myBank, uint16_t myAddress, uint8_t myData);

  void readSRAM();
  void writeSRAM();

  void readROM();
  void writeFlash(void);

  void writeCheck(void);
  void detectCheck(void);
  void eraseAll(void);
} /* namespace OSCR::Cores::Satellaview */

# endif /* HAS_SATELLAVIEW */
#endif /* OSCR_CORE_SATELLAVIEW_H_ */
