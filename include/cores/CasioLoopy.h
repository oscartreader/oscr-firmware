#pragma once
#if !defined(OSCR_CORE_CASIOLOOPY_H_)
# define OSCR_CORE_CASIOLOOPY_H_

# include "config.h"

# if HAS_LOOPY
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Casio Loopy
 */
namespace OSCR::Cores::CasioLoopy
{
  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void setAddress(uint32_t A);
  uint16_t getWord();
  uint8_t getByte();
  void setByte(uint8_t D);
  uint8_t readByte(uint32_t myAddress);
  void writeByte(uint32_t myAddress, uint8_t myData);
  uint16_t readWord(uint32_t myAddress);
  void dataOut();
  void dataIn();
  void readROM();
  void writeSRAM();
  void formatSRAM();
  void readSRAM();
} /* namespace OSCR::Cores::CasioLoopy */

# endif /* HAS_LOOPY */
#endif /* OSCR_CORE_CASIOLOOPY_H_ */
