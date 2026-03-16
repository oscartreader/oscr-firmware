#pragma once
#if !defined(OSCR_CORE_SUPERVISION_H_)
# define OSCR_CORE_SUPERVISION_H_

# include "config.h"

# if HAS_WSV
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Watara %Supervision
 */
namespace OSCR::Cores::Supervision
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void controlOut();
  void controlIn();
  void dataIn();
  void dataOut();
  uint8_t readByte(uint32_t addr);

  void readROM();
  void setROMSize();
  void setCart();
} /* namespace OSCR::Cores::Supervision */

# endif /* HAS_WSV */
#endif /* OSCR_CORE_SUPERVISION_H_ */
