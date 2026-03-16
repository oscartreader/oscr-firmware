#pragma once
#if !defined(OSCR_CORE_POKEMONMINI_H_)
# define OSCR_CORE_POKEMONMINI_H_

# include "config.h"

# if HAS_POKE
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Pokemon Mini
 */
namespace OSCR::Cores::PokemonMini
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readData(uint32_t addr);
  void writeData(uint32_t addr, uint8_t data);
  void readROM();
} /* namespace OSCR::Cores::PokemonMini */

# endif /* HAS_POKE */
#endif /* OSCR_CORE_POKEMONMINI_H_ */
