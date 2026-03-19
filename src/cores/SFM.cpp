//******************************************
// SF MEMORY MODULE
//******************************************
#include "config.h"

#if HAS_SFM
# include "cores/include.h"
# include "cores/SFM.h"
# include "cores/SNES.h"
# include "cores/Flash.h"

namespace OSCR::Cores::SFM
{
  /******************************************
     SF Memory Clock Source
  ******************************************/
  // The clock signal for the SF Memory cassette
  // is generated with the Adafruit Clock Generator
  // or a similar external clock source

  using OSCR::Cores::Flash::flashid;
  using OSCR::Cores::SNES::cartOff;

  /******************************************
     Variables
  *****************************************/
  // SF Memory status
  uint8_t ready = 0;

  // Arrays that hold game info
  uint8_t gameAddress[8];


  /******************************************
    Menu
  *****************************************/
  // SFM menu items
  constexpr char const PROGMEM sfmMenuItem1[] = "Game Menu";
  constexpr char const PROGMEM sfmMenuItem2[] = "Flash Menu";

  constexpr char const * const PROGMEM menuOptions[] = {
    sfmMenuItem1,
    sfmMenuItem2,
    OSCR::Strings::MenuOptions::Back,
  };

  // SFM flash menu items
  constexpr char const sfmFlashMenuItem1[] PROGMEM = "Read Flash";
  constexpr char const sfmFlashMenuItem2[] PROGMEM = "Write Flash";
  constexpr char const sfmFlashMenuItem3[] PROGMEM = "Print Mapping";
  constexpr char const sfmFlashMenuItem4[] PROGMEM = "Read Mapping";
  constexpr char const sfmFlashMenuItem5[] PROGMEM = "Write Mapping";
  constexpr char const sfmFlashMenuItem6[] PROGMEM = "Back";

  constexpr char const * const PROGMEM menuOptionsFlash[] = {
    sfmFlashMenuItem1,
    sfmFlashMenuItem2,
    sfmFlashMenuItem3,
    sfmFlashMenuItem4,
    sfmFlashMenuItem5,
    sfmFlashMenuItem6,
  };

  // SFM game menu items
  constexpr char const PROGMEM sfmGameMenuItem2[] = "Read Game";
  constexpr char const PROGMEM sfmGameMenuItem4[] = "Switch Game";

  constexpr char const * const PROGMEM menuOptionsGame[] = {
    OSCR::Strings::MenuOptions::ReadSave,
    sfmGameMenuItem2,
    OSCR::Strings::MenuOptions::WriteSave,
    sfmGameMenuItem4,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    OSCR::Cores::SNES::openCRDB();

    do
    {
      uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::Cores::SFM), menuOptions, sizeofarray(menuOptions));

