//******************************************
// SEGA MEGA DRIVE MODULE
//******************************************
// Writes to Sega CD Backup RAM Cart require an extra wire from MRES (B02) to VRES (B27)
#include "config.h"

#if HAS_MD
# include "cores/include.h"
# include "cores/MegaDrive.h"
# include "cores/Flash.h"

namespace OSCR::Cores::MegaDrive
{
  /******************************************
     Variables
  *****************************************/
  uint32_t sramEnd;
  uint16_t eepSize;
  uint16_t addrhi;
  uint16_t addrlo;
  uint16_t chksum;
  bool is32x = 0;
  bool isSVP = 0;

  using OSCR::Cores::Flash::flashid;
  using OSCR::Cores::Flash::flashSize;
  using OSCR::Cores::Flash::blank;

  //***********************************************
  // EEPROM SAVE TYPES
  // 1 = Acclaim Type 1    [24C02]
  // 2 = Acclaim Type 2    [24C02/24C16/24C65]
  // 3 = Capcom/SEGA       [24C01]
  // 4 = EA                [24C01]
  // 5 = Codemasters       [24C08/24C16/24C65]
  //***********************************************
  uint8_t eepType;

  //*********************************************************
  // SERIAL EEPROM LOOKUP TABLE
  // Format = {chksum, eepType | eepSize}
  // chksum is located in ROM at 0x18E (0xC7)
  // eepType and eepSize are combined to conserve memory
  //*********************************************************
  constexpr uint8_t const MDSize[] PROGMEM = { 1, 2, 4, 8, 12, 16, 20, 24, 32, 40 };

  constexpr uint16_t const PROGMEM eepid[] = {
    // ACCLAIM TYPE 1
    0x5B9F, 0x101,  // NBA Jam (J)
    0x694F, 0x101,  // NBA Jam (UE) (Rev 0)
    0xBFA9, 0x101,  // NBA Jam (UE) (Rev 1)
    // ACCLAIM TYPE 2
    0x16B2, 0x102,   // Blockbuster World Videogame Championship II (U)   [NO HEADER SAVE DATA]
    0xCC3F, 0x102,   // NBA Jam Tournament Edition (W) (Rev 0)            [NO HEADER SAVE DATA]
    0x8AE1, 0x102,   // NBA Jam Tournament Edition (W) (Rev 1)            [NO HEADER SAVE DATA]
    0xDB97, 0x102,   // NBA Jam Tournament Edition 32X (W)
    0x7651, 0x102,   // NFL Quarterback Club (W)
    0xDFE4, 0x102,   // NFL Quarterback Club 32X (W)
    0x3DE6, 0x802,   // NFL Quarterback Club '96 (UE)
    0xCB78, 0x2002,  // Frank Thomas Big Hurt Baseball (UE)
    0x6DD9, 0x2002,  // College Slam (U)
    // CAPCOM
    0xAD23, 0x83,  // Mega Man:  The Wily Wars (E)
    0xEA80, 0x83,  // Rockman Megaworld (J)
    // SEGA
    0x760F, 0x83,  // Evander "Real Deal" Holyfield Boxing (JU)
    0x95E7, 0x83,  // Greatest Heavyweights of the Ring (E)
    0x0000, 0x83,  // Greatest Heavyweights of the Ring (J)             [BLANK CHECKSUM 0000]
    0x7270, 0x83,  // Greatest Heavyweights of the Ring (U)
    0xBACC, 0x83,  // Honoo no Toukyuuji Dodge Danpei (J)
    0xB939, 0x83,  // MLBPA Sports Talk Baseball (U)                    [BAD HEADER SAVE DATA]
    0x487C, 0x83,  // Ninja Burai Densetsu (J)
    0x740D, 0x83,  // Wonder Boy in Monster World (B)
    0x0278, 0x83,  // Wonder Boy in Monster World (J)
    0x9D79, 0x83,  // Wonder Boy in Monster World (UE)
    // EA
    0x8512, 0x84,  // Bill Walsh College Football (UE)                  [BAD HEADER SAVE DATA]
    0xA107, 0x84,  // John Madden Football '93 (UE)                     [NO HEADER SAVE DATA]
    0x246A, 0x84,  // John Madden Football '93 (U) (EA Sports)          [NO HEADER SAVE DATA]
    0x5807, 0x84,  // John Madden Football '93 Championship Edition (U) [NO HEADER SAVE DATA]
    0x2799, 0x84,  // NHLPA Hockey '93 (UE) (Rev 0)                     [NO HEADER SAVE DATA]
    0xFA57, 0x84,  // NHLPA Hockey '93 (UE) (Rev 1)                     [NO HEADER SAVE DATA]
    0x8B9F, 0x84,  // Rings of Power (UE)                               [NO HEADER SAVE DATA]
    // CODEMASTERS
    0x7E65, 0x405,   // Brian Lara Cricket (E)                            [NO HEADER SAVE DATA]
    0x9A5C, 0x2005,  // Brian Lara Cricket 96 (E) (Rev 1.0)               [NO HEADER SAVE DATA]
    0xC4EE, 0x2005,  // Brian Lara Cricket 96 (E) (Rev 1.1)               [NO HEADER SAVE DATA]
    0x7E50, 0x805,   // Micro Machines 2 (E) (J-Cart)                     [NO HEADER SAVE DATA]
    0x165E, 0x805,   // Micro Machines '96 (E) (J-Cart) (Rev 1.0/1.1)     [NO HEADER SAVE DATA]
    0x168B, 0x405,   // Micro Machines Military (E) (J-Cart)              [NO HEADER SAVE DATA]
    0x12C1, 0x2005,  // Shane Warne Cricket (E)                           [NO HEADER SAVE DATA]
  };

  uint8_t eepcount = (sizeof(eepid) / sizeof(eepid[0])) / 2;
  uint16_t eepdata;

  // CD BACKUP RAM
  uint32_t bramSize = 0;

  // REALTEC MAPPER
  bool realtec = 0;

# if defined(ENABLE_CONFIG)
  uint8_t segaSram16bit = 0;
# else /* !ENABLE_CONFIG */
#   ifndef OPTION_MD_DEFAULT_SAVE_TYPE
#     define OPTION_MD_DEFAULT_SAVE_TYPE 0
#   endif /* !OPTION_MD_DEFAULT_SAVE_TYPE */
  uint8_t segaSram16bit = OPTION_MD_DEFAULT_SAVE_TYPE;
# endif /* ENABLE_CONFIG */

  //*****************************************
  // SONIC & KNUCKLES LOCK-ON MODE VARIABLES
  // SnKmode :
  // 0 = Not Sonic & Knuckles
  // 1 = Only Sonic & Knucles
  // 2 = Sonic & Knucles + Sonic1
  // 3 = Sonic & Knucles + Sonic2
  // 4 = Sonic & Knucles + Sonic3
  // 5 = Sonic & Knucles + Other game
  //*****************************************
  static byte SnKmode = 0;
  static uint32_t cartSizeLockon;
  static uint32_t cartSizeSonic2 = 262144;
  static uint16_t chksumLockon;
  static uint16_t chksumSonic2 = 0x0635;

  /******************************************
     Configuration
  *****************************************/
  void pulse_clock(int n)
  {
    for (int i = 0; i < n; i++)
      PORTH ^= (1 << 1);
  }

  /******************************************
     Menu
  *****************************************/

  enum class MDMenuOption : uint8_t
  {
    Cart,
    SegaCDRAM,
# if HAS_FLASH
    FlashCart,
# endif
    Back,
  };

  // MD menu items
  constexpr char const PROGMEM MDMenuItem1[] = "Game Cartridge";
  constexpr char const PROGMEM MDMenuItem2[] = "Sega CD RAM";
  constexpr char const PROGMEM MDMenuItem3[] = "Flash Repro";

  constexpr char const * const PROGMEM menuOptionsMD[] = {
    MDMenuItem1,
    MDMenuItem2,
# if HAS_FLASH
    MDMenuItem3,
# endif
    OSCR::Strings::MenuOptions::Back,
  };

  enum class MDCartMenuOption : uint8_t
  {
    ReadROM,
    ReadSave,
    WriteSave,
    ForceSize,
    RefreshCart,
    Back,
  };

  // Cart menu items
  constexpr char const PROGMEM MDCartMenuItem4[] = "Force ROM size";

  constexpr char const * const PROGMEM menuOptionsMDCart[] = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    MDCartMenuItem4,
    OSCR::Strings::MenuOptions::RefreshCart,
    OSCR::Strings::MenuOptions::Back,
  };

  enum class MDRAMMenuOption : uint8_t
  {
    ReadRAM,
    WriteRAM,
    Back,
  };

  // Sega CD RAM Backup Cartridge menu
  constexpr char const * const PROGMEM menuOptionsSCD[] = {
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    OSCR::Strings::MenuOptions::Back,
  };

  // Sega start menu
  void menu()
  {
    getCartInfo();

    do
    {
      switch (static_cast<MDMenuOption>(OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetCartType), menuOptionsMD, sizeofarray(menuOptionsMD))))
      {
      case MDMenuOption::Cart:
        cartMenu();
        continue;

      case MDMenuOption::SegaCDRAM:
        segaCDMenu();
        continue;

# if HAS_FLASH
      case MDMenuOption::FlashCart:
        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

        cartOn();

        // Setting CS(PH3) LOW
        PORTH &= ~(1 << 3);

        // ID flash
        resetFlash();
        idFlash();
        resetFlash();

        if (flashid == 0xC2F1)
        {
          OSCR::UI::printLine(F("MX29F1610 detected"));
          flashSize = 2097152;
        }
        else if (flashid == 0x017E)
        {
          OSCR::UI::printLine(F("S29GL064N detected"));
          flashSize = 4194304;
        }
        else
        {
          OSCR::UI::print(FS(OSCR::Strings::Labels::ID));
          OSCR::UI::printHex(flashid);
          OSCR::UI::fatalError(FS(OSCR::Strings::Errors::UnknownType));
        }

        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        eraseFlash();
        resetFlash();
        blankCheck();

        if (flashid == 0xC2F1)
          write29F1610();
        else if (flashid == 0x017E)
          write29GL();

        resetFlash();

        delay(1000);

        resetFlash();

        delay(1000);

        OSCR::UI::printLine(FS(OSCR::Strings::Status::Verifying));

        verifyFlash();

        // Set CS(PH3) HIGH
        PORTH |= (1 << 3);

        cartOff();
        break;
# endif

      case MDMenuOption::Back:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void cartMenu()
  {
    do
    {
      switch (static_cast<MDCartMenuOption>(OSCR::UI::menu(FS(OSCR::Strings::Cores::MegaDrive), menuOptionsMDCart, sizeofarray(menuOptionsMDCart))))
      {
      case MDCartMenuOption::ReadROM:
        // common ROM read fail state: no cart inserted - tends to report impossibly large cartSize
        // largest known game so far is supposedly "Paprium" at 10MB, so cap sanity check at 16MB
        if (cartSize == 0 || cartSize > 16777216)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        if (realtec)
        {
          readRealtec();
        }
        else
        {
          readROM();

          crc32_t crc = OSCR::Storage::Shared::getCRC32();

          if (OSCR::Databases::Basic::compareCRC(FS((is32x) ? OSCR::Strings::FileType::MegaDrive32X : OSCR::Strings::FileType::MegaDrive), crc))
          {
            OSCR::Storage::Shared::rename_P(OSCR::Databases::Basic::crdb->record()->data()->name, FS(OSCR::Strings::FileType::MegaDrive));
          }
        }
        break;

      case MDCartMenuOption::ReadSave:
        // Does cartridge have SRAM
        if ((saveType == 1) || (saveType == 2) || (saveType == 3))
        {
          OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

          cartOn();
          enableSram(1);
          readSram();
          enableSram(0);
          cartOff();
        }
        else if (saveType == 4)
        {
          readEEP();
        }
        else
        {
          OSCR::UI::error(FS(OSCR::Strings::Errors::NoSave));
          continue;
        }
        break;

      case MDCartMenuOption::WriteSave:
        // Does cartridge have SRAM
        if ((saveType == 1) || (saveType == 2) || (saveType == 3))
        {
          OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

          cartOn();
          enableSram(1);
          writeSram();
          enableSram(0);
          cartOff();

          if (writeErrors == 0)
          {
            break;
          }
          else
          {
            OSCR::Lang::printErrorVerifyBytes(writeErrors);
          }
        }
        else if (saveType == 4)
        {
          // Launch file browser
          OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

          writeEEP();

          break;
        }
        else
        {
          OSCR::UI::error(FS(OSCR::Strings::Errors::NoSave));
        }
        continue;

      case MDCartMenuOption::ForceSize:
        force_cartSize();
        continue;

      case MDCartMenuOption::RefreshCart:
        cartOn();

        // For multi-game carts
        // Set reset pin to output (PH0)
        DDRH |= (1 << 0);
        // Switch RST(PH0) to LOW
        PORTH &= ~(1 << 0);

        delay(3000);  // wait 3 secs to switch to next game

        cartOff();

        getCartInfo();

        continue;

      case MDCartMenuOption::Back:
        return;
      }

      OSCR::UI::waitButton();
    }
    while(true);
  }

  void segaCDMenu()
  {
    do
    {
      switch (static_cast<MDRAMMenuOption>(OSCR::UI::menu(FS(MDMenuItem2), menuOptionsSCD, sizeofarray(menuOptionsSCD))))
      {
      case MDRAMMenuOption::ReadRAM:
        if (bramSize < 1)
        {
          OSCR::UI::error(FS(OSCR::Strings::Errors::InvalidType));
          break;
        }

        readBram();

        break;

      case MDRAMMenuOption::WriteRAM:
        if (bramSize < 1)
        {
          OSCR::UI::error(FS(OSCR::Strings::Errors::InvalidType));
          break;
        }

        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

        writeBram();

        break;

      case MDRAMMenuOption::Back:
        return;
      }

      OSCR::UI::waitButton();
    }
    while(true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::MegaDrive));
  }

  /******************************************
     Setup
  *****************************************/
  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

