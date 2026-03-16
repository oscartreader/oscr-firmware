#pragma once
#if !defined(OSCR_CORE_PCENGINE_H_)
# define OSCR_CORE_PCENGINE_H_

# include "config.h"

# if HAS_PCE
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for PC Engine & TurboGrafx
 */
namespace OSCR::Cores::PCEngine
{
  void menu();
  void hucardMenu();
  void turbochipMenu();

#if HAS_FLASH
  void flashMenu();
#endif

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void pin_read_write(void);
  void reset_cart();
  uint8_t read_byte(uint32_t address);
  void data_output();
  void data_input();
  void write_byte(uint32_t address, uint8_t data);
  void detect_rom_size();
  void read_bank_ROM(uint32_t address_start, uint32_t address_end);
  void read_bank_RAM(uint32_t address_start, int block_index);
  void unlock_tennokoe_bank_RAM();
  void lock_tennokoe_bank_RAM();
  void read_tennokoe_bank(int bank_index);
  void write_tennokoe_bank(int bank_index);
  void readROM();
  void flash_mode();
  void flash_wait_status(uint8_t expected);
  void flash();
  void set_cs_rd_low();
} /* namespace OSCR::Cores::PCEngine */

# endif /* HAS_PCE */
#endif /* OSCR_CORE_PCENGINE_H_ */