      switch (menuSelection)
      {
      case 0: // Game menu
        gameMenu();
        break;

      case 1: // Flash menu
        flashMenu();
        break;

      case 2: // Back
        OSCR::Cores::SNES::closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while(true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::SFM));
  }

  void gameMenu()
  {
    if (!unlockHirom(false)) return;

    OSCR::UI::MenuOptions< 7, 20> gamesMenu;

    uint8_t const nameMax = min(menuoptionsize(gamesMenu), kFileNameMax);

    delay(300);

    // Fill arrays with data
    if (getGames(gamesMenu))
    {
      // Create menu of options to choose from wait for user to choose one.
      uint8_t const gameSubMenu = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectType), gamesMenu);
      uint8_t timeout = 0;

      do
      {
        // Switch to game
        send(gameSubMenu + 0x80);

        delay(200);

        timeout++;

        // Abort, something is wrong
        if (timeout > 5)
        {
          OSCR::UI::clear();
          OSCR::UI::fatalError();

          OSCR::UI::printHexLine(readBank(0, 0x2400));

          OSCR::UI::printLine();

          OSCR::UI::fatalError(FS(OSCR::Strings::Errors::TimedOut));
        }
      }
      while (readBank(0, 0x2400) != 0x7D);

      // Copy gameCode to fileName in case of japanese chars in fileName
      strlcpy(fileName, gamesMenu[gameSubMenu + 1], nameMax);
    }
    else
    {
      // No menu so switch to only game
      // Switch to game
      send(0x80);
      delay(200);

      // Copy gameCode to fileName in case of japanese chars in fileName
      strlcpy(fileName, gamesMenu[0], nameMax);
    }

    // Print info
    getCartInfo();

    gameOptions();
  }

  void gameOptions()
  {
    uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::Cores::SFM), menuOptionsGame, sizeofarray(menuOptionsGame));

    switch (menuSelection)
    {
    case 0: // Read sram
      OSCR::Cores::SNES::readSRAM();
      break;

    case 1: // Read rom
      readROM();
      OSCR::Cores::SNES::compare_checksum();
      OSCR::Cores::SNES::matchCRC();
      break;

    case 2: // Write sram
      OSCR::Cores::SNES::writeSRAM(1);

      writeErrors = OSCR::Cores::SNES::verifySRAM();

      if (writeErrors != 0)
      {
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
      }
      break;

    case 3: // Switch game
      gameMenu();
      return;

    case 4: // Back
      return;
    }

    OSCR::UI::waitButton();
  }

  void flashMenu()
  {
    do
    {
      uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::Cores::SFM), menuOptionsFlash, sizeofarray(menuOptionsFlash));

      switch (menuSelection)
      {
      case 0: // Read Flash
        // Reset to HIROM ALL
        romType = 1;

        if (!unlockHirom(false)) continue;

        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));

        // Reset flash
        resetFlash();

        OSCR::Cores::Flash::flashSize = 4194304;
        numBanks = 64;

        OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SFM), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

        // Read flash
        readFlash();
        break;

      case 1: // Write Flash
        if (!OSCR::Prompts::confirmErase()) continue;
        writeFlash();
        break;

      case 2: // Print mapping
        // Reset to HIROM ALL
        romType = 1;

        if (!unlockHirom(false)) continue;

        if (!checkFlashIds()) continue;

        // Reset flash
        resetFlash();
        delay(100);
        printMapping();
        resetFlash();

        cartOff();
        break;

      case 3: // Read mapping
        // Reset to HIROM ALL
        romType = 1;

        if (!unlockHirom(false)) continue;

        if (!checkFlashIds()) continue;

        // Reset flash
        resetFlash();
        delay(100);
        readMapping();
        resetFlash();

        cartOff();
        break;

      case 4: // Write mapping
        if (!OSCR::Prompts::confirmErase()) continue;

        // Erase mapping
        eraseMapping(0xD0);
        eraseMapping(0xE0);

        OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

        if (!blankcheckMapping())
        {
          OSCR::UI::error(FS(OSCR::Strings::Common::FAIL));
          break;
        }

        // Launch file browser
        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

        // Write mapping
        writeMapping(0xD0, 0);
        writeMapping(0xE0, 256);

        cartOff();
        break;

      // Go back
      case 5:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  // Read the games from the menu area
  bool getGames(OSCR::UI::MenuOptions< 7, 20> & gamesMenu)
  {
    bool hasMenu = true;

    // Check if menu is present
    uint8_t menuString[] = { 0x4D, 0x45, 0x4E, 0x55, 0x20, 0x50, 0x52, 0x4F, 0x47, 0x52, 0x41, 0x4D };

    for (int i = 0; i < 12; i++)
    {
      if (menuString[i] != readBank(0xC0, 0x7FC0 + i))
      {
        hasMenu = false;
      }
    }

    if (hasMenu)
    {
      // Count number of games
      gamesMenu.count = 0;

      for (uint16_t i = 0x0000; i < 0xE000; i += 0x2000)
      {
        if (readBank(0xC6, i) == gamesMenu.count)
          gamesMenu.count++;
      }

      // Get game info
      for (int i = 0; i < gamesMenu.count; i++)
      {
        // Read starting address and size
        gameAddress[i] = 0xC0 + readBank(0xC6, i * 0x2000 + 0x01) * 0x8;

        // Read game code
        uint8_t myByte = 0;
        uint8_t myLength = 0;
        for (int j = 0; j < 9; j++)
        {
          myByte = readBank(0xC6, i * 0x2000 + 0x07 + j);

          // Remove funny characters
          if (((myByte >= ',' && myByte <= '9') || (myByte >= 'A' && myByte <= 'z')) && myLength < 9)
          {
            gamesMenu[i][myLength] = myByte;
            myLength++;
          }
        }

        // End char array in case game code is less than 9 chars
        gamesMenu[i][myLength] = '\0';
      }
    }
    else
    {
      uint16_t base;
      gamesMenu.count = 1;

      // Check if hirom
      if (readBank(0xC0, 0xFFD5) == 0x31)
      {
        base = 0xFF00;
      }
      else
      {
        base = 0x7F00;
      }

      // gameVersion[0] = readBank(0xC0, base + 0xDB);
      gamesMenu[0][0] = 'G';
      gamesMenu[0][1] = 'A';
      gamesMenu[0][2] = 'M';
      gamesMenu[0][3] = 'E';
      gamesMenu[0][4] = '-';
      gamesMenu[0][5] = readBank(0xC0, base + 0xB2);
      gamesMenu[0][6] = readBank(0xC0, base + 0xB3);
      gamesMenu[0][7] = readBank(0xC0, base + 0xB4);
      gamesMenu[0][8] = readBank(0xC0, base + 0xB5);
      gamesMenu[0][9] = '\0';
      // gameSize[0] = 1 << (readBank(0xC0, base + 0xD7) - 7);
    }

    return hasMenu;
  }

  /******************************************
     Setup
  *****************************************/
  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set cicrstPin(PG1) to Output
    DDRG |= (1 << 1);
    // Output a high signal to disable snesCIC
    PORTG |= (1 << 1);
    // Set cichstPin(PG0) to Input
    DDRG &= ~(1 << 0);

    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //BA0-BA7
    DDRL = 0xFF;

    // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
    DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
    // Switch RST(PH0) and WR(PH5) to HIGH
    PORTH |= (1 << 0) | (1 << 5);
    // Switch CS(PH3) and RD(PH6) to LOW
    PORTH &= ~((1 << 3) | (1 << 6));

    // Set IRQ(PH4) to Input
    DDRH &= ~(1 << 4);
    // Activate Internal Pullup Resistors
    //PORTH |= (1 << 4);

    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
    // Enable Internal Pullups
    //PORTC = 0xFF;

    // Unused pins
    // Set CPU Clock(PH1) to Output
    DDRH |= (1 << 1);
    //PORTH &= ~(1 << 1);

