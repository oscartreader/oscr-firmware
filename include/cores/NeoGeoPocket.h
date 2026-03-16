#pragma once
#if !defined(OSCR_CORE_NEOGEOPOCKET_H_)
# define OSCR_CORE_NEOGEOPOCKET_H_

# include "config.h"

# if HAS_NGP
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Neo Geo Pocket
 */
namespace OSCR::Cores::NeoGeoPocket
{
  void menu();

  void cartOn();
  void cartOff();

  void printHeader();

  void dataIn();
  void dataOut();

  bool getCartInfo();
  void printCartInfo();
  void readROM();
  void changeSize();
  void writeByte(uint32_t addr, uint8_t data);
  uint8_t readByte(uint32_t addr);
} /* namespace OSCR::Cores::NeoGeoPocket */

# endif /* HAS_NGP */
#endif /* OSCR_CORE_NEOGEOPOCKET_H_ */
