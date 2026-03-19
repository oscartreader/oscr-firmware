#pragma once
#if !defined(OSCR_CORE_CPS3_H_)
# define OSCR_CORE_CPS3_H_

# include "config.h"

# if HAS_CPS3
#   include "syslibinc.h"
#   include "common/Types.h"
#   include "common/crc32.h"

/**
 * @brief System core for the CP System III
 */
namespace OSCR::Cores::CPS3
{
  struct CartCD
  {
    uint32_t disc = 0;
    uint32_t patch = 0;
    uint32_t region = 0;
    uint32_t regionOffset = 0;
    uint32_t multiCart = 0;
    uint32_t multiCartPatch1 = 0;
    uint32_t multiCartPatch2 = 0;
    uint32_t offsetNoCD = 0;
  };

  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void flashromCPS_Cartridge();
  void flashromCPS_SIMM2x8();
  void flashromCPS_SIMM4x8();
  void enable64MSB();
  void enable64LSB();

  void writeByteCommand_Flash2x8(uint32_t bank, uint8_t command);
  void readCartridge();
  void writeCartridge(CartCD cartCD);
  void verifyCartridge(CartCD cartCD);
  CartCD setCartridgePatchData(crc32_t & crc32);
  void id_SIMM2x8();
  void resetSIMM2x8();
  void blankcheckSIMM2x8();
  void readSIMM2x8();
  void readSIMM2x8B();
  void eraseSIMM2x8();
  void writeSIMM2x8();
  void id_SIMM4x8();
  void resetSIMM4x8();
  void blankcheckSIMM4x8();
  void eraseSIMM4x8();
  void readSIMM4x8();
  void writeSIMM4x8();
  void printSIMM4x8(int numBytes);
  void resetFlash2x8(uint32_t bank);
  void idFlash2x8(uint32_t bank);
  void eraseFlash2x8(uint32_t bank);
  int busyCheck2x8(uint32_t addr, uint16_t c);
} /* namespace OSCR::Cores::CPS3 */

# endif /* HAS_CPS3 */
#endif /* OSCR_CORE_CPS3_H_ */
