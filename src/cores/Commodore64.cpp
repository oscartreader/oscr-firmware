//******************************************
// COMMODORE 64 MODULE
//******************************************
#include "config.h"

#if HAS_C64
# include "cores/include.h"
# include "cores/Commodore64.h"

namespace OSCR::Cores::Commodore64
{
  // Commodore 64
  // Cartridge Pinout
  // 44P 2.54mm pitch connector
  //
  //   FRONT             BACK
  //    SIDE             SIDE
  //          +-------+
  //     GND -| 1   A |- GND
  //   +5VDC -| 2   B |- /ROMH
  //   +5VDC -| 3   C |- /RESET
  //    /IRQ -| 4   D |- /NMI
  //     R/W -| 5   E |- PHI2
  //  DOTCLK -| 6   F |- A15
  //    /IO1 -| 7   H |- A14
  //   /GAME -| 8   J |- A13
  //  /EXROM -| 9   K |- A12
  //    /IO2 -| 10  L |- A11
  //   /ROML -| 11  M |- A10
  //      BA -| 12  N |- A9
  //    /DMA -| 13  P |- A8
  //      D7 -| 14  R |- A7
  //      D6 -| 15  S |- A6
  //      D5 -| 16  T |- A5
  //      D4 -| 17  U |- A4
  //      D3 -| 18  V |- A3
  //      D2 -| 19  W |- A2
  //      D1 -| 20  X |- A1
  //      D0 -| 21  Y |- A0
  //     GND -| 22  Z |- GND
  //          +-------+
  //
  //                                                 TOP
  //       +---------------------------------------------------------------------------------------+
  //       | 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22 |
  // LEFT  |                                                                                       | RIGHT
  //       | A   B   C   D   E   F   H   J   K   L   M   N   P   R   S   T   U   V   W   X   Y   Z |
  //       +---------------------------------------------------------------------------------------+
  //                                                BOTTOM
  //
  // CONTROL PINS:
  // /RESET(PH0) - SNES RESET
  // PHI2(PH1)   - SNES CPUCLK
  // /GAME(PH3)  - SNES /CS
  // /EXROM(PH4) - SNES /IRQ
  // R/W(PH6)    - SNES /RD
  // /ROML(PL0)  - SNES A16
  // /ROMH(PL1)  - SNES A17
  // /IO1(PL2)   - SNES A18
  // /IO2(PL3)   - SNES A19

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  using OSCR::Databases::Basic::mapperDetail;

  //******************************************
  //  Defines
  //******************************************
  #define PHI2_ENABLE PORTH |= (1 << 1)
  #define PHI2_DISABLE PORTH &= ~(1 << 1)
  #define ROML_DISABLE PORTL |= (1 << 0)
  #define ROML_ENABLE PORTL &= ~(1 << 0)
  #define ROMH_DISABLE PORTL |= (1 << 1)
  #define ROMH_ENABLE PORTL &= ~(1 << 1)
  #define IO1_DISABLE PORTL |= (1 << 2)
  #define IO1_ENABLE PORTL &= ~(1 << 2)
  #define IO2_DISABLE PORTL |= (1 << 3)
  #define IO2_ENABLE PORTL &= ~(1 << 3)

  //******************************************
  //  Supported Mappers
  //******************************************
  // Supported Mapper Array
  // Format = {romSizeLow, romSizeHigh}

  constexpr uint16_t const romSizes[] = {
    4,
    8,
    12,
    16,
    20,
    32,
    64,
    128,
    256,
    512,
  };

  uint8_t mapper;
  uint8_t romBanks;

  // EEPROM MAPPING
  // 07 MAPPER
  // 08 ROM SIZE

