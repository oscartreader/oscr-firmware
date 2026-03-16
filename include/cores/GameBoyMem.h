#pragma once
#if !defined(OSCR_CORE_GAMEBOYMEM_H_)
# define OSCR_CORE_GAMEBOYMEM_H_

# include "config.h"

# if HAS_GBX
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for Game Boy Memory Modules
 */
namespace OSCR::Cores::GameBoyMem
{
  void menu();

  uint8_t readByte(uint16_t myAddress);
  void writeByte(uint16_t myAddress, uint8_t myData);

  void printSdBuffer(uint16_t startByte, uint16_t numBytes);
  void readROM();

  void send(uint8_t myCommand);

# if HAS_FLASH
  void send(uint8_t myCommand, uint16_t myAddress, uint8_t myData);
  void sendSequence(uint8_t myData);
  void switchGame(uint8_t myData);
  void resetFlash();
  bool readFlashID();
  void eraseFlash();
  bool blankcheckFlash();
  void writeFlash();
  void readMapping();
  bool eraseMapping();
  void writeMapping();
# endif /* HAS_FLASH */
} /* namespace OSCR::Cores::GameBoyMem */

# endif /* HAS_GBX */
#endif /* OSCR_CORE_GAMEBOYMEM_H_ */
