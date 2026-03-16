#pragma once
#if !defined(OSCR_CORE_VSMILE_H_)
# define OSCR_CORE_VSMILE_H_

# include "config.h"

# if HAS_VSMILE
#   include "syslibinc.h"
#   include "common/Types.h"

#   define VSMILE_OE_HIGH   PORTH |= (1<<3)   // SNES /CART
#   define VSMILE_OE_LOW    PORTH &= ~(1<<3)
#   define VSMILE_CSB1_HIGH PORTH |= (1<<6)   // SNES /RD
#   define VSMILE_CSB1_LOW  PORTH &= ~(1<<6)
#   define VSMILE_CSB2_HIGH PORTH |= (1<<4)   // SNES /IRQ
#   define VSMILE_CSB2_LOW  PORTH &= ~(1<<4)

/**
 * @brief System core for the V.Smile
 */
namespace OSCR::Cores::VSmile
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint16_t read_rom_word(uint32_t address);
  uint16_t read_rom2_word(uint32_t address);
  uint16_t read_rom3_word(uint32_t address);

  void readROM();
  void setROMSize();

  void setCart();
} /* namespace OSCR::Cores::VSmile */

# endif /* HAS_VSMILE */
#endif /* OSCR_CORE_VSMILE_H_ */
