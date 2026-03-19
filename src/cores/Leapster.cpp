//******************************************
// LEAP MODULE
//******************************************
#include "config.h"

#if HAS_LEAP
# include "cores/include.h"
# include "cores/Leapster.h"
# include "cores/Flash.h"

namespace OSCR::Cores::Leapster
{
  // Leapster
  // Cartridge pinout
  // 60P 1.27mm pitch connector
  //
  //     FRONT               BACK
  //      SIDE               SIDE
  //            +---------+
  //       VCC -| 1B   1A |- VCC
  //        nc -| 2B   2A |- GND
  //       D11 -| 3B   3A |- D4
  //        D3 -| 4B   4A |- D12
  //       D10 -| 5B   5A |- D5
  //       GND -| 6B   6A |- D2
  //       D13 -| 7B   7A |- D9
  //        nc -| 8B   8A |- nc
  //        D6 -| 9B   9A |- D1
  //       D14 -| 10B 10A |- D8
  //            |---------|
  //        D7 -| 11B 11A |- GND
  //        D0 -| 12B 12A |- D15
  //      BYTE -| 13B 13A |- OE
  //        nc -| 14B 14A |- A22
  //    FLA_CE -| 15B 15A |- A23
  //        CE -| 16B 16A |- A16
  //        A0 -| 17B 17A |- A15
  //        A1 -| 18B 18A |- A14
  //        A2 -| 19B 19A |- A13
  //       GND -| 20B 20A |- A3
  //       A12 -| 21B 21A |- A4
  //       A11 -| 22B 22A |- A5
  //       A10 -| 23B 23A |- A6
  //        A9 -| 24B 24A |- A7
  //        A8 -| 25B 25A |- A17
  //       A19 -| 26B 26A |- A18
  //       A20 -| 27B 27A |- A21
  //        WE -| 28B 28A |- GND
  //   EEP_SDA -| 29B 29A |- EEP_WP
  //        nc -| 30B 30A |- EEP_SCL
  //            +---------+
  //
  // CONTROL PINS:
  // EEPROM WP (PH0) - SNES /RESET
  // EEPROM SCL (PH1) - SNES CPUCLK
  // EEPROM SDA (PH4) - SNES /IRQ
  // OE (PH3) - SNES /CART
  // WE (PH5) - SNES /WR
  // CE (PH6) - SNES /RD
  // BYTE (PE5) - SNES REFRESH
  // FLASH CE (PJ0) - SNES /PARD

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  using OSCR::Cores::Flash::flashid;

  //******************************************
  // DEFINES
  //******************************************
  // CONTROL PINS - OE/WE/CE
  #define OE_HIGH PORTH |= (1<<3)
  #define OE_LOW PORTH &= ~(1<<3)
  #define WE_HIGH PORTH |= (1<<5)
  #define WE_LOW PORTH &= ~(1<<5)
  #define CE_HIGH PORTH |= (1<<6)
  #define CE_LOW PORTH &= ~(1<<6)

  // BYTE PIN IS A0 IN BYTE MODE - SHIFT ADDRESS >> 1
  #define BYTE_HIGH PORTE |= (1<<5)
  #define BYTE_LOW PORTE &= ~(1<<5)

  // SERIAL I2C EEPROM PINS 24LC02B 2K
  // PIN 5 - SDA
  // PIN 6 - SCL
  // PIN 7 - WP
  // CART B30 = SDA
  // CART A31 = SCL
  // CART A30 = WP
  #define WP_HIGH PORTH |= (1<<0)
  #define WP_LOW PORTH &= ~(1<<0)
  #define SCL_HIGH PORTH |= (1<<1)
  #define SCL_LOW PORTH &= ~(1<<1)
  #define SDA_HIGH PORTH |= (1<<4)
  #define SDA_LOW PORTH &= ~(1<<4)

  // FLASH 39VF040 512K
  // CART B16 = CE#
  #define FL_CE_HIGH PORTJ |= (1<<0)
  #define FL_CE_LOW PORTJ &= ~(1<<0)

  #define DATA_C_READ { DDRC = 0; PORTC = 0xFF; } // [INPUT PULLUP]
  #define DATA_A_READ { DDRA = 0; PORTA = 0xFF; } // [INPUT PULLUP]