# if defined(ENABLE_CONFIG)
    OSCR::Configuration::getInteger(F("md.saveType"), segaSram16bit);
# endif /*ENABLE_CONFIG*/

    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output RST(PH0) CLK(PH1) CS(PH3) WRH(PH4) WRL(PH5) OE(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0), AS(PJ1) to Output
    DDRJ |= (1 << 0) | (1 << 1);

    //set ASEL(PG5) to Output
    DDRG |= (1 << 5);

    // Set Data Pins (D0-D15) to Input
    DDRC = 0x00;
    DDRA = 0x00;

    // Setting RST(PH0) CS(PH3) WRH(PH4) WRL(PH5) OE(PH6) HIGH
    PORTH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Setting TIME(PJ0) AS(PJ1) HIGH
    PORTJ |= (1 << 0) | (1 << 1);

    // setting ASEL(PG5) HIGH
    PORTG |= (1 << 5);

    delay(200);
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  /******************************************
     I/O Functions
  *****************************************/

  /******************************************
    Low level functions
  *****************************************/
  void writeWord(uint32_t myAddress, uint16_t myData)
  {
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
    PORTC = myData;
    PORTA = (myData >> 8) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    __asm__("nop\n\t"
            "nop\n\t");

    // Switch WR(PH5) to LOW
    PORTH &= ~(1 << 5);
    // Setting CS(PH3) LOW
    PORTH &= ~(1 << 3);

    // Leave WR low for at least 200ns
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
            "nop\n\t");

    // Setting CS(PH3) HIGH
    PORTH |= (1 << 3);
    // Switch WR(PH5) to HIGH
    PORTH |= (1 << 5);

    // Leave WR high for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  }

  uint16_t readWord(uint32_t myAddress)
  {
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    NOP;

    // Setting CS(PH3) LOW
    PORTH &= ~(1 << 3);
    // Setting OE(PH6) LOW
    PORTH &= ~(1 << 6);
    // Setting AS(PJ1) LOW
    PORTJ &= ~(1 << 1);
    // Setting ASEL(PG5) LOW
    PORTG &= ~(1 << 5);
    // Pulse CLK(PH1), needed for SVP enhanced carts
    pulse_clock(10);

    // most MD ROMs are 200ns, comparable to SNES > use similar access delay of 6 x 62.5 = 375ns
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // Read
    uint16_t tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

    // Setting CS(PH3) HIGH
    PORTH |= (1 << 3);
    // Setting OE(PH6) HIGH
    PORTH |= (1 << 6);
    // Setting AS(PJ1) HIGH
    PORTJ |= (1 << 1);
    // Setting ASEL(PG5) HIGH
    PORTG |= (1 << 5);
    // Pulse CLK(PH1), needed for SVP enhanced carts
    pulse_clock(10);

    // these 6x nop delays have been here before, unknown what they mean
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    return tempWord;
  }

  void writeFlash(uint32_t myAddress, uint16_t myData)
  {
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
    PORTC = myData;
    PORTA = (myData >> 8) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    __asm__("nop\n\t");

    // Switch WE(PH5) to LOW
    PORTH &= ~(1 << 5);

    // Leave WE low for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Switch WE(PH5)to HIGH
    PORTH |= (1 << 5);

    // Leave WE high for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  }

  uint16_t readFlash(uint32_t myAddress)
  {
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    __asm__("nop\n\t");

    // Setting OE(PH6) LOW
    PORTH &= ~(1 << 6);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Read
    uint16_t tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

    __asm__("nop\n\t");

    // Setting OE(PH6) HIGH
    PORTH |= (1 << 6);
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    return tempWord;
  }

  // Switch data pins to write
  void dataOut()
  {
    DDRC = 0xFF;
    DDRA = 0xFF;

    dataDir = DataDirection::Out;
  }

  // Switch data pins to read
  void dataIn()
  {
    DDRC = 0x00;
    DDRA = 0x00;

    // Enable Internal Pullups (needed for games like Batman Forever that are open bus with random bytes on the last 1MB, so we get a clean 0xFF padding)
    PORTC = 0xFF;
    PORTA = 0xFF;

    dataDir = DataDirection::In;
  }

  /******************************************
    MEGA DRIVE functions
  *****************************************/
  uint8_t copyToRomName(char* output, uint8_t const * input, uint8_t length)
  {
    uint8_t myLength = 0;

    for (uint8_t i = 0; i < 48; i++) {
      if (((input[i] >= '0' && input[i] <= '9') || (input[i] >= 'A' && input[i] <= 'z')) && myLength < length) {
        output[myLength++] = input[i];
      }
    }

    return myLength;
  }

  void getCartInfo()
  {
    cartOn();
    dataIn();

    // Get cart size
    cartSize = ((long(readWord(0xD2)) << 16) | readWord(0xD3)) + 1;

    // Check for 32x compatibility
    if ((readWord(0x104 / 2) == 0x2033) && (readWord(0x106 / 2) == 0x3258))
      is32x = 1;
    else
      is32x = 0;

    // Get cart checksum
    chksum = readWord(0xC7);

    // Get cart ID
    char id[15];

    for (uint8_t c = 0; c < 14; c += 2)
    {
      // split word
      uint16_t myWord = readWord((0x180 + c) / 2);

      // write to buffer
      id[c] = myWord >> 8;
      id[c + 1] = myWord & 0xFF;
    }

    // Get cart name
    for (uint8_t c = 0; c < 48; c += 2)
    {
      // split word
      uint16_t myWord = readWord((0x150 + c) / 2);

      // write to buffer
      OSCR::Storage::Shared::buffer[c] = myWord >> 8;
      OSCR::Storage::Shared::buffer[c + 1] = myWord & 0xFF;
    }

    fileName[copyToRomName(fileName, OSCR::Storage::Shared::buffer, kFileNameMax - 1)] = 0;

    // Identify games using SVP chip
    if (!strncmp_P(id, PSTR("GM MK-1229 "), 11) || !strncmp_P(id, PSTR("GM G-7001  "), 11))  // Virtua Racing (E/U/J)
      isSVP = 1;
    else
      isSVP = 0;

    // Fix cartridge sizes and checksums according to no-intro database
    if (cartSize == 0x400000)
    {
      switch (chksum)
      {
      case 0xCE25:  // Super Street Fighter 2 (J) 40Mbit
      case 0xE41D:  // Super Street Fighter 2 (E) 40Mbit
      case 0xE017:  // Super Street Fighter 2 (U) 40Mbit
        cartSize = 0x500000;
        break;

      case 0x0000:  // Demons of Asteborg v1.0 (W) 120Mbit
        cartSize = 0xEAF2F4;
        break;

      case 0xBCBF:  // Demons of Asteborg v1.1 (W) 120Mbit
      case 0x6E1E:  // Demons of Asteborg v1.11 (W) 120Mbit
        cartSize = 0xEA0000;
        break;
      }
    }
    if (cartSize == 0x300000)
    {
      switch (chksum)
      {
      case 0xBC5F:  // Batman Forever (World)
      case 0x3CDD:  // Donald in Maui Mallard (Brazil) (En)
      case 0x44AD:  // Donald in Maui Mallard (Europe) (Rev A)
      case 0x2D9A:  // Foreman for Real (World)
      case 0x5648:  // Justice League Task Force (World)
      case 0x0A29:  // Mega 6 Vol. 3 (Europe)
      case 0x7651:  // NFL Quarterback Club (World)
      case 0x74CA:  // WWF RAW (World)
        cartSize = 0x400000;
        break;
      }
    }
    if (cartSize == 0x200000)
    {
      switch (chksum)
      {
      case 0x2078:  // Dynamite Headdy (USA, Europe)
        chksum = 0x9877;
        break;

      case 0xAE95:  // Winter Olympic Games (USA)
        chksum = 0x56A0;
        break;
      }
    }

    if (cartSize == 0x180000)
    {
      switch (chksum)
      {
      case 0xFFE2:  // Cannon Fodder (Europe)
      case 0xF418:  // Chaos Engine, The (Europe)
      case 0xF71D:  // Fatal Fury (Europe, Korea) (En)
      case 0xA884:  // Flashback (Europe) (En,Fr)
      case 0x7D68:  // Flashback - The Quest for Identity (USA) (En,Fr)
      case 0x030D:  // Shining Force (Europe)
      case 0xE975:  // Shining Force (USA)
        cartSize = 0x200000;
        break;
      }
    }
    if (cartSize == 0x100000)
    {
      switch (chksum)
      {
      case 0xCDF5:  // Life on Mars (Aftermarket)
        cartSize = 0x400000;
        chksum = 0x603A;
        break;

      case 0xF85F:  // Metal Dragon (Aftermarket)
        cartSize = 0x200000;
        chksum = 0x6965;
        break;
      }
    }
    if (cartSize == 0xC0000)
    {
      switch (chksum)
      {
        case 0x9D79:  // Wonder Boy in Monster World (USA, Europe)
          cartSize = 0x100000;
          break;
      }
    }
    if (cartSize == 0x80000)
    {
      switch (chksum)
      {
      case 0x06C1:  // Madden NFL 98 (USA)
        cartSize = 0x200000;
        chksum = 0x8473;
        break;
      case 0x5B3A:  // NHL 98 (USA)
        cartSize = 0x200000;
        chksum = 0x5613;
        break;
      case 0xD07D:  // Zero Wing (Japan)
        cartSize = 0x100000;
        chksum = 0xF204;
        break;
      case 0x95C9:  // Zero Wing (Europe)
      case 0x9144:  // Zoop (Europe)
      case 0xB8D4:  // Zoop (USA)
        cartSize = 0x100000;
        break;
      case 0xC422:  // Jeopardy! (USA)
        chksum = 0xC751;
        break;
      case 0x0C6A:  // Monopoly (USA)
        chksum = 0xE1AA;
        break;
      case 0xA760:  // Gain Ground (USA)
        chksum = 0x97CD;
        break;
      case 0x1404:  // Wonder Boy III - Monster Lair (Japan, Europe) (En)
        chksum = 0x53B9;
        break;
      }
    }
    if (cartSize == 0x40000)
    {
      switch (chksum)
      {
        case 0x8BC6:  // Pac-Attack (USA)
        case 0xB344:  // Pac-Panic (Europe)
          cartSize = 0x100000;
          break;
      }
    }
    if (cartSize == 0x20000)
    {
      switch (chksum)
      {
        case 0x7E50:  // Micro Machines 2 - Turbo Tournament (Europe)
          cartSize = 0x100000;
          chksum = 0xD074;
          break;
        case 0x168B:  // Micro Machines - Military (Europe)
          cartSize = 0x100000;
          chksum = 0xCEE0;
          break;
      }
    }

    // Fatman (Japan)
    if (!strncmp_P(id, PSTR("GM T-44013 "), 11) && (chksum == 0xFFFF))
    {
      chksum = 0xC560;
      cartSize = 0xA0000;
    }

    // Slaughter Sport (USA)
    if (!strncmp_P(fileName, PSTR("GMT5604600jJ"), 12) && (chksum == 0xFFFF))
    {
      strncpy_P(fileName, PSTR("SLAUGHTERSPORT"), kFileNameMax);
      chksum = 0x6BAE;
    }

    // Fixes aftermarket cartridges
    // Beggar Prince (Rev 1)
    if (!strncmp_P(id, PSTR("SF-001"), 6) && (chksum == 0x3E08))
    {
      cartSize = 0x400000;
    }
    // Legend of Wukong
    if (!strncmp_P(id, PSTR("SF-002"), 6) && (chksum == 0x12B0))
    {
      chksum = 0x45C6;
    }
    // YM2612 Instrument Editor
    if (!strncmp_P(id, PSTR("GM 10101010"), 11) && (chksum == 0xC439))
    {
      chksum = 0x21B0;
      cartSize = 0x100000;
    }
    // Technoptimistic
    if (!strncmp_P(id, PSTR("MU REMUTE01"), 11) && (chksum == 0x0000))
    {
      chksum = 0xB55C;
      cartSize = 0x400000;
    }
    // Decoder
    if (!strncmp_P(id, PSTR("GM REMUTE02"), 11) && (chksum == 0x0000))
    {
      chksum = 0x5426;
      cartSize = 0x400000;
    }
    // Handy Harvy
    if (!strncmp_P(id, PSTR("GM HHARVYSG"), 11) && (chksum == 0x0000))
    {
      chksum = 0xD9D2;
      cartSize = 0x100000;
    }
    // Jim Power - The Lost Dimension in 3D
    if (!strncmp_P(id, PSTR("GM T-107036"), 11) && (chksum == 0x0000))
    {
      chksum = 0xAA28;
    }
    // mikeyeldey95
    if (!strcmp_P(id, PSTR("GM 00000000-43")) && (chksum == 0x0000))
    {
      chksum = 0x921B;
      cartSize = 0x400000;
    }
    // Enryuu Seiken Xiao-Mei
    if (!strcmp_P(id, PSTR("GM 00000000-00")) && (chksum == 0x1E0C))
    {
      chksum = 0xE7E5;
      cartSize = 0x400000;
    }
    // Life on Earth - Reimagined
    if (!strcmp_P(id, PSTR("GM 00000000-00")) && (chksum == 0x6BD5))
    {
      chksum = 0x1FEA;
      cartSize = 0x400000;
    }
    // Sasha Darko's Sacred Line I
    if (!strcmp_P(id, PSTR("GM 00000005-00")) && (chksum == 0x9F34))
    {
      chksum = 0xA094;
      cartSize = 0x400000;
    }
    // Sasha Darko's Sacred Line II
    if (!strcmp_P(id, PSTR("GM 00000005-00")) && (chksum == 0x0E9B))
    {
      chksum = 0x6B4B;
      cartSize = 0x400000;
    }
    // Sasha Darko's Sacred Line (Watermelon Release)
    if (!strcmp_P(id, PSTR("GM T-574323-00")) && (chksum == 0xAEDD))
    {
      cartSize = 0x400000;
    }
    // Kromasphere
    if (!strcmp_P(id, PSTR("GM MK-0000 -00")) && (chksum == 0xC536))
    {
      chksum = 0xFAB1;
      cartSize = 0x200000;
    }
    // YM2017
    if (!strcmp_P(id, PSTR("GM CSET0001-02")) && (chksum == 0x0000))
    {
      chksum = 0xE3A9;
    }
    // The Curse of Illmore Bay
    if (!strcmp_P(id, PSTR("1774          ")) && (chksum == 0x0000))
    {
      chksum = 0x6E34;
      cartSize = 0x400000;
    }
    // Coffee Crisis
    if (!strcmp_P(id, PSTR("JN-20160131-03")) && (chksum == 0x0000))
    {
      chksum = 0x8040;
      cartSize = 0x400000;
    }
    // Romeow & Julicat
    if (!strncmp_P(fileName, PSTR("ROMEOWJULICAT"), 13) && (chksum == 0x0000))
    {
      chksum = 0xB094;
      cartSize = 0x200000;
    }

    // Sonic & Knuckles checks
    SnKmode = 0;
    if (!strcmp_P(id, PSTR("GM MK-1563 -00")) && (chksum == 0xDFB3))
    {
      char labelLockon[17];
      memset(labelLockon, 0, 17);

      // Get labelLockon
      for (uint8_t c = 0; c < 16; c += 2)
      {
        // split word
        uint16_t myWord = readWord((0x200100 + c) / 2);
        uint8_t loByte = myWord & 0xFF;
        uint8_t hiByte = myWord >> 8;

        // write to buffer
        labelLockon[c] = hiByte;
        labelLockon[c + 1] = loByte;
      }

      // check Lock-on game presence
      if (!(strcmp_P(labelLockon, PSTR("SEGA MEGA DRIVE ")) & strcmp_P(labelLockon, PSTR("SEGA GENESIS    "))))
      {
        char idLockon[15];

        // Lock-on cart checksum
        chksumLockon = readWord(0x1000C7);

        // Lock-on cart size
        cartSizeLockon = ((long(readWord(0x1000D2)) << 16) | readWord(0x1000D3)) + 1;

        // Get IdLockon
        for (uint8_t c = 0; c < 14; c += 2)
        {
          // split word
          uint16_t myWord = readWord((0x200180 + c) / 2);
          uint8_t loByte = myWord & 0xFF;
          uint8_t hiByte = myWord >> 8;

          // write to buffer
          idLockon[c] = hiByte;
          idLockon[c + 1] = loByte;
        }

        if (!strncmp_P(idLockon, PSTR("GM 00001009-0"), 13) || !strncmp_P(idLockon, PSTR("GM 00004049-0"), 13)) // Sonic1 ID:GM 00001009-0? or GM 00004049-0?
        {
          SnKmode = 2;
        }
        else if (!strcmp_P(idLockon, PSTR("GM 00001051-00")) || !strcmp_P(idLockon, PSTR("GM 00001051-01")) || !strcmp_P(idLockon, PSTR("GM 00001051-02"))) // Sonic2 ID:GM 00001051-00 or GM 00001051-01 or GM 00001051-02
        {
          SnKmode = 3;
          // Prepare Sonic2 Banks
          writeSSF2Map(0x509878, 1);  // 0xA130F1
        }
        else if (!strcmp_P(idLockon, PSTR("GM MK-1079 -00"))) // Sonic3 ID:GM MK-1079 -00
        {
          SnKmode = 4;
        }
        else // Other game
        {
          SnKmode = 5;
        }
      }
      else
      {
        SnKmode = 1;
      }
    }

    // Serial EEPROM Check
    for (int i = 0; i < eepcount; i++)
    {
      int index = i * 2;
      uint16_t eepcheck = pgm_read_word(eepid + index);
      if (eepcheck == chksum) {
        eepdata = pgm_read_word(eepid + index + 1);
        eepType = eepdata & 0x7;
        eepSize = eepdata & 0xFFF8;
        saveType = 4;  // SERIAL EEPROM
        break;
      }
    }

    // Greatest Heavyweights of the Ring (J) has blank chksum 0x0000
    // Other non-save carts might have the same blank chksum
    // Check header for Serial EEPROM Data
    if (chksum == 0x0000) {
      if (readWord(0xD9) != 0xE840) {  // NOT SERIAL EEPROM
        eepType = 0;
        eepSize = 0;
        saveType = 0;
      }
    }

    // Codemasters EEPROM Check
    // Codemasters used the same incorrect header across multiple carts
    // Carts with checksum 0x165E or 0x168B could be EEPROM or non-EEPROM
    // Check the first DWORD in ROM (0x444E4C44) to identify the EEPROM carts
    if ((chksum == 0x165E) || (chksum == 0x168B)) {
      if (readWord(0x00) != 0x444E) {  // NOT SERIAL EEPROM
        eepType = 0;
        eepSize = 0;
        saveType = 0;
      }
    }

    // CD Backup RAM Cart Check
    // 4 = 128KB (2045 Blocks) Sega CD Backup RAM Cart
    // 6 = 512KB (8189 Blocks) Ultra CD Backup RAM Cart (Aftermarket)
    uint16_t bramCheck = readWord(0x00);
    if ((((bramCheck & 0xFF) == 0x04) && ((chksum & 0xFF) == 0x04)) || (((bramCheck & 0xFF) == 0x06) && ((chksum & 0xFF) == 0x06))) {
      uint32_t p = 1 << (bramCheck & 0xFF);
      bramSize = p * 0x2000L;
    }
    if (saveType != 4) {  // NOT SERIAL EEPROM
      // Check if cart has sram
      saveType = 0;
      sramSize = 0;

      // FIXED CODE FOR SRAM/FRAM/PARALLEL EEPROM
      // 0x5241F820 SRAM (ODD BYTES/EVEN BYTES)
      // 0x5241F840 PARALLEL EEPROM - READ AS SRAM
      // 0x5241E020 SRAM (BOTH BYTES)
      if (readWord(0xD8) == 0x5241)
      {
        uint16_t sramType = readWord(0xD9);
        if ((sramType == 0xF820) || (sramType == 0xF840)) // SRAM/FRAM ODD/EVEN BYTES
        {
          // Get sram start and end
          sramBase = ((long(readWord(0xDA)) << 16) | readWord(0xDB));
          sramEnd = ((long(readWord(0xDC)) << 16) | readWord(0xDD));

          if (sramBase == 0x20000020 && sramEnd == 0x00010020) // Fix for Psy-o-blade
          {
            sramBase = 0x200001;
            sramEnd = 0x203FFF;
          }

          // Check alignment of sram
          // 0x300001 for HARDBALL '95 (U)
          // 0x3C0001 for Legend of Wukong (Aftermarket)
          if ((sramBase == 0x200001) || (sramBase == 0x300001) || (sramBase == 0x3C0001))
          {
            // low byte
            saveType = 1;  // ODD
            sramSize = (sramEnd - sramBase + 2) / 2;

            // Right shift sram base address so [A21] is set to high 0x200000 = 0b001[0]00000000000000000000
            sramBase = sramBase >> 1;
          }
          else if (sramBase == 0x200000)
          {
            // high byte
            saveType = 2;  // EVEN

            sramSize = (sramEnd - sramBase + 1) / 2;

            // Right shift sram base address so [A21] is set to high 0x200000 = 0b001[0]00000000000000000000
            sramBase = sramBase / 2;
          }
          else
          {
            OSCR::UI::print(("sramType: "));
            OSCR::UI::printHexLine(sramType);

            OSCR::UI::print(("sramBase: "));
            OSCR::UI::printHexLine(sramBase);

            OSCR::UI::print(("sramEnd: "));
            OSCR::UI::printHexLine(sramEnd);

            OSCR::UI::fatalError(FS(OSCR::Strings::Errors::UnknownType));
          }
        }
        else if (sramType == 0xE020) // SRAM BOTH BYTES
        {
          // Get sram start and end
          sramBase = ((long(readWord(0xDA)) << 16) | readWord(0xDB));
          sramEnd = ((long(readWord(0xDC)) << 16) | readWord(0xDD));

          if (sramBase == 0x200001)
          {
            saveType = 3;  // BOTH
            sramSize = sramEnd - sramBase + 2;
            sramBase = sramBase >> 1;
          }
          else if (sramBase == 0x200000)
          {
            saveType = 3;  // BOTH
            sramSize = sramEnd - sramBase + 1;
            sramBase = sramBase >> 1;
          }
          else if (sramBase == 0x3FFC00)
          {
            // Used for some aftermarket carts without sram
            saveType = 0;
          }
          else
          {
            printHeader();
            OSCR::UI::print(("sramType: "));
            OSCR::UI::printHexLine(sramType);

            OSCR::UI::print(("sramBase: "));
            OSCR::UI::printHexLine(sramBase);

            OSCR::UI::print(("sramEnd: "));
            OSCR::UI::printHexLine(sramEnd);

            OSCR::UI::fatalError(FS(OSCR::Strings::Errors::UnknownType));
          }
        }
      }
      else
      {
        // SRAM CARTS WITH BAD/MISSING HEADER SAVE DATA
        switch (chksum)
        {
        case 0xC2DB:     // Winter Challenge (UE)
          saveType = 1;  // ODD
          sramBase = 0x200001;
          sramEnd = 0x200FFF;
          break;

        case 0xD7B6:     // Buck Rogers: Countdown to Doomsday (UE)
        case 0xFE3E:     // NBA Live '98 (U)
        case 0xFDAD:     // NFL '94 starring Joe Montana (U)
        case 0x632E:     // PGA Tour Golf (UE) (Rev 1)
        case 0xD2BA:     // PGA Tour Golf (UE) (Rev 2)
        case 0x44FE:     // Super Hydlide (J)
          saveType = 1;  // ODD
          sramBase = 0x200001;
          sramEnd = 0x203FFF;
          break;

        case 0xDB5E:     // Might & Magic: Gates to Another World (UE) (Rev 1)
        case 0x3428:     // Starflight (UE) (Rev 0)
        case 0x43EE:     // Starflight (UE) (Rev 1)
          saveType = 3;  // BOTH
          sramBase = 0x200001;
          sramEnd = 0x207FFF;
          break;

        case 0xBF72:     // College Football USA '96 (U)
        case 0x72EF:     // FIFA Soccer '97 (UE)
        case 0xD723:     // Hardball III (U)
        case 0x06C1:     // Madden NFL '98 (U)
        case 0xDB17:     // NHL '96 (UE)
        case 0x5B3A:     // NHL '98 (U)
        case 0x2CF2:     // NFL Sports Talk Football '93 starring Joe Montana (UE)
        case 0xE9B1:     // Summer Challenge (U)
        case 0xEEE8:     // Test Drive II: The Duel (U)
          saveType = 1;  // ODD
          sramBase = 0x200001;
          sramEnd = 0x20FFFF;
          break;
        }

        if (saveType == 1)
        {
          sramSize = (sramEnd - sramBase + 2) / 2;
          sramBase = sramBase >> 1;
        }
        else if (saveType == 3)
        {
          sramSize = sramEnd - sramBase + 2;
          sramBase = sramBase >> 1;
        }
      }
    }

    //Get Lock-on cart name
    if (SnKmode >= 2)
    {
      char romNameLockon[12];

      //Change fileName
      strcpy_P(fileName, PSTR("SnK_"));

      for (uint8_t c = 0; c < 48; c += 2)
      {
        // split word
        uint16_t myWord = readWord((0x200150 + c) / 2);
        uint8_t loByte = myWord & 0xFF;
        uint8_t hiByte = myWord >> 8;

        // write to buffer
        OSCR::Storage::Shared::buffer[c] = hiByte;
        OSCR::Storage::Shared::buffer[c + 1] = loByte;
      }
      romNameLockon[copyToRomName(romNameLockon, OSCR::Storage::Shared::buffer, sizeof(romNameLockon) - 1)] = 0;

      switch (SnKmode)
      {
        case 2: strcat_P(fileName, PSTR("SONIC1")); break;
        case 3: strcat_P(fileName, PSTR("SONIC2")); break;
        case 4: strcat_P(fileName, PSTR("SONIC3")); break;
        case 5: strcat(fileName, romNameLockon); break;
      }
    }

    // Realtec Mapper Check
    uint16_t realtecCheck1 = readWord(0x3F080);  // 0x7E100 "SEGA" (BootROM starts at 0x7E000)
    uint16_t realtecCheck2 = readWord(0x3F081);
    if ((realtecCheck1 == 0x5345) && (realtecCheck2 == 0x4741))
    {
      realtec = 1;
      strcpy_P(fileName, PSTR("Realtec"));
      cartSize = 0x80000;
    }

    // Some games are missing the ROM size in the header, in this case calculate ROM size by looking for mirror of the first line of the ROM
    // This does not work for cartridges that have SRAM mapped directly after the maskrom like Striker (Europe)
    if ((cartSize < 0x8000) || (cartSize > 0xEAF400))
    {
      [&]
      {
        for (cartSize = 0x10000; cartSize < 0x200000; cartSize += 0x10000)
        {
          for (uint8_t i = 0; i < 9; i++)
          {
            if (readWord(i) != readWord(cartSize + i)) break; // Exit this loop, continue outer loop
            if (i == 8) return; // exit lambda
          }
        }
      }();

      cartSize = cartSize * 2;
    }

    cartOff();

    printHeader();
    OSCR::UI::printLine(FS(OSCR::Strings::MenuOptions::CartInfo));
    OSCR::UI::printLine();
    OSCR::UI::print(FS(OSCR::Strings::Labels::NAME));
    OSCR::UI::printLine(fileName);

    if (bramCheck != 0x00FF)
    {
      OSCR::UI::print(FS(OSCR::Strings::Common::RAM));
      OSCR::UI::print(FS(OSCR::Strings::Labels::CHECKSUM));
      OSCR::UI::printHexLine(bramCheck);
    }

    if (bramSize > 0)
    {
      OSCR::UI::print(FS(OSCR::Strings::Common::RAM));
      OSCR::UI::print(FS(OSCR::Strings::Labels::SIZE));
      OSCR::Lang::printBytesLine((bramSize >> 10) * 1024);
    }

    OSCR::UI::print(FS(OSCR::Strings::Labels::SIZE));
    OSCR::Lang::printBytes(cartSize * 8);

    switch (SnKmode)
    {
    case 2:
    case 4:
    case 5:
      OSCR::UI::print(OSCR::Strings::Symbol::Plus);
      OSCR::Lang::printBytes(cartSizeLockon * 8);
      break;

    case 3:
      OSCR::UI::print(OSCR::Strings::Symbol::Plus);
      OSCR::Lang::printBytes(cartSizeLockon * 8);
      OSCR::UI::print(OSCR::Strings::Symbol::Plus);
      OSCR::Lang::printBytes(cartSizeSonic2 * 8);
      break;
    }
    OSCR::UI::printLine();

    OSCR::UI::print(FS(OSCR::Strings::Labels::CHECKSUM));
    OSCR::UI::printHex(chksum);

    switch (SnKmode)
    {
    case 2:
    case 4:
    case 5:
      OSCR::UI::print(OSCR::Strings::Symbol::Plus);
      OSCR::UI::printHex(chksumLockon);
      break;
    case 3:
      OSCR::UI::print(OSCR::Strings::Symbol::Plus);
      OSCR::UI::printHex(chksumLockon);

      OSCR::UI::print(OSCR::Strings::Symbol::Plus);
      OSCR::UI::printHex(chksumSonic2);
      break;
    }

    OSCR::UI::printLine();

    OSCR::UI::print(FS(OSCR::Strings::Labels::SAVE));
    if (saveType == 4)
    {
      OSCR::Lang::printBytesLine(eepSize * 8);
    }
    else if (sramSize > 0)
    {
      OSCR::Lang::printBytesLine(sramSize * 8);
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::None));
    }

    OSCR::UI::waitButton();
  }

  void writeSSF2Map(uint32_t myAddress, uint16_t myData)
  {
    dataOut();

    // Set TIME(PJ0) HIGH
    PORTJ |= (1 << 0);

    // 0x50987E * 2 = 0xA130FD  Bank 6 (0x300000-0x37FFFF)
    // 0x50987F * 2 = 0xA130FF  Bank 7 (0x380000-0x3FFFFF)
    PORTL = (myAddress >> 16) & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTF = myAddress & 0xFF;
    PORTC = myData;
    PORTA = (myData >> 8) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    __asm__("nop\n\t"
            "nop\n\t");

    // Strobe TIME(PJ0) LOW to latch the data
    PORTJ &= ~(1 << 0);
    // Switch WR(PH5) to LOW
    PORTH &= ~(1 << 5);

    // Leave WR low for at least 200ns
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
            "nop\n\t");

    // Switch WR(PH5) to HIGH
    PORTH |= (1 << 5);
    // Set TIME(PJ0) HIGH
    PORTJ |= (1 << 0);

    dataIn();
  }

  // Read rom and save to the SD card
  void readROM()
  {
    // Checksum
    uint16_t calcCKS = 0;
    uint16_t calcCKSLockon = 0;
    uint16_t calcCKSSonic2 = 0;

    cartOn();
    dataIn();

    printHeader();

    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::MegaDrive), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    // Phantasy Star/Beyond Oasis with 74HC74 and 74HC139 switch ROM/SRAM at address 0x200000
    if (0x200000 < cartSize && cartSize < 0x400000)
    {
      enableSram(0);
    }

    // Prepare SSF2 Banks
    if (cartSize > 0x400000)
    {
      writeSSF2Map(0x50987E, 6);  // 0xA130FD
      writeSSF2Map(0x50987F, 7);  // 0xA130FF
    }

    uint8_t offsetSSF2Bank = 0;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(cartSize + (SnKmode < 2 ? 0 : cartSizeLockon) + (SnKmode == 3 ? cartSizeSonic2 : 0));

    for (uint32_t currBuffer = 0; currBuffer < cartSize / 2; currBuffer += 512)
    {
      if (currBuffer == 0x200000)
      {
        writeSSF2Map(0x50987E, 8);  // 0xA130FD
        writeSSF2Map(0x50987F, 9);  // 0xA130FF
        offsetSSF2Bank = 1;
      }

      // Demons of Asteborg Additional Banks
      else if (currBuffer == 0x280000) {
        writeSSF2Map(0x50987E, 10);  // 0xA130FD
        writeSSF2Map(0x50987F, 11);  // 0xA130FF
        offsetSSF2Bank = 2;
      } else if (currBuffer == 0x300000) {
        writeSSF2Map(0x50987E, 12);  // 0xA130FD
        writeSSF2Map(0x50987F, 13);  // 0xA130FF
        offsetSSF2Bank = 3;
      } else if (currBuffer == 0x380000) {
        writeSSF2Map(0x50987E, 14);  // 0xA130FD
        writeSSF2Map(0x50987F, 15);  // 0xA130FF
        offsetSSF2Bank = 4;
      } else if (currBuffer == 0x400000) {
        writeSSF2Map(0x50987E, 16);  // 0xA130FD
        writeSSF2Map(0x50987F, 17);  // 0xA130FF
        offsetSSF2Bank = 5;
      } else if (currBuffer == 0x480000) {
        writeSSF2Map(0x50987E, 18);  // 0xA130FD
        writeSSF2Map(0x50987F, 19);  // 0xA130FF
        offsetSSF2Bank = 6;
      } else if (currBuffer == 0x500000) {
        writeSSF2Map(0x50987E, 20);  // 0xA130FD
        writeSSF2Map(0x50987F, 21);  // 0xA130FF
        offsetSSF2Bank = 7;
      } else if (currBuffer == 0x580000) {
        writeSSF2Map(0x50987E, 22);  // 0xA130FD
        writeSSF2Map(0x50987F, 23);  // 0xA130FF
        offsetSSF2Bank = 8;
      } else if (currBuffer == 0x600000) {
        writeSSF2Map(0x50987E, 24);  // 0xA130FD
        writeSSF2Map(0x50987F, 25);  // 0xA130FF
        offsetSSF2Bank = 9;
      } else if (currBuffer == 0x680000) {
        writeSSF2Map(0x50987E, 26);  // 0xA130FD
        writeSSF2Map(0x50987F, 27);  // 0xA130FF
        offsetSSF2Bank = 10;
      } else if (currBuffer == 0x700000) {
        writeSSF2Map(0x50987E, 28);  // 0xA130FD
        writeSSF2Map(0x50987F, 29);  // 0xA130FF
        offsetSSF2Bank = 11;
      }

      for (uint16_t currWord = 0, d = 0; currWord < 512; currWord++, d+=2)
      {
        if (d == 512)
        {
          OSCR::Storage::Shared::writeBuffer();
          OSCR::UI::ProgressBar::advance(512);
        }

        uint32_t myAddress = currBuffer + currWord - (offsetSSF2Bank * 0x80000);
        PORTF = myAddress & 0xFF;
        PORTK = (myAddress >> 8) & 0xFF;
        PORTL = (myAddress >> 16) & 0xFF;

        // Arduino running at 16Mhz -> one nop = 62.5ns
        NOP;
        // Setting CS(PH3) LOW
        PORTH &= ~(1 << 3);
        // Setting OE(PH6) LOW
        PORTH &= ~(1 << 6);
        // Setting AS(PJ1) LOW
        PORTJ &= ~(1 << 1);
        // Setting ASEL(PG5) LOW
        PORTG &= ~(1 << 5);

        // Pulse CLK(PH1)
        if (isSVP)
          pulse_clock(10);

        // most MD ROMs are 200ns, comparable to SNES > use similar access delay of 6 x 62.5 = 375ns
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;

        uint16_t buffPos = ((d < 512) ? (d) : (d - 512));

        // Read
        OSCR::Storage::Shared::buffer[buffPos] = PINA;
        OSCR::Storage::Shared::buffer[buffPos + 1] = PINC;

        // Setting CS(PH3) HIGH
        PORTH |= (1 << 3);
        // Setting OE(PH6) HIGH
        PORTH |= (1 << 6);
        // Setting AS(PJ1) HIGH
        PORTJ |= (1 << 1);
        // Setting ASEL(PG5) HIGH
        PORTG |= (1 << 5);
        // Pulse CLK(PH1)
        if (isSVP)
          pulse_clock(10);

        // Skip first 256 words
        if (((currBuffer == 0) && (currWord >= 256)) || (currBuffer > 0))
        {
          calcCKS += ((OSCR::Storage::Shared::buffer[buffPos] << 8) | OSCR::Storage::Shared::buffer[buffPos + 1]);
        }
      }

      OSCR::Storage::Shared::writeBuffer();
      OSCR::UI::ProgressBar::advance(512);
    }

    for (uint8_t snki = 1; snki < SnKmode; snki++)
    {
      uint32_t sizeSNK = ((snki == 1) ? cartSizeLockon : cartSizeSonic2) / 2;
      uint32_t addressSNK = (cartSize / 2) + ((snki == 1) ? 0 : sizeSNK);

      for (uint32_t currBuffer = 0; currBuffer < sizeSNK; currBuffer += 512)
      {
        for (uint16_t currWord = 0, d = 0; currWord < 512; currWord++, d+=2)
        {
          if (d == 512)
          {
            OSCR::Storage::Shared::writeBuffer();
            OSCR::UI::ProgressBar::advance(512);
          }

          uint32_t myAddress = currBuffer + currWord + addressSNK;
          PORTF = myAddress & 0xFF;
          PORTK = (myAddress >> 8) & 0xFF;
          PORTL = (myAddress >> 16) & 0xFF;

          // Arduino running at 16Mhz -> one nop = 62.5ns
          NOP;
          // Setting CS(PH3) LOW
          PORTH &= ~(1 << 3);
          // Setting OE(PH6) LOW
          PORTH &= ~(1 << 6);
          // Setting AS(PJ1) LOW
          PORTJ &= ~(1 << 1);
          // Setting ASEL(PG5) LOW
          PORTG &= ~(1 << 5);
          // Pulse CLK(PH1)
          if (isSVP)
            pulse_clock((snki < 2) ? 10 : 1);

          // most MD ROMs are 200ns, comparable to SNES > use similar access delay of 6 x 62.5 = 375ns
          NOP;
          NOP;
          NOP;
          NOP;
          NOP;
          NOP;

          uint16_t buffPos = ((d < 512) ? (d) : (d - 512));

          // Read
          OSCR::Storage::Shared::buffer[buffPos] = PINA;
          OSCR::Storage::Shared::buffer[buffPos + 1] = PINC;

          // Setting CS(PH3) HIGH
          PORTH |= (1 << 3);
          // Setting OE(PH6) HIGH
          PORTH |= (1 << 6);
          // Setting AS(PJ1) HIGH
          PORTJ |= (1 << 1);
          // Setting ASEL(PG5) HIGH
          PORTG |= (1 << 5);
          // Pulse CLK(PH1)
          if (isSVP)
            pulse_clock((snki < 2) ? 10 : 1);

          if (snki < 2)
          {
            // Skip first 256 words
            if ((currBuffer > 0) || (currWord >= 256))
            {
              calcCKSLockon += ((OSCR::Storage::Shared::buffer[buffPos] << 8) | OSCR::Storage::Shared::buffer[buffPos + 1]);
            }
          }
          else
          {
            calcCKSSonic2 += ((OSCR::Storage::Shared::buffer[buffPos] << 8) | OSCR::Storage::Shared::buffer[buffPos + 1]);
          }
        }

        OSCR::Storage::Shared::writeBuffer();
        OSCR::UI::ProgressBar::advance(512);
      }
    }

    cartOff();

    OSCR::UI::ProgressBar::finish();

    // Close the file:
    OSCR::Storage::Shared::close();

    // Reset SSF2 Banks
    if (cartSize > 0x400000)
    {
      writeSSF2Map(0x50987E, 6);  // 0xA130FD
      writeSSF2Map(0x50987F, 7);  // 0xA130FF
    }

    // Calculate internal checksum
    OSCR::UI::print(FS(OSCR::Strings::Labels::CHK));
    OSCR::UI::printHex(chksum);

    if (chksum != calcCKS)
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
      OSCR::UI::printHexLine(calcCKS);

      OSCR::UI::error(FS(OSCR::Strings::Common::FAIL));
      return;
    }

    OSCR::UI::printLineSync();

    // More checksums
    if (SnKmode >= 2)
    {
      OSCR::UI::print(F("Lock-on "));
      OSCR::UI::print(FS(OSCR::Strings::Labels::CHK));

      if (chksumLockon != calcCKSLockon)
      {
        OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
        OSCR::UI::printHexLine(calcCKSLockon);

        OSCR::UI::error(FS(OSCR::Strings::Common::FAIL));
        return;
      }

      OSCR::UI::printLine();
    }

    if (SnKmode == 3)
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::Plus));
      OSCR::UI::print(FS(OSCR::Strings::Labels::CHK));

      if (chksumSonic2 != calcCKSSonic2)
      {
        OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
        OSCR::UI::printHexLine(calcCKSSonic2);

        OSCR::UI::error(FS(OSCR::Strings::Common::FAIL));
        return;
      }

      OSCR::UI::printLine();
    }
  }

  /******************************************
    SRAM functions
  *****************************************/
  // Sonic 3 sram enable
  void enableSram(bool enableSram)
  {
    dataOut();

    // Set D0 to either 1(enable SRAM) or 0(enable ROM)
    PORTC = enableSram;

    // Strobe TIME(PJ0) LOW to latch the data
    PORTJ &= ~(1 << 0);
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    // Set TIME(PJ0) HIGH
    PORTJ |= (1 << 0);
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    dataIn();
  }

  // Write sram to cartridge
  void writeSram()
  {
    dataOut();

    // Create filepath
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    // Write to the lower byte
    if (saveType == 1)
    {
      for (uint32_t currByte = sramBase; currByte < sramBase + sramSize; currByte++)
      {
        if (segaSram16bit > 0)
        {
          // skip high byte
          OSCR::Storage::Shared::sharedFile.read();
        }

        uint16_t data = OSCR::Storage::Shared::sharedFile.read() & 0xFF;
        writeWord(currByte, data);
      }
    }
    else if (saveType == 2) // Write to the upper byte
    {
      for (uint32_t currByte = sramBase; currByte < sramBase + sramSize; currByte++)
      {
        uint16_t data = (OSCR::Storage::Shared::sharedFile.read() << 8) & 0xFF00;

        writeWord(currByte, data);

        if (segaSram16bit > 0)
        {
          // skip low byte
          OSCR::Storage::Shared::sharedFile.read();
        }
      }
    }
    else if (saveType == 3) // Write to both bytes
    {
      for (uint32_t currByte = sramBase; currByte < sramBase + sramSize; currByte++)
      {
        uint16_t data = (OSCR::Storage::Shared::sharedFile.read() << 8) & 0xFF00;

        data |= (OSCR::Storage::Shared::sharedFile.read() & 0xFF);

        writeWord(currByte, data);
      }
    }
    else
      OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));

    OSCR::Storage::Shared::close();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Common::DONE));

    dataIn();
  }

  // Read sram and save to the SD card
  void readSram()
  {
    dataIn();

    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::MegaDrive), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveRAM));

    for (uint32_t currBuffer = sramBase; currBuffer < sramBase + sramSize; currBuffer += 256)
    {
      for (int currWord = 0; currWord < 256; currWord++)
      {
        uint16_t myWord = readWord(currBuffer + currWord);

        if (saveType == 2)
        {
          // Only use the upper byte
          if (segaSram16bit > 0)
          {
            OSCR::Storage::Shared::buffer[(currWord * 2) + 0] = ((myWord >> 8) & 0xFF);
            OSCR::Storage::Shared::buffer[(currWord * 2) + 1] = ((myWord >> 8) & 0xFF);
          }
          else
          {
            OSCR::Storage::Shared::buffer[currWord] = ((myWord >> 8) & 0xFF);
          }
        }
        else if (saveType == 1)
        {
          // Only use the lower byte
          if (segaSram16bit > 0)
          {
            OSCR::Storage::Shared::buffer[(currWord * 2) + 0] = (myWord & 0xFF);
            OSCR::Storage::Shared::buffer[(currWord * 2) + 1] = (myWord & 0xFF);
          }
          else
          {
            OSCR::Storage::Shared::buffer[currWord] = (myWord & 0xFF);
          }
        }
        else if (saveType == 3) // BOTH
        {
          OSCR::Storage::Shared::buffer[currWord * 2] = ((myWord >> 8) & 0xFF);
          OSCR::Storage::Shared::buffer[(currWord * 2) + 1] = (myWord & 0xFF);
        }
      }

      OSCR::Storage::Shared::writeBuffer((saveType == 3 || segaSram16bit > 0) ? 512 : 256);
    }

    if (segaSram16bit == 2)
    {
      // pad to 64KB
      for (int i = 0; i < 512; i++)
      {
        OSCR::Storage::Shared::buffer[i] = 0xFF;
      }

      uint32_t padsize = (1UL << 16) - (sramSize << 1);
      uint32_t padblockcount = padsize >> 9;  // number of 512 byte blocks

      for (uint32_t i = 0; i < padblockcount; i++)
      {
        OSCR::Storage::Shared::writeBuffer();
      }
    }

    dataIn();

    writeErrors = 0;

    // Open file on sd card
    OSCR::Storage::Shared::rewind();

    for (uint32_t currBuffer = sramBase; currBuffer < sramBase + sramSize; currBuffer += 256)
    {
      for (int currWord = 0; currWord < 256; currWord++)
      {
        uint16_t myWord = readWord(currBuffer + currWord);

        if (saveType & 2)
        {
          // Use the upper byte
          OSCR::Storage::Shared::buffer[(currWord * 2)] = ((myWord >> 8) & 0xFF);
        }

        if (saveType & 1)
        {
          // Use the lower byte
          OSCR::Storage::Shared::buffer[(currWord * 2) + (saveType == 3)] = (myWord & 0xFF);
        }
      }

      uint8_t step = saveType == 3 ? 1 : 2;

      // Check OSCR::Storage::Shared::buffer content against file on sd card
      for (uint16_t i = 0; i < 512; i += step)
      {
        if (saveType == 1 && segaSram16bit > 0)
        {
          // skip high byte
          OSCR::Storage::Shared::sharedFile.read();
        }

        uint8_t b = OSCR::Storage::Shared::sharedFile.read();

        if (saveType == 2 && segaSram16bit > 0)
        {
          // skip low byte
          OSCR::Storage::Shared::sharedFile.read();
        }

        if (b != OSCR::Storage::Shared::buffer[i])
        {
          writeErrors++;
        }
      }
    }

    // Close the file:
    OSCR::Storage::Shared::close();
  }