# ifdef ENABLE_CLOCKGEN
    // Clock Generator
    if (!OSCR::ClockGen::initialize())
    {
      OSCR::UI::clear();
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::ClockGenMissing));
    }

    OSCR::ClockGen::clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
    OSCR::ClockGen::clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
    OSCR::ClockGen::clockgen.set_freq(2147727200ULL, SI5351_CLK0);

    // start outputting master clock
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 0);
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK2, 0);
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK0, 1);
# endif /* ENABLE_CLOCKGEN */

    // Wait until all is stable
    delay(500);

    // Switch to HiRom All
    uint8_t timeout = 0;
    send(0x04);
    delay(200);
    while (readBank(0, 0x2400) != 0x2A)
    {
      delay(100);

      // Try again
      send(0x04);
      delay(100);
      timeout++;

      // Abort, something is wrong
      if (timeout == 5)
      {
        OSCR::UI::fatalError(FS(OSCR::Strings::Errors::TimedOut));
      }
    }
  }

  /******************************************
     I/O Functions
  *****************************************/
  // Switch control pins to write
  void controlOut()
  {
    // Switch RD(PH6) and WR(PH5) to HIGH
    PORTH |= (1 << 6) | (1 << 5);
    // Switch CS(PH3) to LOW
    PORTH &= ~(1 << 3);
  }

  // Switch control pins to read
  void controlIn()
  {
    // Switch WR(PH5) to HIGH
    PORTH |= (1 << 5);
    // Switch CS(PH3) and RD(PH6) to LOW
    PORTH &= ~((1 << 3) | (1 << 6));
  }

  /******************************************
     Low level functions
  *****************************************/
  void readyBank(uint8_t bank)
  {
    // Reset to defaults
    writeBank(bank, 0x0000, 0x38);
    writeBank(bank, 0x0000, 0xD0);

    // Read Extended Status Register (GSR and PSR)
    writeBank(bank, 0x0000, 0x71);

    // Page Buffer Swap
    writeBank(bank, 0x0000, 0x72);

    // Read Page Buffer
    writeBank(bank, 0x0000, 0x75);
  }

  void writeBankSequence(uint8_t data, uint8_t bank = 0)
  {
    if (romType)
    {
      writeBank(bank, 0x5555L * 2, 0xAA);
      writeBank(bank, 0x2AAAL * 2, 0x55);
      writeBank(bank, 0x5555L * 2, data);
    }
    else
    {
      writeBank(1, 0x8000 + 0x1555L * 2, 0xAA);
      writeBank(0, 0x8000 + 0x2AAAL * 2, 0x55);
      writeBank(1, 0x8000 + 0x1555L * 2, data);
    }
  }

  // Write one byte of data to a location specified by bank and address, 00:0000
  void writeBank(uint8_t myBank, uint16_t myAddress, uint8_t myData)
  {
    DDRC = 0xFF;

    PORTL = myBank;
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTC = myData;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Switch WR(PH5) to LOW
    PORTH &= ~(1 << 5);

    // Leave WR low for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Switch WR(PH5) to HIGH
    PORTH |= (1 << 5);

    // Leave WR high for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  }

  // Read one byte of data from a location specified by bank and address, 00:0000
  uint8_t readBank(uint8_t myBank, uint16_t myAddress)
  {
    DDRC = 0x00;
    PORTC = 0xFF;

    PORTL = myBank;
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Read
    uint8_t tempByte = PINC;
    return tempByte;
  }

  /******************************************
    SNES ROM Functions
  ******************************************/
  void getCartInfo()
  {
    // Print start page
    if (checkcart() == 0)
    {
      // Checksum either corrupt or 0000
      OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
      OSCR::UI::printLine(F("Rom header corrupt"));
      OSCR::UI::printLine(F("or missing"));
      OSCR::UI::waitButton();
    }

    OSCR::UI::printValue(OSCR::Strings::Common::Name, fileName);

    OSCR::UI::printValue(OSCR::Strings::Common::Revision, romVersion);

    OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);
    OSCR::UI::printHexLine(checksum);

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, romSize * 1024 * 1024);

    OSCR::UI::printType_P(OSCR::Strings::Common::Cart, romType ? OSCR::Strings::SNES::HiROM : OSCR::Strings::SNES::LoROM);

    OSCR::UI::printValue(OSCR::Strings::Common::Banks, numBanks);

    OSCR::UI::printSize(OSCR::Strings::Common::RAM, sramSize * 1024);

    OSCR::UI::waitButton();
  }

  // Read header information
  bool checkcart()
  {
    // Read ROM header
    uint8_t snesHeader[80];

    for (uint8_t c = 0; c < 80; c++)
    {
      snesHeader[c] = readBank(0, 0xFFB0 + c);
    }

    // Calculate CRC32 of header
    crc32_t crc = crc32_t(OSCR::CRC32::calculateCRC(BUFFN(snesHeader)));

    // Get Checksum as string
    checksum = (readBank(0, 0xFFDF) << 8) | readBank(0, 0xFFDE);

    romType = readBank(0, 0xFFD5);

    if ((romType >> 5) != 1) // Detect invalid romType byte due to too long ROM name (22 chars)
    {
      romType = 0; // LoROM   // Krusty's Super Fun House (U) 1.0 & Contra 3 (U)
    }
    else
    {
      romType &= 1; // Must be LoROM or HiROM
    }

    // Check RomSize
    uint8_t romSizeExp = readBank(0, 65495) - 7;

    romSize = 1;

    while (romSizeExp--)
      romSize *= 2;

    numBanks = (long(romSize) * 1024 * 1024 / 8) / (32768 + (long(romType) * 32768));

    //Check SD card for alt config, pass CRC32 of snesHeader but filter out 0000 and FFFF checksums
    if ((0x0000 != checksum) && (0xFFFF != checksum))
    {
      OSCR::Cores::SNES::checkAltConf(checksum, crc);
    }

    // Get name
    uint8_t myByte = 0;
    uint8_t myLength = 0;

    for (uint16_t i = 65472; i < 65492; i++)
    {
      myByte = readBank(0, i);

      if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 15)
      {
        fileName[myLength] = char(myByte);
        myLength++;
      }
    }

    // If name consists out of all japanese characters use game code
    if (myLength == 0)
    {
      // Get rom code
      fileName[0] = 'S';
      fileName[1] = 'H';
      fileName[2] = 'V';
      fileName[3] = 'C';
      fileName[4] = '-';
      for (uint16_t i = 0; i < 4; i++)
      {
        myByte = readBank(0, 0xFFB2 + i);

        if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 4)
        {
          fileName[myLength + 5] = char(myByte);
          myLength++;
        }
      }

      if (myLength == 0)
      {
        // Rom code unknown
        useDefaultName();
      }
    }

    // Read sramSizeExp
    uint8_t sramSizeExp = readBank(0, 0xFFD8);

    // Calculate sramSize
    if (sramSizeExp != 0)
    {
      sramSizeExp = sramSizeExp + 3;

      sramSize = 1;

      while (sramSizeExp--)
        sramSize *= 2;
    }
    else
    {
      sramSize = 0;
    }

    // ROM Version
    romVersion = readBank(0, 65499);

    // Test if checksum is equal to reverse checksum
    if (((uint16_t(readBank(0, 65500)) + (uint16_t(readBank(0, 65501)) * 256)) + (uint16_t(readBank(0, 65502)) + (uint16_t(readBank(0, 65503)) * 256))) == 65535)
    {
      return (checksum == 0x0000);
    }
    else // Either rom checksum is wrong or no cart is inserted
    {
      return 0;
    }
  }

  // Read rom to SD card
  void readROM()
  {
    uint16_t startByte = (romType == 0) ? 32768 : 0;
    uint16_t startBank = (romType == 0) ? 0 : 192;
    uint16_t banks = numBanks + startBank;

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SFM), FS(OSCR::Strings::FileType::SFM_NP), fileName, FS(OSCR::Strings::FileType::SNES));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    for (uint16_t currBank = startBank; currBank < banks; currBank++)
    {
      // Dump the bytes to SD 512B at a time
      for (uint32_t currByte = startByte; currByte < 65536; currByte += 512)
      {
        for (uint16_t c = 0; c < 512; c++)
        {
          OSCR::Storage::Shared::buffer[c] = readBank(currBank, currByte + c);
        }

        OSCR::Storage::Shared::writeBuffer();
      }
    }

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  /******************************************
     29F1601 flashrom functions (NP)
  *****************************************/
  // Reset the MX29F1601 flashrom, startbank is 0xC0 for first and 0xE0 for second flashrom
  void resetFlash(int startBank)
  {
    // Reset command sequence
    writeBankSequence(0xF0, startBank);
  }

  void resetFlash()
  {
    resetFlash(0xC0);
    resetFlash(0xE0);
  }

  // Print flashrom manufacturer and device ID
  void idFlash(uint8_t bank)
  {
    uint8_t const startBank = (romType) ? bank : 0;
    uint16_t const startAddress = (romType) ? 0 : 0x8000;

    writeBankSequence(0x90, startBank);

    // Read the two id bytes into a string
    flashid = readBank(startBank, startAddress) << 8;
    flashid |= readBank(startBank, startAddress + 0x02);
  }

  bool checkFlashId(uint8_t id)
  {
    idFlash(id);

    if (flashid != 0xC2F3)
    {
      cartOff();

      OSCR::UI::printLabel(OSCR::Strings::Common::ID);
      OSCR::UI::printHexLine(flashid);

      OSCR::UI::error(FS(OSCR::Strings::Errors::InvalidType));
      return false;
    }

    return true;
  }

  bool checkFlashIds()
  {
    return (checkFlashId(0xC0) && checkFlashId(0xE0));
  }

  // Write the flashroms by reading a file from the SD card, pos defines where in the file the reading/writing should start
  void writeFlashData(uint8_t startBank, uint32_t pos)
  {
    uint16_t bankSize = romType ? 0x10000 : 0x8000;
    uint16_t bankMax = startBank + numBanks;
    uint16_t startAddress = romType ? 0 : 0x8000;

    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Status::Writing));
    OSCR::UI::printHexLine(startBank);
    OSCR::UI::update();

    // Seek to a new position in the file
    if (pos != 0)
    {
      OSCR::Storage::Shared::sharedFile.seekCur(pos);
    }

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(numBanks * bankSize));

    for (uint16_t currBank = startBank; currBank < bankMax; currBank++)
    {
      // Fill SDBuffer with 1 page at a time then write it repeat until all bytes are written
      for (uint32_t currByte = startAddress; currByte < 0x10000; currByte += 128)
      {
        OSCR::Storage::Shared::readBuffer(128);

        // Write command sequence
        writeBankSequence(0xA0, startBank);

        for (uint8_t c = 0; c < 128; c++)
        {
          // Write one byte of data
          writeBank(currBank, currByte + c, OSCR::Storage::Shared::buffer[c]);
        }

        // Write the last byte twice or else it won't write at all
        writeBank(currBank, currByte + 127, OSCR::Storage::Shared::buffer[127]);

        // Wait until write is finished
        busyCheck(startBank);
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(bankSize);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();
  }

  // Delay between write operations based on status register
  void busyCheck(uint8_t startBank)
  {
    // Read register
    readBank(startBank, 0x0000);

    // Read D7 while D7 = 0
    //1 = B00000001, 1 << 7 = B10000000, PINC = B1XXXXXXX (X = don't care), & = bitwise and
    while (!(PINC & (1 << 7)))
    {
      // CE or OE must be toggled with each subsequent status read or the
      // completion of a program or erase operation will not be evident.
      // Switch RD(PH6) to HIGH
      PORTH |= (1 << 6);

      // one nop ~62.5ns
      __asm__("nop\n\t");

      // Switch RD(PH6) to LOW
      PORTH &= ~(1 << 6);

      // one nop ~62.5ns
      __asm__("nop\n\t");
    }
  }

  // Erase the flashrom to 0xFF
  void eraseFlash(int startBank)
  {
    writeBankSequence(0x80, startBank);
    writeBankSequence(0x10, startBank);

    // Wait for erase to complete
    busyCheck(startBank);
  }

  // Check if an erase succeeded, return 1 if blank and 0 if not
  uint8_t blankcheck(int startBank)
  {
    uint16_t const bankMax = numBanks + (romType ? startBank : 0);
    uint16_t const startAddress = romType ? 0 : 0x8000;

    if (!romType) startBank = 0;

    for (uint16_t currBank = startBank; currBank < bankMax; currBank++)
    {
      for (uint32_t currByte = startAddress; currByte < 0x10000; currByte++)
      {
        if (readBank(currBank, currByte) != 0xFF)
        {
          currBank = bankMax;
          return false;
        }
      }
    }

    return true;
  }

  // Check if a write succeeded, returns 0 if all is ok and number of errors if not
  uint32_t verifyFlash(int startBank, uint32_t pos)
  {
    uint16_t const bankMax = startBank + numBanks;
    uint16_t const startAddress = romType ? 0 : 0x8000;

    if (!romType) startBank = 0;

    writeErrors = 0;

    OSCR::Storage::Shared::openRO();

    // Set file starting position
    OSCR::Storage::Shared::sharedFile.seekCur(pos);

    for (uint16_t currBank = startBank; currBank < bankMax; currBank++)
    {
      for (uint32_t currByte = startAddress; currByte < 0x10000; currByte += 512)
      {
        // Fill buffer
        OSCR::Storage::Shared::fill();

        for (int c = 0; c < 512; c++)
        {
          if (readBank(currBank, currByte + c) != OSCR::Storage::Shared::buffer[c])
          {
            writeErrors++;
          }
        }
      }
    }

    OSCR::Storage::Shared::close();

    // Return 0 if verified ok, or number of errors
    return writeErrors;
  }

  // Read flashroms and save them to the SD card
  void readFlash()
  {
    // Reset to HIROM ALL
    romType = 1;

    printHeader();

    if (!unlockHirom(false)) return;

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SFM), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    resetFlash();

    OSCR::Cores::Flash::flashSize = 4194304;
    numBanks = 64;

    if (romType)
    {
      for (uint16_t currBank = 0xC0; currBank < 0xC0 + numBanks; currBank++)
      {
        for (uint32_t currByte = 0; currByte < 0x10000; currByte += 512)
        {
          for (int c = 0; c < 512; c++)
          {
            OSCR::Storage::Shared::buffer[c] = readBank(currBank, currByte + c);
          }

          OSCR::Storage::Shared::writeBuffer();
        }
      }
    }
    else
    {
      for (uint16_t currBank = 0; currBank < numBanks; currBank++)
      {
        for (uint32_t currByte = 0x8000; currByte < 0x10000; currByte += 512)
        {
          for (int c = 0; c < 512; c++)
          {
            OSCR::Storage::Shared::buffer[c] = readBank(currBank, currByte + c);
          }

          OSCR::Storage::Shared::writeBuffer();
        }
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();
  }

  // Read the current mapping from the hidden "page buffer" and print it
  void printMapping()
  {
    readyBank(0xC0);

    // Read the mapping out of the first chip
    for (uint16_t currByte = 0xFF00; currByte < 0xFF50; currByte += 10)
    {
      for (int c = 0; c < 10; c++)
      {
        // Now print the significant bits
        OSCR::UI::printHex<false>(readBank(0xC0, currByte + c));
      }

      OSCR::UI::printLine();
    }

    OSCR::UI::update();

    resetFlash();
  }

  // Read the current mapping from the hidden "page buffer"
  void readMapping()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SFM), FS(OSCR::Strings::FileType::SFM_NP), "NP", FS(OSCR::Strings::FileType::Map));

    readyBank(0xC0);

    // Get name, add extension and convert to char array for sd lib

    // Read the mapping info out of the 1st chip
    for (uint32_t currByte = 0xFF00; currByte <= 0xFFFF; currByte++)
    {
      OSCR::Storage::Shared::sharedFile.write(readBank(0xC0, currByte));
    }

    readyBank(0xE0);

    // Read the mapping info out of the 1st chip
    for (uint32_t currByte = 0xFF00; currByte <= 0xFFFF; currByte++)
    {
      OSCR::Storage::Shared::sharedFile.write(readBank(0xE0, currByte));
    }

    // Close the file:
    OSCR::Storage::Shared::close();

    resetFlash();
  }

  void eraseMapping(uint8_t startBank)
  {
    if (!unlockHirom()) return;

    // Get ID
    if (!checkFlashId(startBank)) return;

    resetFlash(startBank);

    // Prepare to erase/write Page Buffer
    writeBankSequence(0x77, startBank);

    // Erase Page Buffer
    writeBankSequence(0xE0, startBank);

    // Wait until complete
    busyCheck(startBank);
  }

  // Check if the current mapping is all 0xFF
  uint8_t blankcheckMapping()
  {
    uint8_t blank = 1;

    readyBank(0xC0);

    // Read the mapping info out of the 1st chip
    for (uint32_t currByte = 0xFF00; currByte <= 0xFFFF; currByte++)
    {
      if (readBank(0xC0, currByte) != 0xFF)
      {
        blank = 0;
      }
    }

    readyBank(0xE0);

    // Read the mapping info out of the 1st chip
    for (uint32_t currByte = 0xFF00; currByte <= 0xFFFF; currByte++)
    {
      if (readBank(0xE0, currByte) != 0xFF)
      {
        blank = 0;
      }
    }

    resetFlash();

    return blank;
  }

  void writeMapping(uint8_t startBank, uint32_t pos)
  {
    if (!unlockHirom()) return;

    // Get ID
    if (!checkFlashId(startBank)) return;

    resetFlash(startBank);

    // Seek to a new position in the file
    if (pos != 0)
    {
      OSCR::Storage::Shared::sharedFile.seekCur(pos);
    }

    // Write to Page Buffer
    for (uint32_t currByte = 0xFF00; currByte < 0xFFFF; currByte += 128)
    {
      // Prepare to erase/write Page Buffer
      writeBankSequence(0x77, startBank);

      // Write Page Buffer Command
      writeBankSequence(0x99, startBank);

      OSCR::Storage::Shared::readBuffer(128);

      for (uint8_t c = 0; c < 128; c++)
      {
        writeBank(startBank, currByte + c, OSCR::Storage::Shared::buffer[c]);
      }

      // Write last byte again
      writeBank(startBank, currByte + 127, OSCR::Storage::Shared::buffer[127]);

      busyCheck(startBank);
    }

    // Close the file:
    OSCR::Storage::Shared::close();
    OSCR::UI::printLine();
  }

  /******************************************
    SF Memory functions
  *****************************************/
  // Switch to HiRom All and unlock Write Protection
  bool unlockHirom(bool unlockWrite)
  {
    cartOn();

    romType = 1;

    if (send(0x04) != 0x2A)
    {
      cartOff();
      OSCR::UI::error(FS(OSCR::Strings::Errors::UnlockFailed));
      return false;
    }

    if (!unlockWrite) return true;

    // Unlock Write Protection
    send(0x02);

    if (readBank(0, 0x2401) == 0x4)
    {
      cartOff();
      OSCR::UI::error(FS(OSCR::Strings::Errors::UnlockFailed));
      return false;
    }

    return true;
  }

  // Send a command to the MX15001 chip
  uint8_t send(uint8_t command)
  {
    // Write command
    writeBank(0, 0x2400, 0x09);

    // Read status
    ready = readBank(0, 0x2400);

    writeBank(0, 0x2401, 0x28);
    writeBank(0, 0x2401, 0x84);

    // NP_CMD_06h, send this only if above read has returned 7Dh, not if it's already returning 2Ah
    if (ready == 0x7D)
    {
      writeBank(0, 0x2400, 0x06);
      writeBank(0, 0x2400, 0x39);
    }

    // Write the command
    writeBank(0, 0x2400, command);

    // Read status
    ready = readBank(0, 0x2400);
    return ready;
  }

  void writeFlash()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();

    OSCR::Cores::Flash::flashSize = 2097152;

    numBanks = 32;

    // Program 1st flashrom
    write(0xC0, 0);

    // Program 2nd flashrom
    write(0xE0, 2097152);

    cartOff();
  }

  // This function will erase and program the NP cart from a 4MB file off the SD card
  void write(uint8_t startBank, uint32_t pos)
  {
    // Switch NP cart's mapping
    if (!unlockHirom()) return;

    // Get ID
    if (!checkFlashId(startBank)) return;

    resetFlash(startBank);

    delay(1000);

    // Erase flash
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    if (!blankcheck(startBank))
    {
      OSCR::UI::clearLine();
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

      eraseFlash(startBank);
      resetFlash(startBank);

      OSCR::UI::clearLine();
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

      if (!blankcheck(startBank))
      {
        OSCR::UI::error(FS(OSCR::Strings::Common::FAIL));
        return;
      }
    }

    OSCR::UI::clearLine();
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    // Write flash
    writeFlashData(startBank, pos);

    // Reset flash
    resetFlash(startBank);

    // Checking for errors
    OSCR::UI::clearLine();
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    writeErrors = verifyFlash(startBank, pos);

    if (writeErrors == 0)
    {
      OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }
  }
} /* namespace OSCR::Cores::SFM */

#endif /* HAS_SFM */
