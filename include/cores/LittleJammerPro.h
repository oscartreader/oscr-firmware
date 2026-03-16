#pragma once
#if !defined(OSCR_CORE_LITTLEJAMMERPRO_H_)
# define OSCR_CORE_LITTLEJAMMERPRO_H_

# include "config.h"

# if HAS_LJPRO
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Little Jammer Pro
 */
namespace OSCR::Cores::LittleJammerPro
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void pulseClock(uint16_t times);
  void readROM();
} /* namespace OSCR::Cores::LittleJammerPro */

# endif /* HAS_LJPRO */
#endif /* OSCR_CORE_LITTLEJAMMERPRO_H_ */
