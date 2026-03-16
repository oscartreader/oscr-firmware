#pragma once
#if !defined(OSCR_CORE_CHANNELF_H_)
# define OSCR_CORE_CHANNELF_H_

# include "config.h"

# if HAS_FAIRCHILD
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Fairchild Channel F
 */
namespace OSCR::Cores::ChannelF
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void clearRegister();
  void setROMC(uint8_t command);
  void setREAD();
  uint8_t readData();
  void readROM();
  void read16K();
  void setROMSize();
  void setCart();
} /* namespace OSCR::Cores::ChannelF */

# endif /* HAS_FAIRCHILD */
#endif /* OSCR_CORE_CHANNELF_H_ */
