#pragma once
#if !defined(OSCR_CORE_SFM_H_)
# define OSCR_CORE_SFM_H_

# include "config.h"

# if HAS_SFM
#   include "syslibinc.h"
#   include "common/Types.h"
#   include "ui.h"

/**
 * @brief System core for SFC Memory Modules
 */
namespace OSCR::Cores::SFM
{
  extern byte ready;
  extern byte gameAddress[8];

  void menu();

  void cartOn();

  void printHeader();

  void gameMenu();
  void gameOptions();
  void flashMenu();
  //void getGames(char gameCode[8][20], bool* hasMenu, byte* numGames);
  bool getGames(OSCR::UI::MenuOptions< 7, 20> & gamesMenu);
  void controlOut();
  void controlIn();
  void writeBank(uint8_t myBank, uint16_t myAddress, uint8_t myData);
  uint8_t readBank(uint8_t myBank, uint16_t myAddress);
  void getCartInfo();
  bool checkcart();
  void readROM();

  void resetFlash();
  void resetFlash(int startBank);

  void idFlash(uint8_t startBank);
  bool checkFlashId(uint8_t id);
  bool checkFlashIds();

  void writeFlash();
  void writeFlashData(uint8_t startBank, uint32_t pos);

  void busyCheck(uint8_t startBank);

  void eraseFlash(int startBank);
  uint8_t blankcheck(int startBank);
  uint32_t verifyFlash(int startBank, uint32_t pos);

  void readFlash();
  void printMapping();
  void readMapping();
  void eraseMapping(uint8_t startBank);
  uint8_t blankcheckMapping();
  void writeMapping(uint8_t startBank, uint32_t pos);
  bool unlockHirom(bool unlockWrite = true);
  uint8_t send(uint8_t command);
  void write(uint8_t startBank, uint32_t pos);
} /* namespace OSCR::Cores::SFM */

# endif /* HAS_SFM */
#endif /* OSCR_CORE_SFM_H_ */