# if HAS_FLASH
  //******************************************
  // Flashrom Functions
  //******************************************
  void resetFlash()
  {
    // Set data pins to output
    dataOut();

    // Reset command sequence
    writeFlash(0x5555, 0xAA);
    writeFlash(0x2AAA, 0x55);
    writeFlash(0x5555, 0xF0);

    // Set data pins to input again
    dataIn();
  }

  void write29F1610()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    // Get rom size from file
    fileSize = OSCR::Storage::Shared::getSize();

    if (fileSize > flashSize)
    {
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotLargeEnough));
    }

    // Set data pins to output
    dataOut();

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize / 2);

    // Fill OSCR::Storage::Shared::buffer with 1 page at a time then write it repeat until all bytes are written
    for (uint32_t currByte = 0; currByte < fileSize / 2; currByte += 64)
    {
      OSCR::Storage::Shared::readBuffer(128);

      // Write command sequence
      writeFlash(0x5555, 0xAA);
      writeFlash(0x2AAA, 0x55);
      writeFlash(0x5555, 0xA0);

      // Write one full page at a time
      for (uint8_t c = 0, d = 0; c < 64; c++, d+=2)
      {
        uint16_t currWord = ((OSCR::Storage::Shared::buffer[d] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[d + 1] & 0xFF);
        writeFlash(currByte + c, currWord);
      }

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck();

      // update progress bar
      OSCR::UI::ProgressBar::advance(64);
    }

    OSCR::UI::ProgressBar::finish();

    // Set data pins to input again
    dataIn();

    // Close the file:
    OSCR::Storage::Shared::close();
  }

  void write29GL()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    // Get rom size from file
    fileSize = OSCR::Storage::Shared::getSize();

    if (fileSize > flashSize)
    {
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotLargeEnough));
    }

    // Set data pins to output
    dataOut();

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currSdBuffer = 0; currSdBuffer < fileSize; currSdBuffer += 512)
    {
      OSCR::Storage::Shared::readBuffer();

      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 32)
      {
        // Two unlock cycles
        writeFlash(0x555, 0xAA);
        writeFlash(0x2AA, 0x55);

        // Write Buffer Load command to Sector Address
        writeFlash((currSdBuffer + currWriteBuffer) / 2, 0x25);

        // Sector Address, Word count
        writeFlash((currSdBuffer + currWriteBuffer) / 2, 16 - 1);

        uint16_t currWord;

        // Load buffer
        for (uint8_t currByte = 0; currByte < 32; currByte += 2)
        {
          currWord = ((OSCR::Storage::Shared::buffer[currWriteBuffer + currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currWriteBuffer + currByte + 1] & 0xFF);
          writeFlash((currSdBuffer + currWriteBuffer + currByte) / 2, currWord);
        }

        // Write buffer
        writeFlash((currSdBuffer + currWriteBuffer + 32 - 2) / 2, 0x29);

        // Check if write is complete
        // Set data pins to input
        dataIn();

        // Read the status register
        uint16_t statusReg = readFlash((currSdBuffer + currWriteBuffer + 32 - 2) / 2);

        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F))
        {
          statusReg = readFlash((currSdBuffer + currWriteBuffer + 32 - 2) / 2);
        }

        // Set data pins to output
        dataOut();
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    // Set data pins to input again
    dataIn();

    // Close the file:
    OSCR::Storage::Shared::close();
  }

  void idFlash()
  {
    // Set data pins to output
    dataOut();

    // ID command sequence
    writeFlash(0x5555, 0xAA);
    writeFlash(0x2AAA, 0x55);
    writeFlash(0x5555, 0x90);

    // Set data pins to input again
    dataIn();

    // Read the two id bytes into a string
    flashid = (readFlash(0) & 0xFF) << 8;
    flashid |= readFlash(1) & 0xFF;
  }

  uint8_t readStatusReg()
  {
    // Set data pins to output
    dataOut();

    // Status reg command sequence
    writeFlash(0x5555, 0xAA);
    writeFlash(0x2AAA, 0x55);
    writeFlash(0x5555, 0x70);

    // Set data pins to input again
    dataIn();

    // Read the status register
    uint8_t statusReg = readFlash(0);
    return statusReg;
  }

  void eraseFlash()
  {
    // Set data pins to output
    dataOut();

    // Erase command sequence
    writeFlash(0x5555, 0xAA);
    writeFlash(0x2AAA, 0x55);
    writeFlash(0x5555, 0x80);
    writeFlash(0x5555, 0xAA);
    writeFlash(0x2AAA, 0x55);
    writeFlash(0x5555, 0x10);

    // Set data pins to input again
    dataIn();

    busyCheck();
  }

  void blankCheck()
  {
    blank = 1;
    for (uint32_t currByte = 0; currByte < flashSize / 2; currByte++)
    {
      if (readFlash(currByte) != 0xFFFF)
      {
        currByte = flashSize / 2;
        blank = 0;
      }
    }

    if (!blank)
    {
      OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
      return;
    }
  }

  void verifyFlash()
  {
    // Open file on sd card
    OSCR::Storage::Shared::openRO();

    // Get rom size from file
    fileSize = OSCR::Storage::Shared::getSize();

    if (fileSize > flashSize)
    {
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotLargeEnough));
    }

    blank = 0;

    for (uint32_t currByte = 0; currByte < fileSize / 2; currByte += 256)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0, d = 0; c < 256; c++, d+=2)
      {
        uint16_t currWord = ((OSCR::Storage::Shared::buffer[d] << 8) | OSCR::Storage::Shared::buffer[d + 1]);

        if (readFlash(currByte + c) != currWord)
        {
          blank++;
        }
      }
    }

    if (blank != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(blank);
    }

    // Close the file:
    OSCR::Storage::Shared::close();
  }

  // Delay between write operations based on status register
  void busyCheck()
  {
    uint32_t statusReg;

    // Set data pins to input
    dataIn();

    do
    {
      // Read the status register
      statusReg = readFlash(0);
    }
    while ((statusReg | 0xFF7F) != 0xFFFF);

    // Set data pins to output
    dataOut();
  }
