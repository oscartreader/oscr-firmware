//******************************************
// GB SMART MODULE
// Supports 32M cart with LH28F016SUT flash
//******************************************
#include "config.h"

#if ((HAS_GBX) && (HAS_FLASH))
# include "cores/include.h"
# include "cores/GameBoy.h"
# include "cores/GameBoySmart.h"

# define GB_SMART_GAMES_PER_PAGE 6

namespace OSCR::Cores::GameBoySmart
{
  using OSCR::Cores::GameBoy::writeByte;

  /******************************************
     Menu
  *****************************************/
  // GB Smart menu items
  constexpr char const PROGMEM gbSmartMenuItem1[] = "Game Menu";
  constexpr char const PROGMEM gbSmartMenuItem2[] = "Flash Menu";

  constexpr char const * const PROGMEM menuOptionsGBSmart[] = {
    gbSmartMenuItem1,
    gbSmartMenuItem2,
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr char const * const PROGMEM menuOptionsGBSmartFlash[] = {
    OSCR::Strings::MenuOptions::Read,
    OSCR::Strings::MenuOptions::Write,
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr char const PROGMEM gbSmartGameMenuItem4[] = "Switch Game";

  constexpr char const * const PROGMEM menuOptionsGBSmartGame[] = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    gbSmartGameMenuItem4,
    OSCR::Strings::MenuOptions::RefreshCart,
    OSCR::Strings::MenuOptions::Back,
  };

  struct GBSmartGameInfo {
    uint8_t start_bank;
    uint8_t rom_type;
    uint8_t rom_size;
    uint8_t sram_size;
    char title[16];
  };

  uint32_t gbSmartSize = 32 * 131072;
  uint16_t gbSmartBanks = 256;

  uint8_t gbSmartBanksPerFlashChip = 128;
  uint8_t gbSmartBanksPerFlashBlock = 4;
  uint32_t gbSmartFlashBlockSize = (gbSmartBanksPerFlashBlock << 14);

  uint8_t gbSmartRomSizeGB = 0x07;
  uint8_t gbSmartSramSizeGB = 0x04;
  uint8_t gbSmartFlashSizeGB = 0x06;

  uint8_t signature[48];
  uint16_t gameMenuStartBank;

  void menu()
  {
    uint8_t selection = OSCR::UI::menu(FS(OSCR::Strings::Cores::GBSmartModule), menuOptionsGBSmart, sizeofarray(menuOptionsGBSmart));

    switch (selection)
    {
    case 0:
      gameMenu();
      break;

    case 1:
      flashMenu();
      break;

    default:
      return;
    }
  }

  bool refreshCart()
  {
    while (!OSCR::Cores::GameBoy::checkCart())
    {
      switch(OSCR::Prompts::abortRetryContinue())
      {
      case OSCR::Prompts::AbortRetryContinue::Abort:    return false;
      case OSCR::Prompts::AbortRetryContinue::Retry:    continue; // repeat the loop
      case OSCR::Prompts::AbortRetryContinue::Continue: break;    // exit the switch and then exit the loop
      }

      break; // Only reached when continue is selected above.
    }

    return true;
  }

  bool gameOptionsMenu()
  {
    if (!refreshCart()) return false;

    do
    {
      uint8_t selection = OSCR::UI::menu(FS(OSCR::Strings::Cores::GBSmartModule), menuOptionsGBSmartGame, sizeofarray(menuOptionsGBSmartGame));

      switch (selection)
      {
      case 0:  // Read Game
        if (!OSCR::Cores::GameBoy::readROM()) continue;
        break;

      case 1:  // Read SRAM
        if (!OSCR::Cores::GameBoy::readSRAM()) continue;
        break;

      case 2:  // Write SRAM
        if (!OSCR::Cores::GameBoy::writeSRAM()) continue;
        break;

      case 3: // Switch Game
        gameMenuStartBank = 0x02;
        return true;

      case 4:
        if (!refreshCart()) return false;
        break;

      case 5:
        return false;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void gameMenu()
  {
    bool hasMenu;
    uint8_t numGames;
    uint8_t gameSubMenu = 0;
    size_t i = 0;;
    struct GBSmartGameInfo gbSmartGames[GB_SMART_GAMES_PER_PAGE];

    do
    {
      do
      {
        if (gameMenuStartBank > 0xFE)
        {
          gameMenuStartBank = 0x02;
        }

        getGames(gbSmartGames, hasMenu, numGames);

        if (!hasMenu)
        {
          gameSubMenu = 0;
          break;
        }

        OSCR::UI::MenuOptions< 7, 20> menuOptionsGBSmartGames;

        for (i = 0; i < numGames; i++)
        {
          strncpy(menuOptionsGBSmartGames[i], gbSmartGames[i].title, menuOptionsGBSmartGames.length);
          menuOptionsGBSmartGames[i][menuOptionsGBSmartGames.length - 1] = '\0';
        }

        strncpy_P(menuOptionsGBSmartGames[i], OSCR::Strings::Symbol::Ellipsis, menuOptionsGBSmartGames.length);

        gameSubMenu = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectOne), menuOptionsGBSmartGames);
      }
      while (gameSubMenu >= i);

      // copy romname
      strcpy(fileName, gbSmartGames[gameSubMenu].title);

      // select a game
      remapStartBank(gbSmartGames[gameSubMenu].start_bank, gbSmartGames[gameSubMenu].rom_size, gbSmartGames[gameSubMenu].sram_size);

      OSCR::Cores::GameBoy::checkCart();
      OSCR::Cores::GameBoy::printCartInfo();
    }
    while (gameOptionsMenu());
  }

  void flashMenu()
  {
    do
    {
      uint8_t selection = OSCR::UI::menu(FS(OSCR::Strings::Cores::GBSmartModule), menuOptionsGBSmartFlash, sizeofarray(menuOptionsGBSmartFlash));

      switch (selection)
      {
      case 0: // read flash
        readFlash();
        break;

      case 1: // write flash
        writeFlash();
        break;

      default:
        menu();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // take from GB::setup
    // Set RST(PH0) to Input
    DDRH &= ~(1 << 0);
    // Activate Internal Pullup Resistors
    PORTH |= (1 << 0);

    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;

    // Set Control Pins to Output CS(PH3) WR(PH5) RD(PH6) AUDIOIN(PH4) RESET(PH0)
    DDRH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    // Output a high signal on all pins, pins are active low therefore everything is disabled now
    PORTH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;

    delay(400);

    remapStartBank(0x00, 0x00, 0x00);

    for (uint8_t i = 0; i < 0x30; i++)
    {
      signature[i] = readByte(0x0104 + i);
    }

    gameMenuStartBank = 0x02;
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  uint8_t readByte(uint16_t myAddress)
  {
    DDRC = 0x00;
    PORTC = 0xFF;

    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Switch CS(PH3) and RD(PH6) to LOW
    PORTH &= ~((1 << 3) | (1 << 6));

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read
    uint8_t tempByte = PINC;

    // Switch CS(PH3) and RD(PH6) to HIGH
    PORTH |= (1 << 3) | (1 << 6);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    return tempByte;
  }

  void getOneGame(struct GBSmartGameInfo *gbSmartGames, uint8_t bank, uint16_t base)
  {
    uint8_t myByte, myLength = 0;
    uint16_t title_address = base + 0x0134;

    for (uint8_t j = 0; j < 15; j++)
    {
      myByte = readByte(title_address++);

      if (((myByte >= '0' && myByte <= '9') || (myByte >= 'A' && myByte <= 'z')))
      {
        gbSmartGames->title[myLength++] = myByte;
      }
    }

    gbSmartGames->title[myLength] = 0x00;
    gbSmartGames->start_bank = bank;
    gbSmartGames->rom_type = readByte(base + 0x0147);
    gbSmartGames->rom_size = readByte(base + 0x0148);
    gbSmartGames->sram_size = readByte(base + 0x0149);
  }

  void getGames(struct GBSmartGameInfo * gbSmartGames, bool & hasMenu, uint8_t & numGames)
  {
    static constexpr uint8_t const menu_title[] = { 0x47, 0x42, 0x31, 0x36, 0x4D };
    uint16_t i;

    cartOn();

    // reset remap setting
    remapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

    // check if contain menu
    hasMenu = true;

    for (i = 0; i < 5; i++)
    {
      if (readByte(0x0134 + i) != menu_title[i])
      {
        hasMenu = false;
        break;
      }
    }

    if (hasMenu)
    {
      for (i = gameMenuStartBank, numGames = 0; i < gbSmartBanks && numGames < GB_SMART_GAMES_PER_PAGE;)
      {
        // switch bank
        writeByte(0x2100, i);

        // read signature
        for (uint8_t j = 0x00; j < 0x30; j++)
        {
          if (readByte(0x4104 + j) != signature[j])
          {
            i += 0x02;
            continue;
          }
        }

        getOneGame(&gbSmartGames[numGames], i, 0x4000);

        i += (2 << gbSmartGames[(numGames)++].rom_size);
      }

      gameMenuStartBank = i;
    }
    else
    {
      getOneGame(&gbSmartGames[0], 0, 0);

      numGames = 1;
      gameMenuStartBank = 0xFE;
    }

    cartOff();
  }

  void readFlash()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GBSmartModule), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

    // reset flash to read array state
    for (uint16_t i = 0x00; i < gbSmartBanks; i += gbSmartBanksPerFlashChip)
    {
      resetFlash(i);
    }

    // remaps mmc to full access
    remapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

    // dump fixed bank 0x00
    for (uint16_t addr = 0x0000; addr <= 0x3FFF; addr += 512)
    {
      for (uint16_t c = 0; c < 512; c++)
        OSCR::Storage::Shared::buffer[c] = readByte(addr + c);

      OSCR::Storage::Shared::writeBuffer();
    }

    // read rest banks
    for (uint16_t bank = 0x01; bank < gbSmartBanks; bank++)
    {
      writeByte(0x2100, bank);

      for (uint16_t addr = 0x4000; addr <= 0x7FFF; addr += 512)
      {
        for (uint16_t c = 0; c < 512; c++)
        {
          OSCR::Storage::Shared::buffer[c] = readByte(addr + c);
        }

        OSCR::Storage::Shared::writeBuffer();
      }
    }

    // back to initial state
    writeByte(0x2100, 0x01);

    OSCR::Storage::Shared::close();
  }

  void writeFlash()
  {
    if (!OSCR::Prompts::confirmErase()) return;

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    for (uint16_t bank = 0x00; bank < gbSmartBanks; bank += gbSmartBanksPerFlashChip)
    {
      OSCR::UI::clear();

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

      eraseFlash(bank);
      resetFlash(bank);

      OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

      if (!blankCheckingFlash(bank))
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
        OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
        return;
      }

      OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));

      // write full chip
      writeFlash(bank);

      // reset chip
      writeFlashByte(0x0000, 0xFF);
    }

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    OSCR::Storage::Shared::rewind();

    // remaps mmc to full access
    remapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

    for (uint16_t addr = 0x0000; addr <= 0x3FFF; addr += 512)
    {
      OSCR::Storage::Shared::fill();

      for (uint16_t c = 0; c < 512; c++)
      {
        if (readByte(addr + c) != OSCR::Storage::Shared::buffer[c])
          writeErrors++;
      }
    }

    // verify rest banks
    for (uint16_t bank = 0x01; bank < gbSmartBanks; bank++)
    {
      writeByte(0x2100, bank);

      for (uint16_t addr = 0x4000; addr <= 0x7FFF; addr += 512)
      {
        OSCR::Storage::Shared::fill();

        for (uint16_t c = 0; c < 512; c++)
        {
          if (readByte(addr + c) != OSCR::Storage::Shared::buffer[c])
          {
            writeErrors++;
          }
        }
      }
    }

    // back to initial state
    writeByte(0x2100, 0x01);

    OSCR::Storage::Shared::close();

    if (writeErrors == 0)
    {
      OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }
  }

