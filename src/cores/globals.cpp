

#include "cores/include.h"
#include "common/Util.h"

namespace OSCR::Cores
{
  // Data Direction
  __constinit DataDirection dataDir = DataDirection::Unknown;

  // ROM Size
  __constinit uint32_t romSize;

  // Cartridge Size
  __constinit uint32_t cartSize;

  // Number of Banks
  __constinit uint32_t numBanks;

  // 21 chars for ROM name, one char for termination
  constexpr uint8_t const kFileNameMax = 22;
  __constinit char fileName[kFileNameMax] = {};

  __constinit uint32_t sramSize;
  __constinit uint16_t romType;
  __constinit uint8_t saveType;

  __constinit char checksumStr[9];
  __constinit uint16_t checksum;
  __constinit uint8_t romVersion;
  __constinit char cartID[5];
  __constinit char vendorID[5];
  __constinit uint32_t fileSize;
  __constinit uint32_t sramBase;

  __constinit uint8_t eepbit[8]; // used by MD and NES modules

  crc32_t crc32sum;

  __constinit uint32_t writeErrors = 0;

  __constinit void * cartCRDB = nullptr;
  __constinit bool fromCRDB = false;

  bool useDefaultName()
  {
    return OSCR::Util::copyStr_P(BUFFN(fileName), OSCR::Strings::FileType::DefaultName);
  }

  uint8_t setOutName(char const * const src, uint8_t const srcMaxLen)
  {
    return OSCR::Storage::copyFileName(src, srcMaxLen, fileName, kFileNameMax);
  }

  uint8_t setOutName_P(char const * const src)
  {
    return OSCR::Storage::copyFileName_P(src, fileName, kFileNameMax);
  }

  void resetCRDB()
  {
    cartCRDB = nullptr;
    fromCRDB = false;
  }

  void resetGlobals()
  {
    romSize = 0;
    cartSize = 0;
    numBanks = 128;
    sramSize = 0;
    romType = 0;
    saveType = 0;
    checksumStr[0] = '\0';
    checksum = 0;
    romVersion = 0;
    cartID[0] = '\0';
    vendorID[0] = '\0';
    fileSize = 0;
    sramBase = 0;
    eepbit[0] = '\0';
    writeErrors = 0;
    dataDir = DataDirection::Unknown;

    resetCRDB();

    useDefaultName();
  }
}
