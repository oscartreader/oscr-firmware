#pragma once
#if !defined(OSCR_CORE_GAMEBOY_H_)
# define OSCR_CORE_GAMEBOY_H_

# include "config.h"

# if HAS_GBX
#   include "syslibinc.h"
#   include "common/Types.h"

namespace OSCR::Cores
{
  /**
   * @brief System core for Game Boy X devices
   */
  namespace GameBoyX
  {
    extern char const PROGMEM GBFlashItem6[];

    void menu();
    void menuPelican();
    void menuDatel();
    void menuFlash();
    void menu29F();

# if HAS_FLASH
    bool flashCFI();
# endif /* HAS_FLASH */
  } /* namespace OSCR::Cores::GameBoyX */

  /**
   * @brief System core for the Game Boy
   */
  namespace GameBoy
  {
    enum class MenuOption : uint8_t
    {
      ReadROM,
      ReadSave,
      WriteSave,
      RefreshCart,
      Back,
    };

    constexpr uint8_t const kMenuOptionMax = 5;

    extern bool audioWE;
    extern uint16_t lastByteAddress;

    void menu();

    void openCRDB();
    void closeCRDB();

    void cartOn();
    void cartOff();

    void setupPort();

    bool checkCart();
    void printCartInfo();
    uint8_t refreshCart(__FlashStringHelper const * menuOptions[], MenuOption * const menuOptionMap);

    bool readROM();
    bool readSave();
    bool writeSave();

    bool readSRAM();
    bool writeSRAM();

    bool readSRAMFLASH_MBC6();
    bool writeSRAMFLASH_MBC6();

    bool readEEPROM_MBC7();
    bool writeEEPROM_MBC7();
    void sendMBC7EEPROM_Inst(uint8_t op, uint8_t addr, uint16_t data);

    void disableFlashSaveMemory();

    uint8_t readByte(uint16_t myAddress);
    void writeByte(int myAddress, uint8_t myData);
    void writeByte(int myAddress, uint8_t myData, bool audio_as_WE);

    uint8_t readByteSRAM(uint16_t myAddress);
    void writeByteSRAM(int myAddress, uint8_t myData);

    void sendW29C020Command(uint8_t cmd);
    void sendW29C020CommandSufix(uint8_t cmd);
    void send28LF040Potection(bool enable);
    void readPelican();
    void writePelican();
    bool isToggle(uint8_t byte1, uint8_t byte2);

    void readMegaMem();
    void writeMegaMem();

    void sendGamesharkCommand(uint8_t cmd);
    void readGameshark();
    void writeGameshark();

    void sendFlashCommand(uint8_t cmd, bool commandSet);
    void busyCheck(uint32_t address, uint8_t data);
    void writeFlash(uint8_t MBC, bool commandSet, bool flashErase);
    void sendCFICommand(uint8_t cmd);
    uint8_t readByteCompensated(int address);
    void writeByteCompensated(int address, uint8_t data);
    bool identifyCFI();
    bool writeCFI();
  } /* namespace OSCR::Cores::GameBoy */
} /* namespace OSCR::Cores */

# endif /* HAS_GBX */
#endif /* OSCR_CORE_GAMEBOY_H_ */
