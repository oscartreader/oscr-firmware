#pragma once
#if !defined(OSCR_CORE_ST_H_)
# define OSCR_CORE_ST_H_

# include "config.h"

# if HAS_ST
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the SuFami Turbo
 */
namespace OSCR::Cores::ST
{
  void menu();

  void cartOn();
  void cartOff();

  void openCRDB();
  void closeCRDB();

  void printHeader();

  bool getHeader(uint16_t bank);
  bool checkAdapter();
  void readSlot(bool cartSlot);
  void readRom(uint16_t bankStart, uint16_t bankEnd);
} /* namespace OSCR::Cores::ST */

# endif /* HAS_ST */
#endif /* OSCR_CORE_ST_H_ */