  void writeFlash(uint32_t start_bank)
  {
    // switch to flash base bank
    remapStartBank(start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

    OSCR::Storage::Shared::sharedFile.seekCur((start_bank << 14));

    OSCR::UI::printHex(start_bank);
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    // handle bank 0x00 on 0x0000
    writeFlashFromMyFile(0x0000);

    // handle rest banks on 0x4000
    for (uint8_t bank = 0x01; bank < gbSmartBanksPerFlashChip; bank++)
    {
      writeByte(0x2100, bank);

      writeFlashFromMyFile(0x4000);
    }

    OSCR::Storage::Shared::close();
    OSCR::UI::printLine();
  }

  void writeFlashFromMyFile(uint32_t addr)
  {
    for (uint16_t i = 0; i < 16384; i += 256)
    {
      OSCR::Storage::Shared::readBuffer(256);

      // sequence load to page
      writeFlashByte(addr, 0xE0);
      writeFlashByte(addr, 0xFF);
      writeFlashByte(addr, 0x00);  // BCH should be 0x00

      // fill page buffer
      for (int d = 0; d < 256; d++)
      {
        writeFlashByte(d, OSCR::Storage::Shared::buffer[d]);
      }

      // start flashing page
      writeFlashByte(addr, 0x0C);
      writeFlashByte(addr, 0xFF);
      writeFlashByte(addr + i, 0x00);  // BCH should be 0x00

      // waiting for finishing
      while ((readByte(addr + i) & 0x80) == 0x00);
    }
  }

  uint8_t blankCheckingFlash(uint8_t flash_start_bank)
  {
    remapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

    // check first bank
    for (uint16_t addr = 0x0000; addr <= 0x3FFF; addr++)
    {
      if (readByte(addr) != 0xFF)
        return 0;
    }

    // check rest banks
    for (uint16_t bank = 0x01; bank < gbSmartBanksPerFlashChip; bank++)
    {
      writeByte(0x2100, bank);

      for (uint16_t addr = 0x4000; addr <= 0x7FFF; addr++)
      {
        if (readByte(addr) != 0xFF)
          return 0;
      }
    }

    return 1;
  }

  void resetFlash(uint8_t flash_start_bank)
  {
    remapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);
    writeFlashByte(0x0, 0xFF);
  }