  //******************************************
  // VARIABLES
  //******************************************
  // ROM File = LEAP.bin
  // RAM File = SAVE.eep
  // FLASH File = SAVE.fla

  uint8_t LEAPSTER[] = {4, 8, 16};
  uint8_t leaplo = 0; // Lowest Entry
  uint8_t leaphi = 2; // Highest Entry
  uint8_t leapsize;
  uint8_t newleapsize;

  uint8_t tempbyte;
  uint16_t tempword;
  uint16_t ptrword;
  uint16_t tempcheck;

  // EEPROM MAPPING
  // 08 ROM SIZE

  //******************************************
  // ROM STRUCTURE
  //******************************************
  constexpr uint8_t const LEAP[] = {'L', 'E', 'A', 'P'}; // "LEAP" MARKER AT 0x00 OR 0x144 (WORD 0xA2)
  constexpr uint8_t const TBL[] = {0x01, 0x00, 0x00, 0x01}; // TABLE START
  constexpr uint8_t const TXT[] = {0x04, 0x00, 0x00, 0x01}; // NEXT DWORD IS TEXT BLOCK [5 SENTENCES] START
  constexpr uint8_t const VER[] = {0x0A, 0x00, 0x00, 0x01}; // NEXT DWORD IS ROM VERSION LOCATION
  constexpr uint8_t const TTL[] = {0x0B, 0x00, 0x00, 0x01}; // NEXT DWORD IS ROM TITLE LOCATION
  constexpr uint8_t const END[] = {0x10, 0x00, 0x00, 0x01}; // LAST BLOCK - END SEARCH

  uint16_t sentenceAddr = 0; // Sentence Block Start Address
  uint16_t versionAddr = 0;  // Version Address
  uint16_t titleAddr = 0;    // Title Address
  char ROMVersion[20];   // Fosters [20] "155-11172 152-11808"
  char ROMTitle[50];     // "Mr. Pencils Learn to Draw and Write." [37]
  // "Thomas and Friends Calling All Engines" [39]
  // "The Letter Factory.v1.0 - Initial Release JBM3" [47]

  //******************************************
  // DATA INTEGRITY BLOCK
  //******************************************
  // 5 Sentences - 172 bytes
  // Location not static between ROMs

  constexpr uint8_t const LeapCheck[] = {
    0x4C, 0x69, 0x6C, 0x20, 0x64, 0x75, 0x63, 0x6B, 0x65, 0x64, 0x2E, 0x20, 0x20, 0x54, 0x68, 0x65,
    0x20, 0x6A, 0x65, 0x74, 0x20, 0x7A, 0x69, 0x70, 0x70, 0x65, 0x64, 0x20, 0x70, 0x61, 0x73, 0x74,
    0x20, 0x68, 0x65, 0x72, 0x20, 0x68, 0x65, 0x61, 0x64, 0x2E, 0x20, 0x20, 0x44, 0x75, 0x73, 0x74,
    0x20, 0x66, 0x6C, 0x65, 0x77, 0x2C, 0x20, 0x4C, 0x69, 0x6C, 0x20, 0x73, 0x6E, 0x65, 0x65, 0x7A,
    0x65, 0x64, 0x2C, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x4C, 0x65, 0x61, 0x70, 0x20, 0x74, 0x75, 0x72,
    0x6E, 0x65, 0x64, 0x20, 0x72, 0x65, 0x64, 0x2E, 0x20, 0x20, 0x54, 0x68, 0x65, 0x6E, 0x20, 0x4C,
    0x69, 0x6C, 0x20, 0x67, 0x6F, 0x74, 0x20, 0x75, 0x70, 0x2C, 0x20, 0x61, 0x62, 0x6F, 0x75, 0x74,
    0x20, 0x74, 0x6F, 0x20, 0x79, 0x65, 0x6C, 0x6C, 0x2E, 0x20, 0x20, 0x4C, 0x65, 0x61, 0x70, 0x20,
    0x67, 0x61, 0x73, 0x70, 0x65, 0x64, 0x2C, 0x20, 0x22, 0x4C, 0x6F, 0x6F, 0x6B, 0x2C, 0x20, 0x4C,
    0x69, 0x6C, 0x21, 0x20, 0x20, 0x59, 0x6F, 0x75, 0x72, 0x20, 0x74, 0x6F, 0x6F, 0x74, 0x68, 0x21,
    0x20, 0x20, 0x49, 0x74, 0x20, 0x66, 0x65, 0x6C, 0x6C, 0x21, 0x22, 0x00
  };

