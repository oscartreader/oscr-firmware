#pragma once
#if !defined(OSCR_CORE_SNES_H_)
# define OSCR_CORE_SNES_H_

# include "config.h"

# if HAS_SNES
#   include "syslibinc.h"
#   include "common/Types.h"
#   include "common/crc32.h"

#   define snesCRDB ((OSCR::Databases::SNES *)cartCRDB)

/**
 * @brief Strings for the %SNES core
 */
namespace OSCR::Strings::SNES
{
  extern char const PROGMEM SlowROM[];
  extern char const PROGMEM FastROM[];
  extern char const PROGMEM HiROM[];
  extern char const PROGMEM LoROM[];
  extern char const PROGMEM ExHiRom[];

  extern char const PROGMEM DSP1[];
  extern char const PROGMEM DSP2[];
  extern char const PROGMEM SDD1[];
  extern char const PROGMEM SRTC[];
  extern char const PROGMEM BATT[];
  extern char const PROGMEM SPC[];
  extern char const PROGMEM RTClock[];
  extern char const PROGMEM SA1[];

  extern char const * const ICTemplate3;
  extern char const * const ICTemplate2;
  extern char const * const ICTemplate1;
}

/**
 * @brief System core for the %SNES
 */
namespace OSCR::Cores::SNES
{
  struct crdbSNESRecord
  {
    crc32_t crc32;
    uint16_t chksum;
    crc32_t id32;
    uint32_t size;
    uint16_t banks;
    char name[101];
  };

  extern crdbSNESRecord * romDetail;

  void reproCFIMenu();
  void reproMenu();
  void menu();
  void snesMenu();
  void confMenuManual(bool disallowBack);

  void cartOn();
  void cartOff();

  void openCRDB();
  void closeCRDB();

  void printHeader();

  void dataIn();
  void dataOut();
  void controlOut();
  void controlIn();
  void writeBank(uint8_t myBank, uint16_t myAddress, uint8_t myData);
  uint8_t readBank(uint8_t myBank, uint16_t myAddress);
  void readLoRomBanks(uint16_t start, uint16_t total, OSCR::Storage::File * file);
  void readHiRomBanks(uint16_t start, uint16_t total, OSCR::Storage::File * file);
  void getCartInfo();
  bool checkcart();
  void checkAltConf(uint16_t chksumSearch, crc32_t & crc32search);

  uint16_t calc_checksum();
  bool compare_checksum();
  bool matchCRC();

  void nextCFI();

  void readROM();
  void writeSRAM(bool browseFile);
  void readSRAM();
  uint32_t verifySRAM();
  bool eraseSRAM(uint8_t b);
} /* namespace OSCR::Cores::SNES */

# endif /* HAS_SNES */
#endif /* OSCR_CORE_SNES_H_ */
