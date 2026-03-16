#pragma once
#if !defined(OSCR_CORE_POCKETCHALLENGEW_H_)
# define OSCR_CORE_POCKETCHALLENGEW_H_

# include "config.h"

# if HAS_PCW
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Benesse Pocket Challenge W
 */
namespace OSCR::Cores::PocketChallengeW
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void read_setup();
  uint8_t read_ram_byte_1A(uint32_t address);
  uint8_t read_ram_byte_1B(uint32_t address);
  void write_ram_byte_1A(uint32_t address, uint8_t data);
  void write_ram_byte_1B(uint32_t address, uint8_t data);
  uint32_t detect_rom_size();
  void readSingleROM();
  void check_multi();
  void write_bank_byte(uint8_t data);
  void readMultiROM();
  void switchBank(int bank);
  void readSRAM();
  void writeSRAM();
} /* namespace OSCR::Cores::PocketChallengeW */

# endif /* HAS_PCW */
#endif /* OSCR_CORE_POCKETCHALLENGEW_H_ */