  //******************************************
  // MENU
  //******************************************
  // Base Menu
  constexpr char const PROGMEM leapmenuItem4[] = "Read EEPROM";
  constexpr char const PROGMEM leapmenuItem5[] = "Write EEPROM";
  constexpr char const PROGMEM leapmenuItem6[] = "Read FLASH";
  constexpr char const PROGMEM leapmenuItem7[] = "Write FLASH";

  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::SelectCart,
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::SetSize,
    leapmenuItem4,
    leapmenuItem5,
    leapmenuItem6,
    leapmenuItem7,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Leapster), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        break;

      case 1: // Read ROM
        readROM();
        break;

      case 2: // Set Size
        setROMSize();
        break;

      case 3: // Read EEPROM
        readEEP();
        break;

      case 4: // Write EEPROM
        writeEEP();
        break;

      case 5: // Read FLASH
        idFLASH();

        if (flashid == 0xBFD7)
        {
          readFLASH();
        }
        break;

      case 6: // Write FLASH
        idFLASH();

        if (flashid == 0xBFD7)
        {
          writeFLASH();
        }
        break;

      case 7: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Leapster));
  }

  //******************************************
  // SETUP
  //******************************************

  void cartOn()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    // Control Pins PH0..PH1, PH3..PH6  BYTE, OE, CE, WE
    DDRH = 0x7B;  // 0b01111011 - CE, WE, SDA, OE, SCL, WP [OUTPUT] - Unused Pins [INPUT]
    PORTH = 0xFF; // 0b11111111 - CE, WE, SDA, OE, SCL, WP [HIGH] - Unused Pins [HIGH]

    // Address Pins
    DDRF = 0xFF;  // Address A0-A7 [OUTPUT]
    DDRK = 0xFF;  // Address A8-A15 [OUTPUT]
    DDRL = 0xFF;  // Address A16-A23 [OUTPUT]
    // Data Pins
    DDRC = 0x00;  // D0-D7 [INPUT]
    DDRA = 0x00;  // D8-D15 [INPUT]
    // BYTE Pin PE5
    DDRE = 0x20;  // 0b00100000 BYTE [OUTPUT] - Unused Pins [INPUT]
    PORTE = 0xFF; // 0b11111111 BYTE [HIGH] - Unused Pins [HIGH]
    // Flash Pin PJ0
    DDRJ = 0x01;  // 0b00000001 - FLA_CE [OUTPUT] - Unused Pins [INPUT]
    PORTJ = 0xFF; // 0b11111111 - FLA_CE [HIGH] - Unused Pins {HIGH]

    // Check Start for "LEAP" marker
    // Read ROM Version & Title
    checkStart();

    // 39VF040 Flash Check
    //  idFLASH();

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Leapster));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ FUNCTIONS
  //******************************************
  // Max ROM Size 0x1000000 (Highest Address = 0xFFFFFF) - FF FFFF
  uint16_t read_rom_word(uint32_t address) // OLD TIMING #1
  {
    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    PORTF = address & 0xFF;
    CE_LOW;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    OE_LOW;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    uint8_t data1 = PINC;
    uint8_t data2 = PINA;
    uint16_t data = (data1 << 8) | (data2);
    OE_HIGH;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    CE_HIGH;

    return data;
  }

  uint8_t read_flash_byte(uint32_t address) // CE HIGH, OE LOW, FL_CE LOW
  {
    PORTL = (address >> 17) & 0x7F;
    PORTK = (address >> 9) & 0xFF;
    PORTF = (address >> 1) & 0xFF;
    if (address & 0x1) // BYTE = A0
      BYTE_HIGH;
    else
      BYTE_LOW;
    CE_HIGH;
    OE_LOW;
    FL_CE_LOW;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    uint8_t data = PINC;
    FL_CE_HIGH;
    OE_HIGH;

    return data;
  }

  //******************************************
  // WRITE FUNCTION
  //******************************************

  void write_flash_byte(uint32_t address, uint8_t data)
  {
    PORTL = (address >> 17) & 0x7F;
    PORTK = (address >> 9) & 0xFF;
    PORTF = (address >> 1) & 0xFF;
    if (address & 0x1) // BYTE = A0
      BYTE_HIGH;
    else
      BYTE_LOW;
    OE_HIGH;
    FL_CE_LOW;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    PORTC = data;
    WE_LOW;
    WE_HIGH;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    FL_CE_HIGH;
    OE_LOW;
  }

  //******************************************
  // DATA INTEGRITY FUNCTIONS
  //******************************************

  void checkStart()
  {
    delay(500);

    OE_LOW;
    CE_LOW;
    BYTE_HIGH;

    tempword = read_rom_word(0x0);

    if ((LEAP[0] == ((tempword >> 0x8) & 0xFF)) && (LEAP[1] == (tempword & 0xFF)))
    {
      tempword = read_rom_word(0x1);
      findTable(0x4, 0xA2);
    }
    else
    {
      delay(500);

      tempword = read_rom_word(0xA2);

      if ((LEAP[0] == ((tempword >> 0x8) & 0xFF)) && (LEAP[1] == (tempword & 0xFF)))
      {
        tempword = read_rom_word(0xA3);
        findTable(0xA4, 0x146);
      }
    }

    CE_HIGH;
    OE_HIGH;
  }

  void findTable(uint32_t startAddr, uint32_t endAddr)
  {
    delay(500);

    CE_LOW;

    for (uint32_t addr = startAddr; addr < endAddr; addr += 2)
    {
      tempword = read_rom_word(addr);

      if ((TBL[0] == ((tempword >> 0x8) & 0xFF)) && (TBL[1] == (tempword & 0xFF)))
      {
        tempword = read_rom_word(addr + 1);

        if ((TBL[2] == ((tempword >> 0x8) & 0xFF)) && (TBL[3] == (tempword & 0xFF)))
        {
          readTable(startAddr, endAddr);
          break;
        }
      }
    }

    CE_HIGH;
  }

  void readTable(uint32_t startAddr, uint32_t endAddr)
  {
    delay(500);

    CE_LOW;

    for (uint32_t addr = startAddr; addr < endAddr; addr++)
    {
      tempword = read_rom_word(addr);

      if ((TXT[0] == ((tempword >> 0x8) & 0xFF)) && (TXT[1] == (tempword & 0xFF)))
      {
        tempword = read_rom_word(addr + 1);

        if ((TXT[2] == ((tempword >> 0x8) & 0xFF)) && (TXT[3] == (tempword & 0xFF))) // Text Block Marker Found
        {
          ptrword = read_rom_word(addr + 2);
          sentenceAddr = (((ptrword >> 8) & 0xFF) | ((ptrword & 0xFF) << 8)); // Swap Byte Order
        }
      }
      else if ((VER[0] == ((tempword >> 0x8) & 0xFF)) && (VER[1] == (tempword & 0xFF)))
      {
        tempword = read_rom_word(addr + 1);

        if ((VER[2] == ((tempword >> 0x8) & 0xFF)) && (VER[3] == (tempword & 0xFF))) // Version Marker Found
        {
          ptrword = read_rom_word(addr + 2);
          versionAddr = (((ptrword >> 8) & 0xFF) | ((ptrword & 0xFF) << 8)); // Swap Byte Order
        }
      }
      else if ((TTL[0] == ((tempword >> 0x8) & 0xFF)) && (TTL[1] == (tempword & 0xFF)))
      {
        tempword = read_rom_word(addr + 1);

        if ((TTL[2] == ((tempword >> 0x8) & 0xFF)) && (TTL[3] == (tempword & 0xFF))) // Title Marker Found
        {
          ptrword = read_rom_word(addr + 2);
          titleAddr = (((ptrword >> 8) & 0xFF) | ((ptrword & 0xFF) << 8)); // Swap Byte Order
        }
      }
      else if ((END[0] == ((tempword >> 0x8) & 0xFF)) && (END[1] == (tempword & 0xFF)))
      {
        tempword = read_rom_word(addr + 1);

        if ((END[2] == ((tempword >> 0x8) & 0xFF)) && (END[3] == (tempword & 0xFF))) // END OF TABLE
        {
          break;
        }
      }
    }

    CE_HIGH;
  //  OSCR::UI::print(F("Text Addr: "));
  //  OSCR::UI::printHexLine(sentenceAddr);
  //  OSCR::UI::print(F("Version Addr: "));
  //  OSCR::UI::printHexLine(versionAddr);
  //  OSCR::UI::print(F("Title Addr: "));
  //  OSCR::UI::printHexLine(titleAddr);
  //  OSCR::UI::update();

    delay(500);

    CE_LOW;

    for (int x = 0; x < 10; x++)
    {
      tempword = read_rom_word((versionAddr / 2) + x);

      ROMVersion[x * 2] = (tempword >> 0x8) & 0xFF;
      ROMVersion[(x * 2) + 1] = tempword & 0xFF;
    }

    delay(500);

    CE_LOW;

    for (int x = 0; x < 25; x++)
    {
      tempword = read_rom_word((titleAddr / 2) + x);

      ROMTitle[x * 2] = (tempword >> 0x8) & 0xFF;
      ROMTitle[(x * 2) + 1] = tempword & 0xFF;
    }
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Leapster), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    for (uint32_t address = 0, addressMax = 0x100000 << (leapsize + 1); address < addressMax; address++)
    {
      tempword = read_rom_word(address);

      uint16_t buffPos = (address % 256) * 2;

      OSCR::Storage::Shared::buffer[buffPos] = (tempword >> 0x8) & 0xFF;
      OSCR::Storage::Shared::buffer[buffPos + 1] = tempword & 0xFF;

      if (buffPos == 510) OSCR::Storage::Shared::writeBuffer();
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();
  }

  //******************************************
  // ROM SIZE
  //******************************************
  void setROMSize()
  {
    leapsize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeMB), LEAPSTER, sizeofarray(LEAPSTER));

    printHeader();

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, LEAPSTER[leapsize] * 1024 * 1024);
  }

  //******************************************
  // FLASH SAVES
  //******************************************
  // FLASH 39VF040
  // MR. PENCIL'S LEARN TO DRAW & WRITE
  // TOP SECRET - PERSONAL BEESWAX
  // FLASH IS BYTE MODE
  // BYTE PIN IS A0 - SHIFT ADDRESS >> 1

  void dataOut()
  {
    DDRC = 0xFF;
    DDRA = 0xFF;
  }

  void dataIn()
  {
    DDRC = 0x00;
    DDRA = 0x00;
  }

  void idFLASH() // "BFD7" = 39VF040
  {
    printHeader();

    dataOut();

    resetFLASH();

    write_flash_byte(0x5555, 0xAA);
    write_flash_byte(0x2AAA, 0x55);
    write_flash_byte(0x5555, 0x90);

    dataIn();

    flashid = read_flash_byte(0x0000) << 8;
    flashid |= read_flash_byte(0x0001);

    dataOut();

    resetFLASH();

    dataIn();

    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    OSCR::UI::printHexLine(flashid);

    OSCR::UI::update();
  }

  void resetFLASH()
  {
    write_flash_byte(0x5555, 0xF0);
  }

  void eraseFLASH()
  {
    write_flash_byte(0x5555, 0xAA);
    write_flash_byte(0x2AAA, 0x55);
    write_flash_byte(0x5555, 0x80);
    write_flash_byte(0x5555, 0xAA);
    write_flash_byte(0x2AAA, 0x55);
    write_flash_byte(0x5555, 0x10);
  }

  void programFLASH()
  {
    write_flash_byte(0x5555, 0xAA);
    write_flash_byte(0x2AAA, 0x55);
    write_flash_byte(0x5555, 0xA0);
  }

  void statusFLASH()
  {
    uint8_t flashStatus = 1;
    do {
      flashStatus = ((PORTA & 0x80) >> 7); // D7 = PORTA7
      _delay_us(4);
    }
    while (flashStatus == 1);
  }

  void readFLASH()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Leapster), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveFlash));

    OSCR::UI::ProgressBar::init(0x80000, 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    cartOn();

    CE_HIGH;
    OE_LOW;
    FL_CE_LOW;

    for (uint32_t address = 0x0; address < 0x80000; address += 512) // 512K
    {
      for (uint16_t x = 0; x < 512; x++)
      {
        // CONSOLE READS ADDRESS 4X
        do
        {
          tempbyte = read_flash_byte(address + x);
          tempcheck = read_flash_byte(address + x);
        }
        while (tempbyte != tempcheck);

        OSCR::Storage::Shared::buffer[x] = tempbyte;
      }

      OSCR::UI::ProgressBar::advance(512);

      OSCR::Storage::Shared::writeBuffer();
    }

    FL_CE_HIGH;
    OE_HIGH;

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::ProgressBar::finish();
  }

  void writeFLASH()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

    printHeader();

    OSCR::UI::ProgressBar::init(0x80000, 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    cartOn();

    CE_HIGH;

    dataOut();

    eraseFLASH();
    statusFLASH();

    delay(100); // WORKS ~75 OK

    for (uint32_t address = 0x0; address < 0x80000; address += 512) // 512K
    {
      OSCR::Storage::Shared::fill();

      for (uint16_t x = 0; x < 512; x++)
      {
        programFLASH();

        write_flash_byte(address + x, OSCR::Storage::Shared::buffer[x]);
      }

      OSCR::UI::ProgressBar::advance(512);
    }

    dataIn();

    cartOff();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();
  }

  //******************************************
  // EEPROM SAVES
  //******************************************
  // EEPROM 24LC02/24LC16
  // 24LC02B = 256 BYTES
  // 24LC16B = 2048 BYTES
  // DORA THE EXPLORER - WILDLIFE RESCUE 24LC16B
  // SCHOLASTIC MATH MISSIONS 24LC16B

  // MODIFIED FOR COMPATIBILITY WITH ISSI EEPROM
  // PET PALS ISSI 416A-2GLI = 24C16A

  // START - SDA HIGH TO LOW WHEN SCL HIGH
  void eepromStart()
  {
    SCL_LOW;
    SDA_LOW;
    SCL_HIGH;
    _delay_us(2);
    SDA_HIGH;
    _delay_us(2);
    SDA_LOW;
    _delay_us(2);
    SCL_LOW;
    _delay_us(2);
  }

  void eepromSet0()
  {
    SCL_LOW;
    SDA_LOW;
    _delay_us(2);
    SCL_HIGH;
    _delay_us(5);
    SCL_LOW;
    _delay_us(2);
  }

  void eepromSet1()
  {
    SCL_LOW;
    SDA_LOW;
    _delay_us(2);
    SDA_HIGH;
    _delay_us(2);
    SCL_HIGH;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP; // ~500ns
    SCL_LOW;
    SDA_LOW;
    _delay_us(2);
  }

  void eepromDevice()
  {
    eepromSet1();
    eepromSet0();
    eepromSet1();
    eepromSet0();
  }

  void eepromSetDeviceAddress(uint32_t addrhi)
  {
    for (int i = 0; i < 3; i++)
    {
      if ((addrhi >> 2) & 0x1) // Bit is HIGH
      {
        eepromSet1();
      }
      else // Bit is LOW
      {
        eepromSet0();
      }

      addrhi <<= 1; // rotate to the next bit
    }
  }

  void eepromStatus() // ACK
  {
    uint8_t eepStatus = 1;

    DDRH &= ~(1 << 4); // SDA to INPUT

    SCL_LOW;
    _delay_us(2);
    SCL_HIGH;
    _delay_us(5);
    SCL_LOW;

    do
    {
      eepStatus = ((PINH & 0x10) >> 4); // SDA = PORTH4
    }
    while (eepStatus == 1);

    DDRH |= (1 << 4); // SDA to OUTPUT
  }

  void eepromReadMode()
  {
    eepromSet1(); // READ
    eepromStatus(); // ACK
  }

  void eepromWriteMode()
  {
    eepromSet0(); // WRITE
    eepromStatus(); // ACK
  }

  void eepromReadData()
  {
    DDRH &= ~(1 << 4); // SDA to INPUT

    for (int i = 0; i < 8; i++)
    {
      SCL_LOW;
      SDA_LOW;
      _delay_us(2);
      SCL_HIGH;
      eepbit[i] = ((PINH & 0x10) >> 4); // SDA = PORTH4

      if (eepbit[i] == 1)
      {
        SCL_LOW;
        _delay_us(4);
      }
      else
      {
        _delay_us(4);
        SCL_LOW;
      }

      _delay_us(2);
    }

    DDRH |= (1 << 4); // SDA to OUTPUT
  }

  void eepromWriteData(uint8_t data)
  {
    for (int i = 0; i < 8; i++)
    {
      if ((data >> 7) & 0x1) // Bit is HIGH
      {
        eepromSet1();
      }
      else // Bit is LOW
      {
        eepromSet0();
      }

      data <<= 1; // rotate to the next bit
    }

    eepromStatus(); // ACK
  }

  // STOP - SDA LOW TO HIGH WHEN SCL HIGH
  void eepromStop()
  {
    SCL_LOW;
    SDA_LOW;
    _delay_us(1);
    SCL_HIGH;
    _delay_us(2);
    SDA_HIGH;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP; // ~500nS
    SCL_LOW;
    SDA_LOW;
  }

  void eepromSetAddress(uint16_t address)
  {
    for (int i = 0; i < 8; i++)
    {
      if ((address >> 7) & 0x1) // Bit is HIGH
      {
        eepromSet1();
      }
      else // Bit is LOW
      {
        eepromSet0();
      }

      address <<= 1; // rotate to the next bit
    }

    eepromStatus(); // ACK
  }

  void readEepromByte(uint16_t address)
  {
    uint16_t addrhi = address >> 8;
    uint16_t addrlo = address & 0xFF;

    eepromStart(); // START
    eepromDevice(); // DEVICE [1010]
    eepromSetDeviceAddress(addrhi); // ADDR [A10..A8]
    eepromWriteMode();
    eepromSetAddress(addrlo);
    eepromStart(); // START
    eepromDevice(); // DEVICE [1010]
    eepromSetDeviceAddress(addrhi); // ADDR [A10..A8]
    eepromReadMode();
    eepromReadData();
    eepromStop(); // STOP

    // OR 8 bits into byte
    OSCR::Storage::Shared::buffer[addrlo] = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  }

  void writeEepromByte(uint16_t address)
  {
    uint16_t addrhi = address >> 8;
    uint16_t addrlo = address & 0xFF;

    eepromStart(); // START
    eepromDevice(); // DEVICE [1010]
    eepromSetDeviceAddress(addrhi); // ADDR [A10-A8]
    eepromWriteMode(); // WRITE
    eepromSetAddress(addrlo);
    eepromWriteData(OSCR::Storage::Shared::buffer[addrlo]);
    eepromStop(); // STOP
  }

  // Read EEPROM and save to the SD card
  void readEEP()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Leapster), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveEEPROM));

    cartOn();

    for (uint16_t currByte = 0; currByte < 2048; currByte += 256)
    {
      for (int i = 0; i < 256; i++)
      {
        readEepromByte(currByte + i);
      }

      OSCR::Storage::Shared::writeBuffer(256);
    }

    // 24LC02
    //    for (uint16_t currByte = 0; currByte < 256; currByte++)
    //    {
    //      readEepromByte(currByte);
    //    }
    //
    //    OSCR::Storage::Shared::writeBuffer(256);

    cartOff();

    OSCR::Storage::Shared::close();
  }

  void writeEEP()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

    printHeader();

    OSCR::UI::ProgressBar::init(2048, 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    cartOn();

    WP_LOW;

    for (uint16_t currByte = 0; currByte < 2048; currByte += 256)
    {
      OSCR::Storage::Shared::readBuffer(256);

      for (int i = 0; i < 256; i++)
      {
        writeEepromByte(currByte + i);
        delay(50);
      }

      OSCR::UI::ProgressBar::advance(256);
    }

    WP_HIGH;

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::ProgressBar::finish();
  }

  //******************************************
  // CART SELECT CODE
  //******************************************

  void setCart()
  {
    OSCR::Databases::Standard::browseDatabase();

    romSize = romDetail->size;

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::Leapster */

#undef OE_HIGH
#undef OE_LOW
#undef WE_HIGH
#undef WE_LOW
#undef CE_HIGH
#undef CE_LOW
#undef BYTE_HIGH
#undef BYTE_LOW
#undef WP_HIGH
#undef WP_LOW
#undef SCL_HIGH
#undef SCL_LOW
#undef SDA_HIGH
#undef SDA_LOW
#undef FL_CE_HIGH
#undef FL_CE_LOW
#undef DATA_C_READ
#undef DATA_A_READ

#endif