# endif /* HAS_FLASH */

  //******************************************
  // EEPROM Functions
  //******************************************
  void EepromInit(uint8_t eepmode)
  {                                  // Acclaim Type 2
    PORTF = 0x00;                    // ADDR A0-A7
    PORTK = 0x00;                    // ADDR A8-A15
    PORTL = 0x10;                    // ADDR A16-A23
    PORTA = 0x00;                    // DATA D8-D15
    PORTH |= (1 << 0);               // /RES HIGH
    PORTC = eepmode;                 // EEPROM Switch:  0 = Enable (Read EEPROM), 1 = Disable (Read ROM)
    PORTH &= ~(1 << 3);              // CE LOW
    PORTH &= ~(1 << 4) & ~(1 << 5);  // /UDSW + /LDSW LOW
    PORTH |= (1 << 6);               // OE HIGH
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
            "nop\n\t");
    PORTH |= (1 << 4) | (1 << 5);  // /UDSW + /LDSW HIGH
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
            "nop\n\t");
  }

  void writeWord_SDA(uint32_t myAddress, uint16_t myData)
  { /* D0 goes to /SDA when only /LWR is asserted */
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
    PORTC = myData;
    PORTH &= ~(1 << 3);  // CE LOW
    PORTH &= ~(1 << 5);  // /LDSW LOW
    PORTH |= (1 << 4);   // /UDSW HIGH
    PORTH |= (1 << 6);   // OE HIGH
    if (eepSize > 0x100)
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
              "nop\n\t");
    else
      delayMicroseconds(100);
    PORTH |= (1 << 5);  // /LDSW HIGH
    if (eepSize > 0x100)
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
              "nop\n\t");
    else
      delayMicroseconds(100);
  }

  void writeWord_SCL(uint32_t myAddress, uint16_t myData)
  { /* D0 goes to /SCL when only /UWR is asserted */
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
    PORTC = myData;
    PORTH &= ~(1 << 3);  // CE LOW
    PORTH &= ~(1 << 4);  // /UDSW LOW
    PORTH |= (1 << 5);   // /LDSW HIGH
    PORTH |= (1 << 6);   // OE HIGH
    if (eepSize > 0x100)
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
              "nop\n\t");
    else
      delayMicroseconds(100);
    PORTH |= (1 << 4);  // /UDSW HIGH
    if (eepSize > 0x100)
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
              "nop\n\t");
    else
      delayMicroseconds(100);
  }

  void writeWord_CM(uint32_t myAddress, uint16_t myData)
  {  // Codemasters
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
    PORTC = myData;
    PORTA = (myData >> 8) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    __asm__("nop\n\t"
            "nop\n\t");

    // Switch WR(PH4) to LOW
    PORTH &= ~(1 << 4);
    // Setting CS(PH3) LOW
    PORTH &= ~(1 << 3);
    // Pulse CLK(PH1)
    PORTH ^= (1 << 1);

    // Leave WR low for at least 200ns
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
            "nop\n\t");

    // Pulse CLK(PH1)
    PORTH ^= (1 << 1);
    // Setting CS(PH3) HIGH
    PORTH |= (1 << 3);
    // Switch WR(PH4) to HIGH
    PORTH |= (1 << 4);

    // Leave WR high for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  }

  // EEPROM COMMANDS
  void EepromStart()
  {
    if (eepType == 2) {               // Acclaim Type 2
      writeWord_SDA(0x100000, 0x00);  // sda low
      writeWord_SCL(0x100000, 0x00);  // scl low
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x01);  // scl high
      writeWord_SDA(0x100000, 0x00);  // sda low
      writeWord_SCL(0x100000, 0x00);  // scl low
    } else if (eepType == 4) {        // EA
      writeWord(0x100000, 0x00);   // sda low, scl low
      writeWord(0x100000, 0xC0);   // sda, scl high
      writeWord(0x100000, 0x40);   // sda low, scl high
      writeWord(0x100000, 0x00);   // START
    } else if (eepType == 5) {        // Codemasters
      writeWord_CM(0x180000, 0x00);   // sda low, scl low
      writeWord_CM(0x180000, 0x02);   // sda low, scl high
      writeWord_CM(0x180000, 0x03);   // sda, scl high
      writeWord_CM(0x180000, 0x02);   // sda low, scl high
      writeWord_CM(0x180000, 0x00);   // START
    } else {
      writeWord(0x100000, 0x00);  // sda low, scl low
      writeWord(0x100000, 0x03);  // sda, scl high
      writeWord(0x100000, 0x02);  // sda low, scl high
      writeWord(0x100000, 0x00);  // START
    }
  }

  void EepromSet0()
  {
    if (eepType == 2) {               // Acclaim Type 2
      writeWord_SDA(0x100000, 0x00);  // sda low
      writeWord_SCL(0x100000, 0x01);  // scl high
      writeWord_SDA(0x100000, 0x00);  // sda low
      writeWord_SCL(0x100000, 0x00);  // scl low
    } else if (eepType == 4) {        // EA
      writeWord(0x100000, 0x00);   // sda low, scl low
      writeWord(0x100000, 0x40);   // sda low, scl high // 0
      writeWord(0x100000, 0x00);   // sda low, scl low
    } else if (eepType == 5) {        // Codemasters
      writeWord_CM(0x180000, 0x00);   // sda low, scl low
      writeWord_CM(0x180000, 0x02);   // sda low, scl high // 0
      writeWord_CM(0x180000, 0x00);   // sda low, scl low
    } else {
      writeWord(0x100000, 0x00);  // sda low, scl low
      writeWord(0x100000, 0x02);  // sda low, scl high // 0
      writeWord(0x100000, 0x00);  // sda low, scl low
    }
  }

  void EepromSet1()
  {
    if (eepType == 2) {               // Acclaim Type 2
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x01);  // scl high
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x00);  // scl low
    } else if (eepType == 4) {        // EA
      writeWord(0x100000, 0x80);   // sda high, scl low
      writeWord(0x100000, 0xC0);   // sda high, scl high // 1
      writeWord(0x100000, 0x80);   // sda high, scl low
      writeWord(0x100000, 0x00);   // sda low, scl low
    } else if (eepType == 5) {        // Codemasters
      writeWord_CM(0x180000, 0x01);   // sda high, scl low
      writeWord_CM(0x180000, 0x03);   // sda high, scl high // 1
      writeWord_CM(0x180000, 0x01);   // sda high, scl low
      writeWord_CM(0x180000, 0x00);   // sda low, scl low
    } else {
      writeWord(0x100000, 0x01);  // sda high, scl low
      writeWord(0x100000, 0x03);  // sda high, scl high // 1
      writeWord(0x100000, 0x01);  // sda high, scl low
      writeWord(0x100000, 0x00);  // sda low, scl low
    }
  }

  void EepromDevice()
  {  // 24C02+
    EepromSet1();
    EepromSet0();
    EepromSet1();
    EepromSet0();
  }

  void EepromSetDeviceAddress(uint16_t addrhi)
  {  // 24C02+
    for (int i = 0; i < 3; i++) {
      if ((addrhi >> 2) & 0x1)  // Bit is HIGH
        EepromSet1();
      else  // Bit is LOW
        EepromSet0();
      addrhi <<= 1;  // rotate to the next bit
    }
  }

  void EepromStatus()
  {  // ACK
    uint8_t eepStatus = 1;
    if (eepType == 1) {              // Acclaim Type 1
      writeWord(0x100000, 0x01);  // sda high, scl low
      writeWord(0x100000, 0x03);  // sda high, scl high
      do {
        dataIn();
        eepStatus = ((readWord(0x100000) >> 1) & 0x1);
        dataOut();
        delayMicroseconds(4);
      } while (eepStatus == 1);
      writeWord(0x100000, 0x01);   // sda high, scl low
    } else if (eepType == 2) {        // Acclaim Type 2
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x01);  // scl high
      do {
        dataIn();
        eepStatus = (readWord(0x100000) & 0x1);
        dataOut();
        delayMicroseconds(4);
      } while (eepStatus == 1);
      writeWord_SCL(0x100000, 0x00);  // scl low
    } else if (eepType == 3) {        // Capcom/Sega
      writeWord(0x100000, 0x01);   // sda high, scl low
      writeWord(0x100000, 0x03);   // sda high, scl high
      do {
        dataIn();
        eepStatus = (readWord(0x100000) & 0x1);
        dataOut();
        delayMicroseconds(4);
      } while (eepStatus == 1);
      writeWord(0x100000, 0x01);  // sda high, scl low
    } else if (eepType == 4) {       // EA
      writeWord(0x100000, 0x80);  // sda high, scl low
      writeWord(0x100000, 0xC0);  // sda high, scl high
      do {
        dataIn();
        eepStatus = ((readWord(0x100000) >> 7) & 0x1);
        dataOut();
        delayMicroseconds(4);
      } while (eepStatus == 1);
      writeWord(0x100000, 0x80);  // sda high, scl low
    } else if (eepType == 5) {       // Codemasters
      writeWord_CM(0x180000, 0x01);  // sda high, scl low
      writeWord_CM(0x180000, 0x03);  // sda high, scl high
      do {
        dataIn();
        eepStatus = ((readWord(0x1C0000) >> 7) & 0x1);
        dataOut();
        delayMicroseconds(4);
      } while (eepStatus == 1);
      writeWord_CM(0x180000, 0x01);  // sda high, scl low
    }
  }

  void EepromReadMode()
  {
    EepromSet1();    // READ
    EepromStatus();  // ACK
  }

  void EepromWriteMode()
  {
    EepromSet0();    // WRITE
    EepromStatus();  // ACK
  }

  void EepromReadData()
  {
    if (eepType == 1) {  // Acclaim Type 1
      for (int i = 0; i < 8; i++) {
        writeWord(0x100000, 0x03);  // sda high, scl high
        dataIn();
        eepbit[i] = ((readWord(0x100000) >> 1) & 0x1);  // Read 0x100000 with Mask 0x1 (bit 1)
        dataOut();
        writeWord(0x100000, 0x01);  // sda high, scl low
      }
    } else if (eepType == 2) {  // Acclaim Type 2
      for (int i = 0; i < 8; i++) {
        writeWord_SDA(0x100000, 0x01);  // sda high
        writeWord_SCL(0x100000, 0x01);  // scl high
        dataIn();
        eepbit[i] = (readWord(0x100000) & 0x1);  // Read 0x100000 with Mask 0x1 (bit 0)
        dataOut();
        writeWord_SDA(0x100000, 0x01);  // sda high
        writeWord_SCL(0x100000, 0x00);  // scl low
      }
    } else if (eepType == 3) {  // Capcom/Sega
      for (int i = 0; i < 8; i++) {
        writeWord(0x100000, 0x03);  // sda high, scl high
        dataIn();
        eepbit[i] = (readWord(0x100000) & 0x1);  // Read 0x100000 with Mask 0x1 (bit 0)
        dataOut();
        writeWord(0x100000, 0x01);  // sda high, scl low
      }
    } else if (eepType == 4) {  // EA
      for (int i = 0; i < 8; i++) {
        writeWord(0x100000, 0xC0);  // sda high, scl high
        dataIn();
        eepbit[i] = ((readWord(0x100000) >> 7) & 0x1);  // Read 0x100000 with Mask (bit 7)
        dataOut();
        writeWord(0x100000, 0x80);  // sda high, scl low
      }
    } else if (eepType == 5) {  // Codemasters
      for (int i = 0; i < 8; i++) {
        writeWord_CM(0x180000, 0x03);  // sda high, scl high
        dataIn();
        eepbit[i] = ((readWord(0x1C0000) >> 7) & 0x1);  // Read 0x1C0000 with Mask 0x1 (bit 7)
        dataOut();
        writeWord_CM(0x180000, 0x01);  // sda high, scl low
      }
    }
  }

  void EepromWriteData(uint8_t data)
  {
    for (int i = 0; i < 8; i++) {
      if ((data >> 7) & 0x1)  // Bit is HIGH
        EepromSet1();
      else  // Bit is LOW
        EepromSet0();
      data <<= 1;  // rotate to the next bit
    }
    EepromStatus();  // ACK
  }

  void EepromFinish()
  {
    if (eepType == 2) {               // Acclaim Type 2
      writeWord_SDA(0x100000, 0x00);  // sda low
      writeWord_SCL(0x100000, 0x00);  // scl low
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x00);  // scl low
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x01);  // scl high
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x00);  // scl low
      writeWord_SDA(0x100000, 0x00);  // sda low
      writeWord_SCL(0x100000, 0x00);  // scl low
    } else if (eepType == 4) {        // EA
      writeWord(0x100000, 0x00);   // sda low, scl low
      writeWord(0x100000, 0x80);   // sda high, scl low
      writeWord(0x100000, 0xC0);   // sda high, scl high
      writeWord(0x100000, 0x80);   // sda high, scl low
      writeWord(0x100000, 0x00);   // sda low, scl low
    } else if (eepType == 5) {        // Codemasters
      writeWord_CM(0x180000, 0x00);   // sda low, scl low
      writeWord_CM(0x180000, 0x01);   // sda high, scl low
      writeWord_CM(0x180000, 0x03);   // sda high, scl high
      writeWord_CM(0x180000, 0x01);   // sda high, scl low
      writeWord_CM(0x180000, 0x00);   // sda low, scl low
    } else {
      writeWord(0x100000, 0x00);  // sda low, scl low
      writeWord(0x100000, 0x01);  // sda high, scl low
      writeWord(0x100000, 0x03);  // sda high, scl high
      writeWord(0x100000, 0x01);  // sda high, scl low
      writeWord(0x100000, 0x00);  // sda low, scl low
    }
  }

  void EepromStop()
  {
    if (eepType == 2) {               // Acclaim Type 2
      writeWord_SDA(0x100000, 0x00);  // sda low
      writeWord_SCL(0x100000, 0x01);  // scl high
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x01);  // scl high
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x00);  // scl low
      writeWord_SDA(0x100000, 0x00);  // sda low
      writeWord_SCL(0x100000, 0x00);  // scl low // STOP
    } else if (eepType == 4) {        // EA
      writeWord(0x100000, 0x00);   // sda, scl low
      writeWord(0x100000, 0x40);   // sda low, scl high
      writeWord(0x100000, 0xC0);   // sda, scl high
      writeWord(0x100000, 0x80);   // sda high, scl low
      writeWord(0x100000, 0x00);   // STOP
    } else if (eepType == 5) {        // Codemasters
      writeWord_CM(0x180000, 0x00);   // sda low, scl low
      writeWord_CM(0x180000, 0x02);   // sda low, scl high
      writeWord_CM(0x180000, 0x03);   // sda, scl high
      writeWord_CM(0x180000, 0x01);   // sda high, scl low
      writeWord_CM(0x180000, 0x00);   // STOP
    } else {
      writeWord(0x100000, 0x00);  // sda, scl low
      writeWord(0x100000, 0x02);  // sda low, scl high
      writeWord(0x100000, 0x03);  // sda, scl high
      writeWord(0x100000, 0x01);  // sda high, scl low
      writeWord(0x100000, 0x00);  // STOP
    }
  }

  void EepromSetAddress(uint16_t address)
  {
    if (eepSize > 0x80) {  // 24C02+
      for (int i = 0; i < 8; i++) {
        if ((address >> 7) & 0x1)  // Bit is HIGH
          EepromSet1();
        else  // Bit is LOW
          EepromSet0();
        address <<= 1;  // rotate to the next bit
      }
      EepromStatus();  // ACK
    } else {           // 24C01
      for (int i = 0; i < 7; i++) {
        if ((address >> 6) & 0x1)  // Bit is HIGH
          EepromSet1();
        else  // Bit is LOW
          EepromSet0();
        address <<= 1;  // rotate to the next bit
      }
    }
  }

  void readEepromByte(uint16_t address)
  {
    addrhi = address >> 8;
    addrlo = address & 0xFF;
    dataOut();
    if (eepType == 2)
      EepromInit(0);  // Enable EEPROM
    EepromStart();    // START
    if (eepSize > 0x80) {
      EepromDevice();         // DEVICE [1010]
      if (eepSize > 0x800) {  // MODE 3 [24C65]
        EepromSetDeviceAddress(0);
        EepromWriteMode();
        EepromSetAddress(addrhi);        // ADDR [A15..A8]
      } else {                           // MODE 2 [24C02/24C08/24C16]
        EepromSetDeviceAddress(addrhi);  // ADDR [A10..A8]
        EepromWriteMode();
      }
    }
    EepromSetAddress(addrlo);
    if (eepSize > 0x80) {
      EepromStart();        // START
      EepromDevice();       // DEVICE [1010]
      if (eepSize > 0x800)  // MODE 3 [24C65]
        EepromSetDeviceAddress(0);
      else                               // MODE 2 [24C02/24C08/24C16]
        EepromSetDeviceAddress(addrhi);  // ADDR [A10..A8]
    }
    EepromReadMode();
    EepromReadData();
    EepromFinish();
    EepromStop();  // STOP
    if (eepType == 2)
      EepromInit(1);  // Disable EEPROM
    // OR 8 bits into byte
    OSCR::Storage::Shared::buffer[addrlo] = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  }

  void writeEepromByte(uint16_t address)
  {
    addrhi = address >> 8;
    addrlo = address & 0xFF;

    dataOut();

    if (eepType == 2)
    {
      EepromInit(0);  // Enable EEPROM
    }

    EepromStart();

    if (eepSize > 0x80)
    {
      // DEVICE [1010]
      EepromDevice();

      if (eepSize > 0x800) // MODE 3 [24C65]
      {
        EepromSetDeviceAddress(0);       // [A2-A0] = 000
        EepromWriteMode();               // WRITE
        EepromSetAddress(addrhi);        // ADDR [A15-A8]
      }
      else // MODE 2 [24C02/24C08/24C16]
      {
        EepromSetDeviceAddress(addrhi);  // ADDR [A10-A8]
        EepromWriteMode();               // WRITE
      }

      EepromSetAddress(addrlo);
    }
    else // 24C01
    {
      EepromSetAddress(addrlo);
      EepromWriteMode();
    }

    EepromWriteData(OSCR::Storage::Shared::buffer[addrlo]);
    EepromStop();  // STOP

    if (eepType == 2)
    {
      EepromInit(1);  // Disable EEPROM
    }
  }

  // Read EEPROM and save to the SD card
  void readEEP()
  {
    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::MegaDrive), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveEEPROM));

    cartOn();
    dataIn();

    if (eepSize > 0x100) // 24C04+
    {
      for (uint16_t currByte = 0; currByte < eepSize; currByte += 256)
      {
        for (int i = 0; i < 256; i++)
        {
          readEepromByte(currByte + i);
        }

        OSCR::Storage::Shared::writeBuffer(256);
      }
    }
    else // 24C01/24C02
    {
      for (uint16_t currByte = 0; currByte < eepSize; currByte++)
      {
        readEepromByte(currByte);
      }

      OSCR::Storage::Shared::writeBuffer(eepSize);
    }

    cartOff();

    OSCR::Storage::Shared::close();
  }

  void writeEEP()
  {
    cartOn();
    dataOut();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    if (eepSize > 0x100) // 24C04+
    {
      for (uint16_t currByte = 0; currByte < eepSize; currByte += 256)
      {
        OSCR::Storage::Shared::readBuffer(256);

        for (int i = 0; i < 256; i++)
        {
          writeEepromByte(currByte + i);
          delay(50);  // DELAY NEEDED
        }

        OSCR::UI::update();
      }
    }
    else // 24C01/24C02
    {
      OSCR::Storage::Shared::readBuffer(eepSize);

      for (uint16_t currByte = 0; currByte < eepSize; currByte++)
      {
        writeEepromByte(currByte);

        if ((currByte != 0) && ((currByte + 1) % 64 == 0))
        {
          OSCR::UI::printLine();
        }

        OSCR::UI::update();  // ON SERIAL = delay(100)
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    printHeader();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  //******************************************
  // CD Backup RAM Functions
  //******************************************
  void readBram()
  {
    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::MegaDrive), FS(OSCR::Strings::Directory::Save), "Cart", FS(OSCR::Strings::FileType::SaveBackup));

    cartOn();
    dataIn();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

    for (uint32_t currByte = 0; currByte < bramSize; currByte += 512)
    {
      for (int i = 0; i < 512; i++)
      {
        OSCR::Storage::Shared::buffer[i] = readWord(0x300000 + currByte + i);
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    OSCR::Storage::Shared::close();
  }

  void writeBram()
  {
    cartOn();
    dataOut();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    // Open file on sd card
    // 0x700000-0x7FFFFF: Writes by /LWR latch D0; 1=RAM write enabled, 0=disabled
    writeWord(0x380000, 1);  // Enable BRAM Writes

    for (uint32_t currByte = 0; currByte < bramSize; currByte += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int i = 0; i < 512; i++)
      {
        writeWord(0x300000 + currByte + i, OSCR::Storage::Shared::buffer[i]);
      }
    }

    writeWord(0x380000, 0);  // Disable BRAM Writes

    cartOff();

    // Close the file:
    OSCR::Storage::Shared::close();

    printHeader();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  //******************************************
  // Realtec Mapper Functions
  //******************************************
  void writeRealtec(uint32_t address, uint8_t value)
  {  // Realtec 0x404000 (UPPER)/0x400000 (LOWER)
    dataOut();
    PORTF = address & 0xFF;          // 0x00 ADDR A0-A7
    PORTK = (address >> 8) & 0xFF;   // ADDR A8-A15
    PORTL = (address >> 16) & 0xFF;  //0x20 ADDR A16-A23
    PORTA = 0x00;                    // DATA D8-D15
    PORTH |= (1 << 0);               // /RES HIGH

    PORTH |= (1 << 3);  // CE HIGH
    PORTC = value;
    PORTH &= ~(1 << 4) & ~(1 << 5);  // /UDSW + /LDSW LOW
    PORTH |= (1 << 4) | (1 << 5);    // /UDSW + /LDSW HIGH
    dataIn();
  }

  void readRealtec()
  {
    // Set control
    cartOn();
    dataIn();

    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::MegaDrive), FS(OSCR::Strings::Directory::ROM), fileName);

    // Realtec Registers
    writeWord(0x201000, 4);  // Number of 128K Blocks 0x402000 (0x201000)
    writeRealtec(0x200000, 1);  // ROM Lower Address 0x400000 (0x200000)
    writeRealtec(0x202000, 0);  // ROM Upper Address 0x404000 (0x202000)

    for (uint32_t currBuffer = 0; currBuffer < cartSize / 2; currBuffer += 256)
    {
      for (uint16_t currWord = 0, d = 0; currWord < 256; currWord++, d+=2)
      {
        uint16_t myWord = readWord(currBuffer + currWord);
        // Split word into two bytes
        // Left
        OSCR::Storage::Shared::buffer[d] = ((myWord >> 8) & 0xFF);
        // Right
        OSCR::Storage::Shared::buffer[d + 1] = (myWord & 0xFF);
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    OSCR::Storage::Shared::close();
  }

  void force_cartSize()
  {
    cartSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeM), MDSize, sizeofarray(MDSize));
    cartSize = MDSize[cartSize] * 131072;
  }
} /* namespace OSCR::Cores::MD */

#endif /* HAS_MD */
