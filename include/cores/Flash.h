#pragma once
#if !defined(OSCR_CORE_FLASH_H_)
# define OSCR_CORE_FLASH_H_

# include "config.h"

# include "syslibinc.h"
# include "common/Types.h"

/**
 * @brief System core for flash chips
 */
namespace OSCR::Cores::Flash
{
# if HAS_FLASH
  extern char const * const menuOptionsFlash[];
  extern uint8_t const kMenuOptionFlashMax;
# endif /* HAS_FLASH */

  extern uint16_t flashid;
  extern uint32_t flashSize;
  extern uint32_t blank;
  extern uint8_t mapping;

# if HAS_FLASH
  extern uint32_t flashBanks;
  extern bool flashX16Mode;
  extern bool flashSwitchLastBits;
  extern uint8_t flashromType;
  extern uint8_t secondID;
  extern uint32_t time;
  extern uint32_t sectorSize;
  extern uint16_t bufferSize;
  extern bool byteCtrl;

  void menu();
  void menuFlash8();
  void menuFlash16();
  void menuEPROM();

  void printHeader();

  void setupCFI();
  void printFlashSize(int index);
  void printFlashType(int index);
  uint8_t selectFlashtype(bool option);
  bool getFlashDetail();
  void id8();
  void id16();
  void setupVoltage();
  void setup8();
  void setup16();
  void setup_Eprom();
  void dataIn();
  void dataOut();
  void writeByte(uint32_t myAddress, uint8_t myData);
  uint8_t readByte(uint32_t myAddress);
  void writeWord(uint32_t myAddress, uint16_t myData);
  uint16_t readWord(uint32_t myAddress);
  void writeByteCommand(uint8_t command);
  void writeByteCommandShift(uint8_t command);
  void writeWordCommand(uint8_t command);
  void writeByteCommand(uint8_t command);
  void busyWait(uint32_t address, uint8_t data);
  void busyWait(uint32_t address);
  void busyWait();
  void resetFlash29F032();
  void idFlash39SF040();
  void idFlash29F032();
  void eraseFlash29F032();
  void writeFlash29F032();
  bool busyCheck29F032(uint32_t addr, uint8_t c);
  void resetFlash29F1610();
  void writeFlash29F1610();
  void writeFlash29F1601();
  void idFlash29F1610();
  void eraseFlash29F1610();
  void writeFlash29LV640();
  void writeFlash29GL(uint32_t sectorSize, uint8_t bufferSize);
  void writeFlash29F800();
  void idFlash28FXXX();
  void resetFlash28FXXX();
  uint8_t statusFlash28FXXX();
  void eraseFlash28FXXX();
  void writeFlash28FXXX();
  void writeFlashE28FXXXJ3A();
  void writeFlashLH28F0XX();
  void blankcheck();
  void verifyFlash(bool closeFile = true);
  void verifyFlash(uint8_t currChip, uint8_t totalChips, bool reversed, bool closeFile = true);
  void readFlash();
  void printFlash(int numBytes);
  void resetFlash8();
  void resetFlash16();
  void writeFlash16();
  void writeFlash16_29F1601();
  void idFlash16();
  uint8_t readStatusReg16();
  void eraseFlash16();
  void blankcheck16();
  void verifyFlash16(bool closeFile = true);
  void readFlash16();
  void printFlash16(int numBytes);
  void busyCheck16();
  void busyCheck16_29LV640(uint32_t myAddress, uint16_t myData);
  void writeFlash16_29LV640();
  uint16_t writeWord_Eprom(uint32_t myAddress, uint16_t myData);
  uint16_t readWord_Eprom(uint32_t myAddress);
  void blankcheck_Eprom();
  void read_Eprom();
  void write_Eprom();
  void verify_Eprom();
  void print_Eprom(int numBytes);
  void sendCFICommand(uint8_t cmd);
  uint8_t readByteCompensated(int address);
  void writeByteCompensated(int address, uint8_t data);
  void startCFIMode(bool x16Mode);
  bool identifyCFIbyIds(bool x16Mode = false);
  void identifyCFI();
  void adjustFileSizeOffset(uint8_t currChip, uint8_t totalChips, bool reversed);
  void writeCFI(uint8_t currChip, uint8_t totalChips, bool reversed);
# endif /* HAS_FLASH */

} /* namespace OSCR::Cores::Flash */

#endif /* OSCR_CORE_FLASH_H_ */