  void eraseFlash(uint8_t flash_start_bank)
  {
    remapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

    // handling first flash block
    writeFlashByte(0x0000, 0x20);
    writeFlashByte(0x0000, 0xD0);

    while ((readByte(0x0000) & 0x80) == 0x00);

    // rest of flash block
    for (uint32_t ba = gbSmartBanksPerFlashBlock; ba < gbSmartBanksPerFlashChip; ba += gbSmartBanksPerFlashBlock)
    {
      writeByte(0x2100, ba);

      writeFlashByte(0x4000, 0x20);
      writeFlashByte(0x4000, 0xD0);

      while ((readByte(0x4000) & 0x80) == 0x00);
    }
  }

  void writeFlashByte(uint32_t myAddress, uint8_t myData)
  {
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTC = myData;

    // wait for 62.5 x 4 = 250ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Pull FLASH_WE (PH4) low
    PORTH &= ~(1 << 4);

    // pull low for another 250ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Pull FLASH_WE (PH4) high
    PORTH |= (1 << 4);

    // pull high for another 250ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );
  }

  // rom_start_bank = 0x00 means back to original state
  void remapStartBank(uint8_t rom_start_bank, uint8_t rom_size, uint8_t sram_size)
  {
    rom_start_bank &= 0xFE;

    // clear base bank setting
    writeByte(0x1000, 0xA5);
    writeByte(0x7000, 0x00);
    writeByte(0x1000, 0x98);
    writeByte(0x2000, rom_start_bank);

    if (rom_start_bank > 1)
    {
      // start set new base bank
      writeByte(0x1000, 0xA5);

      rom_start_bank = getResizeParam(rom_size, sram_size);

      writeByte(0x7000, rom_start_bank);
      writeByte(0x1000, 0x98);

      writeByte(0x2100, 0x01);
    }
  }

  // Get magic number for 0x7000 register.
  // Use for setting correct rom and sram size
  // Code logic is take from SmartCard32M V1.3 menu code,
  // see 0x2DB2 to 0x2E51 (0xA0 bytes)
  uint8_t getResizeParam(uint8_t rom_size, uint8_t sram_size)
  {
    if (rom_size < 0x0F)
    {
      rom_size &= 0x07;
      rom_size ^= 0x07;
    }
    else
    {
      rom_size = 0x01;
    }

    if (sram_size > 0)
    {
      if (sram_size > 1)
      {
        sram_size--;
        sram_size ^= 0x03;
        sram_size <<= 4;
        sram_size &= 0x30;
      }
      else
      {
        sram_size = 0x20;  //  2KiB treat as 8KiB
      }
    }
    else
    {
      sram_size = 0x30;  // no sram
    }

    return (sram_size | rom_size);
  }
} /* namespace OSCR::Cores::GameBoySmart */

#endif /* ENABLE_GBX && HAS_FLASH */
