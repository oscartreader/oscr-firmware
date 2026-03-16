#pragma once
#if !defined(OSCR_CORE_MEGADRIVE_H_)
# define OSCR_CORE_MEGADRIVE_H_

# include "config.h"

# if HAS_MD
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Sega MegaDrive/Genesis
 */
namespace OSCR::Cores::MegaDrive
{
  void pulse_clock(int n);
  void menu();
  void cartMenu();
  void segaCDMenu();

  void cartOn();
  void cartOff();

  void readROM();
  void readEEP();
  void writeEEP();
  void readBram();
  void writeBram();

  void force_cartSize();

  uint16_t readWord(uint32_t myAddress);
  void writeWord(uint32_t myAddress, uint16_t myData);

  void enableSram(bool enableSram);
  void readSram();
  void writeSram();

  uint16_t readFlash(uint32_t myAddress);
  void writeFlash(uint32_t myAddress, uint16_t myData);

  void dataOut();
  void dataIn();

  uint8_t copyToRomName(char * output, uint8_t const * input, uint8_t length);
  void getCartInfo();

  void writeSSF2Map(uint32_t myAddress, uint16_t myData);

  void EepromInit(uint8_t eepmode);

  void writeWord_SDA(uint32_t myAddress, uint16_t myData);
  void writeWord_SCL(uint32_t myAddress, uint16_t myData);
  void writeWord_CM(uint32_t myAddress, uint16_t myData);

  void EepromStart();
  void EepromSet0();
  void EepromSet1();

  void EepromDevice();
  void EepromSetDeviceAddress(uint16_t addrhi);

  void EepromStatus();

  void EepromSetAddress(uint16_t address);

  void EepromReadMode();
  void EepromWriteMode();

  void EepromReadData();
  void EepromWriteData(uint8_t data);

  void writeEepromByte(uint16_t address);
  void readEepromByte(uint16_t address);

  void EepromFinish();
  void EepromStop();

  void readRealtec();
  void writeRealtec(uint32_t address, uint8_t value);

#   if HAS_FLASH
  void idFlash();

  void resetFlash();
  void eraseFlash();
  void blankCheck();
  void verifyFlash();
  void busyCheck();

  uint8_t readStatusReg();

  void write29F1610();
  void write29GL();
#   endif /* HAS_FLASH */
} /* namespace OSCR::Cores::MegaDrive */

# endif /* HAS_MD */
#endif /* OSCR_CORE_MEGADRIVE_H_ */
