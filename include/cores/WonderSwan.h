#pragma once
#if !defined(OSCR_CORE_WONDERSWAN_H_)
# define OSCR_CORE_WONDERSWAN_H_

# include "config.h"

# if HAS_WS
#   include "syslibinc.h"
#   include "common/Types.h"

#   ifdef OPTION_WS_ADAPTER_V2
#     define WS_CLK_BIT 5  // USE PE5 as CLK
#   else
#     define WS_CLK_BIT 3  // USE PE3 as CLK
#   endif

/**
 * @brief System core for the Bandai %WonderSwan and the Benesse Pocket Challenge V2
 */
namespace OSCR::Cores::WonderSwan
{
  enum class MenuOption : uint8_t
  {
    ReadROM,
    ReadSave,
    WriteSave,
    RefreshCart,
    WriteWitchOS,
    Back,
  };

  constexpr uint8_t const kMenuOptionMax = 6;

  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t refreshCart(__FlashStringHelper const * menuOptions[], MenuOption * const menuOptionMap, bool & hasWitchOS);
  bool headerCheck();
  bool getCartInfo();

  void readROM(bool hasWitchOS);

  void readSave();
  void readSRAM();
  void readEEPROM();

  void writeSave();
  void writeSRAM();
  void writeEEPROM();

  void writeWitchOS();

  void fastProgramWitchFlash(uint32_t addr, uint16_t data);
  void eraseWitchFlashSector(uint32_t sector_addr);
  uint8_t readBytePort(uint8_t port);
  uint8_t readByte(uint32_t addr);
  uint16_t readWord(uint32_t addr);

  void writeBytePort(uint8_t port, uint8_t data);
  void writeByte(uint32_t addr, uint8_t data);
  void writeWord(uint32_t addr, uint16_t data);

  void unprotectEEPROM();

  void generateEepromInstruction(uint8_t *instruction, uint8_t opcode, uint16_t addr);
  bool unlockMMC2003();
  void pulseCLK(uint8_t count);

  void dataIn();
  void dataOut();
} /* namespace OSCR::Cores::WonderSwan */

# endif /* HAS_WS */
#endif /* OSCR_CORE_WONDERSWAN_H_ */
