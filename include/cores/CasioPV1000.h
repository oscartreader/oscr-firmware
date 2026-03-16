#pragma once
#if !defined(OSCR_CORE_CASIOPV1000_H_)
# define OSCR_CORE_CASIOPV1000_H_

# include "config.h"

# if HAS_PV1000
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Casio PV-1000/PV-2000
 */
namespace OSCR::Cores::CasioPV1000
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint16_t addr);
  void readSegment(uint32_t startaddr, uint32_t endaddr);
  void readROM();
  void setROMSize();
  void checkStatus();
  void setCart();
} /* namespace OSCR::Cores::CasioPV1000 */

# endif /* HAS_PV1000 */
#endif /* OSCR_CORE_CASIOPV1000_H_ */
