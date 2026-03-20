#pragma once
#if !defined(OSCR_CORE_GAMEBOYADVANCE_H_)
# define OSCR_CORE_GAMEBOYADVANCE_H_

# include "config.h"

# if HAS_GBX
#   include "syslibinc.h"
#   include "common/Types.h"
#   include "common/crc32.h"

#   define gbaCRDB ((OSCR::Databases::GBA *)cartCRDB)

/**
 * @brief System core for the Game Boy Advance
 */
namespace OSCR::Cores::GameBoyAdvance
{
  struct crdbGBARecord
  {
    crc32_t crc32;
    char serial[5];
    uint16_t size;
    uint16_t saveType;
    char name[101];
  };

  extern crdbGBARecord * romDetail;

  void menu();
  void reproMenu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void configureROM();
  void configureSave();
  void configureCart();

  void readROM();

  void rtcTest();

  void readSRAM(uint32_t sramSize, uint32_t pos);
  void writeSRAM(uint32_t sramSize, uint32_t pos);

  void readFRAM(uint32_t framSize);
  void writeFRAM(uint32_t framSize);

  void writeEeprom(uint16_t eepSize);
  void readEeprom(uint16_t eepSize);
  void send(uint16_t currAddr, uint16_t numBits);
  void writeBlock(uint16_t startAddr, uint16_t eepSize);
  void readBlock(uint16_t startAddress, uint16_t eepSize);
  uint32_t verifyEEP(uint16_t eepSize);

  void initOutputFlash();
  void idFlash();
  void resetFlash();
  uint8_t readByteFlash(uint16_t myAddress);
  void writeByteFlash(uint16_t myAddress, uint8_t myData);
  void eraseFlash();
  bool blankcheckFlash(uint32_t flashSize);
  void switchBank(uint8_t bankNum);

  void readFlash(bool browseFile, uint32_t flashSize, uint32_t pos);
  void writeFlash(bool browseFile, uint32_t flashSize, uint32_t pos, bool isAtmel);
  uint32_t verifyFlash(uint32_t flashSize, uint32_t pos);
  void busyCheck(uint16_t currByte);

  void setROM();

  bool compare_checksum();

  uint16_t readWord(uint32_t myAddress);
  void writeWord(uint32_t myAddress, uint16_t myWord);

  uint16_t readWord_GAB(uint32_t myAddress);
  void writeWord_GAB(uint32_t myAddress, uint16_t myWord);

  uint8_t readByte(uint16_t myAddress);
  void writeByte(uint16_t myAddress, uint8_t myData);

  uint8_t checksumHeader(uint8_t const * header);
  void getCartInfo();

  uint8_t getSaveType();
  void printFlashTypeAndWait(char const * caption);

# if HAS_FLASH
  void reproMenu();
  void repro369in1Menu();

  void resetIntel(uint32_t partitionSize);
  void resetMX29GL128E();
  bool sectorCheckMX29GL128E();
  void idFlashrom();
  bool blankcheckFlashrom();
  void eraseIntel4000();
  void eraseIntel4400();
  void sectorEraseMSP55LV128();
  void sectorEraseMX29GL128E();
  void writeIntel4000();
  void writeMSP55LV128();
  void writeMX29GL128E();
  bool verifyFlashrom();

  void reset369in1();
  void mapBlock369in1(uint32_t offset);
  void printblockNumber(int index);
  void printFileSize(int index);
  uint8_t selectBlockNumber(bool option);
  void read369in1(uint8_t blockNumber, uint8_t fileSizeByte);
  void erase369in1(uint8_t blockNumber);
  void write369in1(uint8_t blockNumber);

  void flashRepro(bool option);
# endif /* HAS_FLASH */
} /* namespace OSCR::Cores::GameBoyAdvance */

# endif /* HAS_GBX */
#endif /* OSCR_CORE_GAMEBOYADVANCE_H_ */
