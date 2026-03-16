#pragma once
#if !defined(OSCR_CORE_VECTREX_H_)
# define OSCR_CORE_VECTREX_H_

# include "config.h"

# if HAS_VECTREX
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the %Vectrex
 */
namespace OSCR::Cores::Vectrex
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint16_t startaddr, uint16_t endaddr);

  void readROM();
  void setROMSize();
  void setCart();
} /* namespace OSCR::Cores::Vectrex */

# endif /* HAS_VECTREX */
#endif /* OSCR_CORE_VECTREX_H_ */
