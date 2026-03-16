#pragma once
#if !defined(OSCR_CORE_N64_H_)
# define OSCR_CORE_N64_H_

# include "config.h"

# if HAS_N64
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Nintendo 64
 */
namespace OSCR::Cores::N64
{
  void menu();
  void cartMenu();
  void menuSaveType();
  void controllerMenu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void setupController();

  inline void adOut();
  inline void adIn();
  inline void setAddress(uint32_t myAddress);

  uint16_t readWord();
  void writeWord(uint16_t myWord);

  void sendJoyBus(uint8_t const * buffer, char length);
  uint16_t recvJoyBus(uint8_t * output, uint8_t byte_count);
  void get_button();

  void resetController();
  void checkController();
  uint8_t readBlock(byte* output, uint16_t myAddress);
  bool readMPK();
  bool checkHeader(byte* buf);
  bool writeMPK();

  bool refreshCart();
  bool getCartInfo();
  void idCart();

  bool readSave();
  bool writeSave();

  bool readEepromPageList(byte* output, uint8_t page_number, uint8_t page_count);
  bool writeEeprom();
  void resetEeprom();
  void readEeprom();

  bool readSaveData(uint32_t sramSize, bool isFlash);
  bool verifySaveData(uint32_t maxAddress, uint16_t offset, uint16_t bufferSize, bool isFlash);

  bool writeSRAM(uint32_t sramSize);
  bool writeFRAM();

  void sendFramCmd(uint32_t myCommand);
  void initFRAM();

  uint8_t waitForFram(uint8_t flashramType);
  uint8_t getFramType();

  void readRom();

  uint32_t blankcheck(uint8_t flashramType);

# if defined(ENABLE_CONTROLLERTEST)
#   ifdef ENABLE_SERIAL
  void controllerTest_Serial();
#   endif /* ENABLE_SERIAL */

#   if ((HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864) || defined(ENABLE_OLED))
  void printSTR(String st, int x, int y);
  void nextscreen();
  void controllerTest_Display();
#   endif /* ENABLE_LCD || ENABLE_OLED */
# endif /* ENABLE_CONTROLLERTEST */

  void sendFlashromGamesharkCommand(uint16_t cmd);
  bool flashGameshark();
  void unlockGSAddressRanges();
  bool idGameshark();
  void resetGameshark();
  void backupGameshark();
  void eraseGameshark();
  void writeGameshark();
  uint32_t verifyGameshark();

  void blankCheck();

  void sendFlashromXplorerCommand(uint16_t cmd);
  void flashXplorer();
  bool idXplorer();
  void resetXplorer();
  void backupXplorer();
  uint32_t unscramble(uint32_t addr);
  uint32_t scramble(uint32_t addr);
  void oddXPaddrWrite(uint32_t addr, uint16_t data);
  void evenXPaddrWrite(uint32_t addr, uint16_t data);
  void eraseXplorer();
  void blankCheck_XP64();
  void writeXplorer();
  uint32_t verifyXplorer();

# if HAS_FLASH
  void flashRepro();
  void sendFlashromCommand(uint32_t addr, uint8_t cmd);
  void idFlashrom();

  void resetFlashrom(uint32_t flashBase);
  void eraseFlashrom();
  void writeFlashrom(uint32_t sectorSize);

  uint32_t verifyFlashrom();

  void writeFlashBuffer(uint32_t sectorSize, uint8_t bufferSize);

  void resetIntel4400();
  void eraseIntel4400();
  void writeIntel4400();

  void resetMSP55LV100(uint32_t flashBase);
  void eraseMSP55LV100();
  void writeMSP55LV100(uint32_t sectorSize);

  void eraseSector(uint32_t sectorSize);
  bool blankcheckFlashrom();
# endif /* HAS_FLASH */
} /* namespace OSCR::Cores::N64 */

# endif /* HAS_N64 */
#endif /* OSCR_CORE_N64_H_ */