  //******************************************
  //  MENU
  //******************************************
  // Base Menu
  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::SelectCart,
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::SetSize,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Commodore64), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        checkStatus();
        break;

      case 1: // Read ROM
        readROM();
        break;

      case 2: // Set Mapper + Size
        setMapper();
        setROMSize();
        checkStatus();
        break;

      case 3: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Commodore64));
  }

  //******************************************
  //  SETUP
  //******************************************
  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // C64 uses A0-A15
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23 - Use A16-A19 for /ROML, /ROMH, /IO1, /IO2
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       /RST(PH0) ---(PH5)   R/W(PH6)
    DDRH |= (1 << 0) | (1 << 5) | (1 << 6);

    // Set Port Pins to Input
    //      /GAME(PH3) /EXROM(PH4)
    DDRH &= ~((1 << 3) | (1 << 4));

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       /RST(PH0)  ---(PH5)   R/W(PH6)
    PORTH |= (1 << 0) | (1 << 5) | (1 << 6);

    // Set /ROML, /ROMH, /IO1, /IO2 to HIGH
    PORTL = 0xFF;  // A16-A23 (A16 = /ROML, A17 = /ROMH, A18 = /IO1, A19 = /IO2)

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTJ |= (1 << 0);  // TIME(PJ0)

  #ifdef ENABLE_CLOCKGEN
    // Clock Generator
    if (!OSCR::ClockGen::initialize())
    {
      OSCR::UI::clear();
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::ClockGenMissing));
    }

    // Set Eeprom clock to 1Mhz
    OSCR::ClockGen::clockgen.set_freq(100000000ULL, SI5351_CLK1);

    // Start outputting Eeprom clock
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 1);  // Eeprom clock

    // Wait for clock generator
    OSCR::ClockGen::clockgen.update_status();

  #else
    // Set PHI2(PH1) to Output
    DDRH |= (1 << 1);

    // Setting Control Pins to HIGH for PHI2(PH1)
    PHI2_ENABLE;
  #endif

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Extended::setup(FS(OSCR::Strings::FileType::Commodore64));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ DATA
  //******************************************
  uint8_t readData(uint16_t addr)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
    NOP;
    NOP;

    // Set R/W(PH6) to HIGH
    PORTH |= (1 << 6);  // R/W HIGH (READ)
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

    return ret;
  }

  void readSegment(uint16_t startaddr, uint32_t endaddr, uint16_t size)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += size)
    {
      for (uint16_t w = 0; w < size; w++)
      {
        uint8_t temp = readData(addr + w);
        OSCR::Storage::Shared::buffer[w] = temp;
      }

      OSCR::Storage::Shared::writeBuffer(size);
    }
  }

  void readSegmentEnableDisable(uint16_t startaddr, uint32_t endaddr, uint8_t romLow, uint16_t size)
  {
    PORTL &= ~(1 << romLow); // enable ROML or ROMH
    readSegment(startaddr, endaddr, size);
    PORTL |= (1 << romLow); // disable ROML or ROMH
  }

  void readSegment16k()
  {
    readSegmentEnableDisable(0x8000, 0xA000, 0);  // 8K
    readSegmentEnableDisable(0xA000, 0xC000, 1);  // +8K = 16K
  }

  void readSegmentBankD0D5(uint16_t banks, uint16_t address, uint8_t romLow)
  {
    PORTL &= ~(1 << romLow); // enable ROML or ROMH

    uint32_t endAddress = address + 0x2000;

    for (uint16_t x = 0; x < banks; x++)
    {
      bankSwitch(0xDE00, x);        // Switch Bank using D0-D5
      readSegment(address, endAddress);
    }

    PORTL |= (1 << romLow); // disable ROML or ROMH
  }

  void readSegmentBankA0A4(uint16_t banks)
  {
    ROML_ENABLE;

    for (uint16_t x = 0; x < banks; x++)
    {
      bankSwitch(0xDE00 + x, 0);    // Switch Bank using address lines
      readSegment(0x8000, 0xA000);  // 8K per bank
    }

    ROML_DISABLE;
  }

  //******************************************
  // WRITE DATA
  //******************************************
  void writeData(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
    NOP;
    NOP;

    DDRC = 0xFF;  // Set to Output
    PORTC = data;
    NOP;
    NOP;
    NOP;

    // Set R/W(PH6) to LOW
    PORTH &= ~(1 << 6);  // R/W LOW (WRITE)
    NOP;
    NOP;
    NOP;

    // Set R/W(PH6) to HIGH
    PORTH |= (1 << 6);
    NOP;
    NOP;

    DDRC = 0x00;  // Reset to Input
  }

  void bankSwitch(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
    NOP;
    NOP;
    NOP;

    DDRC = 0xFF;  // Set to Output
    PORTC = data;
    NOP;
    NOP;
    NOP;

    // Latch Bank Data
    PHI2_DISABLE;                      // PHI2 LOW
    if (((addr >> 8) & 0xFF) == 0xDF)  // 0xDFxx
      IO2_ENABLE;
    else if (((addr >> 8) & 0xFF) == 0xDE)  // 0xDExx
      IO1_ENABLE;
    PORTH &= ~(1 << 6);  // R/W LOW (WRITE)
    NOP;
    NOP;
    NOP;
    PORTH |= (1 << 6);  // R/W HIGH (READ)
    PHI2_ENABLE;        // PHI2 HIGH
    IO2_DISABLE;
    IO1_DISABLE;

    DDRC = 0x00;  // Reset to Input
  }

  //******************************************
  // READ PORT STATE
  //******************************************
  uint8_t readPorts()
  {
    return (PINH >> 3) & 0x3;
  }

  //******************************************
  // READ ROM
  //******************************************
  // ADDRESS RANGES
  // $8000-$9FFF/$A0000-$BFFF/$E000-$FFFF
  // NORMAL (EXROM LOW/GAME LOW): ROML = $8000, ROMH = $A000
  // ULTIMAX (EXROM HIGH/GAME LOW/): ROML = $8000, ROMH = $E000
  // GAME HIGH/EXROM LOW: ROML = $8000

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Commodore64), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::Raw));

    cartOn();

    switch (mapper)
    {
      case 0:  // Normal (4K/8K/16K) & Ultimax (8K/16K)
        // ULTIMAX CARTS
        if (readPorts() == 2) // 2 = 10 = EXROM HIGH/GAME LOW
        {
          if (romSize > 1) // 16K [NO ROML FOR 8K]
          {
            readSegmentEnableDisable(0x8000, 0xA000, 0); // 8K
          }

          readSegmentEnableDisable(0xE000, 0x10000, 1);  // +8K = 8K/16K
        }
        else // NORMAL CARTS
        {
          if (romSize > 0)
          {
            readSegmentEnableDisable(0x8000, 0xA000, 0);   // 8K

            if (romSize > 1)
              readSegmentEnableDisable(0xA000, 0xC000, 1); // +8K = 16K
          }
          else
            readSegmentEnableDisable(0x9000, 0xA000, 0);  // 4K
        }

        break;

      case 1:          // Action Replay (32K)
      case 9:          // Atomic Power (32K)
        ROML_ENABLE;

        for (int x = 0; x < 4; x++)
        {
          bankSwitch(0xDE00, x << 3);   // Switch Bank using D3-D4
          readSegment(0x8000, 0xA000);  // 8K *4 = 32K
        }

        ROML_DISABLE;

        break;

      case 2:          // KCS Power Cartridge (16K)
      case 11:         // Westermann Learning (16K)
      case 16:         // WarpSpeed (16K)
        readSegment16k();
        break;

      case 3:           // Final Cartridge III (64K)
        for (int x = 0; x < 4; x++)
        {
          bankSwitch(0xDFFF, 0x40 + x);  // Switch Bank using $DFFF
          readSegment16k();
        }
        break;

      case 4:          // Simons Basic (16K)
        readSegmentEnableDisable(0x8000, 0xA000, 0);  // 8K
        ROMH_ENABLE;
        bankSwitch(0xDE00, 0x1);      // Switch Bank to ROM
        readSegment(0xA000, 0xC000);  // +8K = 16K
        ROMH_DISABLE;
        break;

      // Ocean Bank 1/B (Single Chip) Selection Notes (Luigi Di Fraia):
      // 128 KiB cartridges (all known titles): bits 0-3 at $DE00 and /ROML (single 128 KiB chip with A16 on pin 22, rather than the /OE signal)
      // 256 KiB cartridges (just "Chase H.Q. II"): bits 0-4 at $DE00 and /ROML (single 256 KiB chip)
      // 512 KiB cartridges (just "Terminator 2"):  bits 0-5 at $DE00 and /ROML (single 512 KiB chip)

      // Ocean 256K ROM Start Data
      // Single Chip:
      // Chase HQ II          09 80 63 80 C3 C2 CD 38 30 4C 63 80 4C C8 80 80
      // Space Gun            09 80 09 80 C3 C2 CD 38 30 78 A2 FF 9A A9 E7 85
      // Two Chip:
      // Robocop 2            09 80 75 80 C3 C2 CD 38 30 4C 75 80 4C FA 80 80
      // Shadow of the Beast  09 80 83 81 C3 C2 CD 38 30 4C 83 81 4C 76 82 80
      // Read 0x8002 to determine whether Single Chip or Two Chip
      // IF 0x75 OR 0x83, THEN Two Chip ELSE Single Chip

      case 5: // Ocean 128K/256K/512K
        {
          ROML_ENABLE;
          bankSwitch(0xDE00, 0);  // Reset Bank 0
          uint8_t checkOcean = readData(0x8002);
          ROML_DISABLE;
          if ((romSize == 8) && ((checkOcean == 0x75) || (checkOcean == 0x83))) // Two Chip 256K
          {
            // Robocop 2 + Shadow of the Beast
            OSCR::UI::printLineSync(F("TWO CHIP"));

            readSegmentBankD0D5(16, 0x8000, 0); // 8K * 16 = 128K
            readSegmentBankD0D5(16, 0xA000, 1); // 8K * 16 = +128K = 256K
          }
          else // Single Chip 128K/256K/512K
          {
            OSCR::UI::printLineSync(F("SINGLE CHIP"));

            romBanks = romSizes[romSize] / 8;
            readSegmentBankD0D5(romBanks, 0x8000, 0); // 8K * Banks = 128K/256K/512K
          }
        }
        break;

      case 6:           // Expert Cartridge (8K)
        readSegmentEnableDisable(0x8000, 0xA000, 0);
        break;

      case 7:          // Fun Play, Power Play (128K)
        ROML_ENABLE;

        for (int x = 0; x < 8; x++)
        {
          bankSwitch(0xDE00, x * 8);    // Switch Bank 0-8
          readSegment(0x8000, 0xA000);  // 8K * 8 = 64K
        }

        ROML_DISABLE;
        ROMH_ENABLE;

        for (int x = 0; x < 8; x++)
        {
          bankSwitch(0xDE00, (x * 8) + 1);  // Switch Bank 9-15
          readSegment(0x8000, 0xA000);      // 8K * 8 = +64K = 128K
        }

        ROMH_DISABLE;
        bankSwitch(0xDE00, 0x86);  // Reset ROM
        break;

      case 8: // Super Games (64K)
        for (int x = 0; x < 4; x++)
        {
          bankSwitch(0xDF00, x);  // Switch Bank
          readSegment16k();
        }
        break;

      case 10: // Epyx Fastload (8K)
        ROML_ENABLE;
        bankSwitch(0xDE00, 0);             // Read IO1 - Trigger Access
        readSegment(0x8000, 0x9E00);       // 7680 Bytes
        readSegment(0x9E00, 0x9F00, 256);  // +256 Bytes = 7936 Bytes
        bankSwitch(0xDF00, 0);             // Read IO2 - Access Last 256 Bytes
        readSegment(0x9F00, 0xA000, 256);  // +256 Bytes = 8K
        ROML_DISABLE;
        break;

      case 12: // Rex Utility (8K)
        ROML_ENABLE;
        bankSwitch(0xDFC0, 0);        // Enable ROM
        readSegment(0x8000, 0xA000);  // 8K
        ROML_DISABLE;
        break;

      case 13: // Final Cartridge I (16K)
        bankSwitch(0xDF00, 0);  // Enable ROM
        readSegment16k();
        break;

      case 14: // Magic Formel (64K)
        ROMH_ENABLE;
        for (int x = 0; x < 8; x++)
        {
          bankSwitch(0xDF00 + x, 0);     // Switch Bank using A0-A2
          readSegment(0xE000, 0x10000);  // 8K * 8 = 64K
        }
        ROMH_DISABLE;
        break;

      case 15: // C64 Game System, System 3 (512K)
        readSegmentBankA0A4(64);
        break;

      case 17: // Dinamic (128K)
        readSegmentBankA0A4(16);
        break;

      case 18: // Zaxxon, Super Zaxxon (SEGA) (20K)
        readSegmentEnableDisable(0x8000, 0x9000, 0);  // 4K
        readSegmentEnableDisable(0xA000, 0xC000, 1);  // +8K = 12K

        // Switch Bank
        readData(0x9000);
        readSegmentEnableDisable(0xA000, 0xC000, 1);  // +8K = 20K
        break;

      case 19: // Magic Desk, Domark, HES Australia (32K/64K/128K)
        romBanks = romSizes[romSize] / 8;
        readSegmentBankD0D5(romBanks, 0x8000, 0);
        break;

      case 20:  // Super Snapshot 5 (64K)
        for (int x = 0; x < 4; x++)
        {
          int bank = (((x & 2) << 3) | (0 << 3) | ((x & 1) << 2));
          bankSwitch(0xDE00, bank);  // Switch Bank using D2-D4 (D3 == 0 Enable ROM)
          readSegment16k();
        }
        break;

      case 21:  // Comal-80 (64K)
        for (int x = 0; x < 4; x++)
        {
          bankSwitch(0xDE00, x + 0x80);  // Switch Bank
          readSegment16k();
        }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    // Compare CRC32 to database and rename ROM if found
    OSCR::Databases::Standard::matchCRC();
  }

  //******************************************
  // MAPPER CODE
  //******************************************

  void setMapper()
  {
    fromCRDB = false;

    OSCR::Databases::Basic::browseMappers();
  }

  //******************************************
  // SET ROM SIZE
  //******************************************
  void setROMSize()
  {
    if (mapperDetail->sizeLow == mapperDetail->sizeHigh)
    {
      romSize = mapperDetail->sizeLow;
      return;
    }

    romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes + mapperDetail->sizeLow, mapperDetail->sizeHigh - mapperDetail->sizeLow);
  }

  //******************************************
  // CHECK STATUS
  //******************************************
  void checkStatus()
  {
    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::MAPPER));
    OSCR::UI::printLine(mapperDetail->id);

    OSCR::UI::printLine(mapperDetail->name);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(romSizes[romSize] * 1024);
  }

  //******************************************
  // CART SELECT CODE
  //******************************************

  void setCart()
  {
    OSCR::Databases::Extended::browseDatabase();

    if (!OSCR::Databases::Basic::matchMapper(romDetail->mapper))
    {
      OSCR::Databases::Basic::printMapperNotFound(romDetail->mapper);
      return;
    }

    romSize = romDetail->size;
    mapper = romDetail->mapper;

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::Commodore64 */
#endif /* HAS_C64 */
