#pragma once
#if !defined(OSCR_CORE_GPC_H_)
# define OSCR_CORE_GPC_H_

# include "config.h"

# if HAS_GPC
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for %SNES Game Processor RAM Cassettes
 */
namespace OSCR::Cores::GPC
{
  void menu();

  void cartOn();
  void cartOff();

  void printHeader();

  void writeBank(uint8_t myBank, uint16_t myAddress, uint8_t myData);
  uint8_t readBank(uint8_t myBank, uint16_t myAddress);

  void readRAM();
  bool writeRAM();
} /* namespace OSCR::Cores::GPC */

# endif /* HAS_GPC */
#endif /* OSCR_CORE_GPC_H_ */
