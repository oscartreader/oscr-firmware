//******************************************
// TEXAS INSTRUMENTS TI-99 MODULE
//******************************************
#include "config.h"

#if HAS_TI99
# include "cores/include.h"
# include "cores/TI99.h"

namespace OSCR::Cores::TI99
{
  // Texas Instruments TI-99
  // Cartridge Pinout
  // 36P 2.54mm pitch connector
  //
  //                   RIGHT
  //                 +-------+
  //            Vss -| 36 35 |- GND
  //          ROMS* -| 34 33 |- Vss
  //            WE* -| 32 31 |- GR
  //             A4 -| 30 29 |- -5V     B
  //             A5 -| 28 27 |- GRC     O
  //    T        A6 -| 26 25 |- DBIN    T
  //    O        A3 -| 24 23 |- A14     T
  //    P        A7 -| 22 21 |- GS*     O
  //             A8 -| 20 19 |- +5V     M
  //    S        A9 -| 18 17 |- D0
  //    I       A10 -| 16 15 |- D1      S
  //    D       A11 -| 14 13 |- D2      I
  //    E       A12 -| 12 11 |- D3      D
  //            A13 -| 10 9  |- D4      E
  //        A15/OUT -|  8 7  |- D5
  //          CRUIN -|  6 5  |- D6
  //        CRUCLK* -|  4 3  |- D7
  //            GND -|  2 1  |- RESET
  //                 +-------+
  //                   LEFT
  //
  //                                         TOP SIDE
  //
  //             CRU CRU A15/
  //         GND CLK* IN OUT A13 A12 A11 A10  A9  A8  A7  A3  A6  A5  A4 WE* ROMS* VSS
  //       +--------------------------------------------------------------------------+
  //       |   2   4   6   8  10  12  14  16  18  20  22  24  26  28  30  32  34  36  |
  // LEFT  |                                                                          | RIGHT
  //       |   1   3   5   7   9  11  13  15  17  19  21  23  25  27  29  31  33  35  |
  //       +--------------------------------------------------------------------------+
  //         RST  D7  D6  D5  D4  D3  D2  D1  D0 +5V GS* A14 DBIN GRC -5V GR VSS GND
  //
  //                                        BOTTOM SIDE

  // CONTROL PINS:
  // RESET(PH0) - SNES RESET
  // GRC(PH1)   - SNES CPUCLK [GROM CLOCK]
  // /GS(PH3)   - SNES /CS    [GROM SELECT]
  // DBIN(PH4)  - SNES /IRQ   [READ MEMORY/M(GROM DIRECTION)]
  // /WE(PH5)   - SNES /WR
  // /ROMS(PH6) - SNES /RD    [ROM SELECT]
  // GR(PL0)    - SNES A16    [GROM READY]

  // /GS = LOW FOR ADDR $9800-$9FFF
  // /ROMS = LOW FOR ADDR $6000-$7FFF

  // NOTE: TI-99 ADDRESS AND DATA BUS ARE BIG-ENDIAN
  // LEAST SIGNIFICANT IS BIT 7 AND MOST SIGNIFICANT IS BIT 0
  // PCB ADAPTER WIRED FOR DIFFERENCE

  // FOR GROM-ONLY BOARDS, TOP PINS DO NOT EXIST
  // DATA PINS ARE MULTIPLEXED TO HANDLE BOTH ADDRESS AND DATA
  // D7-D0 = AD7-AD0

  using OSCR::Databases::Basic::mapperDetail;

  //******************************************
  // DEFINES
  //******************************************
  #define DISABLE_GROM  PORTH |= (1 << 3) // GROM SELECT 9800-9FFF
  #define ENABLE_GROM   PORTH &= ~(1 << 3)
  #define DISABLE_ROM PORTH |= (1 << 6) // ROM SELECT 6000-7FFF
  #define ENABLE_ROM  PORTH &= ~(1 << 6)

  #define GROM_WRITE PORTH &= ~(1 << 4) // DBIN/M LOW
  #define GROM_READ  PORTH |= (1 << 4) // DBIN/M HIGH
  #define GROM_DATA PORTF &= ~(1 << 1) // A14/MO LOW
  #define GROM_ADDR PORTF |= (1 << 1); // A14/MO HIGH

  #define GRC_HI PORTH |= (1 << 1)
  #define GRC_LOW PORTH &= ~(1 << 1)

  //******************************************
  // VARIABLES
  //******************************************
  // Cart Configurations
  // Format = {mapper,gromlo,gromhi,romlo,romhi}
  constexpr uint8_t const PROGMEM ti99mapsize[] = {
    0, 5, 0, 4, // 0: Normal Carts (GROM 0K/6K/12K/18K/24K/30K + ROM 0K/4K/8K/12K/16K)
    1, 3, 4, 4, // 1: MBX (GROM 6K/12K/18K + ROM 16K)
    1, 1, 4, 4, // 2: TI-CALC (GROM 6K + ROM 16K)
  };

  constexpr uint8_t const ti99mapcount = sizeofarray(ti99mapsize) / 4;

  // Mappers
  constexpr char const PROGMEM MAPPER_00[] = "NORMAL";
  constexpr char const PROGMEM MAPPER_01[] = "MBX";
  constexpr char const PROGMEM MAPPER_02[] = "TI-CALC";

  constexpr char const * const PROGMEM menuOptionsMapper[] = {
    MAPPER_00,
    MAPPER_01,
    MAPPER_02,
  };

  uint8_t romSizes[] = {
    0,
    4,
    8,
    12,
    16,
  };

  uint8_t romSizeLow = 0; // Lowest Entry
  uint8_t romSizeHigh = sizeofarray(romSizes) - 1; // Highest Entry

  uint8_t gromSizes[] = {
    0,
    6,
    12,
    18,
    24,
    30,
  };

  uint8_t gromSizeLow = 0; // Highest Entry
  uint8_t gromSizeHigh = sizeofarray(gromSizes) - 1; // Highest Entry

  uint8_t gromSize = 0;
  uint8_t gromMap;

  //bool MBX = 0; // Normal/MBX

  // EEPROM MAPPING
  // 07 MAPPER
  // 08 GROM SIZE
  // 09 CROM SIZE
  // 13 GROM MAP

  //******************************************
  // MENU
  //******************************************
  // Base Menu
  constexpr char const PROGMEM ti99MenuItem2[] = "Read Complete Cart";
  constexpr char const PROGMEM ti99MenuItem3[] = "Read GROM";
  constexpr char const PROGMEM ti99MenuItem6[] = "Set GROM Map";
  constexpr char const* const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::SelectCart,
    ti99MenuItem2,
    ti99MenuItem3,
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::SetSize,
    ti99MenuItem6,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::TI99), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        checkStatus();
        break;

      case 1: // Read Complete Cart
        readGROM();
        readCROM();
        break;

      case 2: // Read GROM
        readGROM();
        break;

      case 3: // Read ROM
        readCROM();
        break;

      case 4: // Set Mapper + Sizes
        setMapper();
        checkStatus();
        break;

      case 5: // Set GROM Map
        gromMenu();
        break;

      case 6: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void gromMenu()
  {
    uint8_t gromSelection = OSCR::UI::rangeSelect(FS(OSCR::Strings::Cores::TI99), 3, 7);
    uint8_t gromBitflag = (1 << gromSelection);

    if (gromMap & gromBitflag) gromMap ^= gromBitflag;
    else gromMap |= gromBitflag;
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::TI99));
  }

  //******************************************
  // SETUP
  //******************************************

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // TI-99 uses A0-A15
    // SNES A0-A7 [TI-99 A15-A8]
    DDRF = 0xFF;
    // SNES A8-A15 [TI-99 A7-A0]
    DDRK = 0xFF;
    // SNES A16-A23 - Use A16 for GR
    DDRL = 0xFE; // A16 to Input

    // Set Control Pins to Output
    //       RST(PH0)   GRC(PH1)   /GS(PH3)   DBIN(PH4)  /WE(PH5)  /ROMS(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       RST(PH0)   GRC(PH1)   /GS(PH3)   DBIN(PH4)  /WE(PH5)  /ROMS(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused PORTL Pins HIGH except GR(PL0)
    PORTL |= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTJ |= (1 << 0); // TIME(PJ0)

    // Set Reset LOW
    PORTH &= ~(1 << 0);

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    cartCRDB = new OSCR::Databases::TI99(FS(OSCR::Strings::FileType::TI99));
    OSCR::Databases::Basic::setupMapper(F("ti99-mappers"));
  }

  void closeCRDB()
  {
    if (cartCRDB == nullptr) return;
    delete ti99CRDB;
    resetGlobals();
  }

  //******************************************
  // READ FUNCTIONS
  //******************************************

  uint8_t readROM(uint16_t addr) // Add Input Pullup
  {
    PORTF = addr & 0xFF;        // A0-A7
    PORTK = (addr >> 8) & 0xFF; // A8-A15
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // DDRC = 0x00; // Set to Input
    PORTC = 0xFF; // Input Pullup
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    return ret;
  }

  void readSegment(uint16_t startaddr, uint16_t endaddr)
  {
    for (uint16_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (int w = 0; w < 512; w++)
      {
        uint8_t temp = readROM(addr + w);
        OSCR::Storage::Shared::buffer[w] = temp;
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  void setupGROM()
  {
    // Setup GROM
    DISABLE_GROM;
    GROM_READ; // DBIN(M) HIGH = READ
    GROM_ADDR; // A14(MO) HIGH = ADDR
    GROM_DATA; // A14(MO) LOW = DATA
  }

  void pulseGRC(int times)
  {
    for (int i = 0; i < (times * 2); i++)
    {
      PORTH ^= (1 << 1);

      // NOP (62.5ns) x 20 = 1250ns = 1.25us
      //    NOP; NOP; NOP; NOP; NOP; // 20 NOPs = 20 x 62.5ns = 1250ns x 2 = 2.5us = 400 kHz
      //    NOP; NOP; NOP; NOP; NOP;
      //    NOP; NOP; NOP; NOP; NOP;
      //    NOP; NOP; NOP; NOP; NOP;
      // Switch to 100 kHz due to some GROMs not reading out properly at faster speeds
      delayMicroseconds(5); // 5us x 2 = 10us = 100 kHz
    }
  }

  void checkGRC()
  {
    do
    {
      pulseGRC(1);
    }
    while (!(PINL & 0x1));
  }

  void pulseGROM(uint16_t addr) // Controlled Pulse Code
  {
    // GRC starts here
    GRC_LOW; // START LOW
    pulseGRC(16);
    ENABLE_GROM;
    pulseGRC(8);
    DISABLE_GROM;

    // Write Base
    GROM_WRITE; // DBIN(M) LOW = WRITE
    GROM_ADDR; // A14(MO) HIGH = ADDR
    DDRC = 0xFF; // Output
    PORTC = (addr >> 8) & 0xFF;
    pulseGRC(16);
    ENABLE_GROM;
    pulseGRC(4); // this works
    //  checkGRC(); // Disable checkGRC() otherwise Q-bert hangs (GROM 7 at 0xE000 with NO GROMs at 0x6000/0x8000/0xA000/0xC000)
    pulseGRC(2); // Replace checkGRC() with 2 pulse cycles
    pulseGRC(10);
    DISABLE_GROM;
    pulseGRC(16);
    PORTC = addr & 0xFF;
    pulseGRC(16);
    ENABLE_GROM;
    pulseGRC(4); // this works
    //  checkGRC(); // Disable checkGRC() otherwise Q-bert hangs (GROM 7 at 0xE000 with NO GROMs at 0x6000/0x8000/0xA000/0xC000)
    pulseGRC(2); // Replace checkGRC() with 2 pulse cycles
    pulseGRC(10);
    DISABLE_GROM;
    pulseGRC(16);
    GROM_READ; // DBIN(M) HIGH = READ
    GROM_DATA; // A14(MO) LOW = DATA
    DDRC = 0x00;
    pulseGRC(16);
  }

  void readSegmentGROM(uint32_t startaddr, uint32_t endaddr)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += 512)
    {
      pulseGROM(addr);

      for (int y = 0; y < 512; y++)
      {
        ENABLE_GROM;

        pulseGRC(2);

        OSCR::Storage::Shared::buffer[y] = PINC;

        pulseGRC(6);

        DISABLE_GROM;

        pulseGRC(16);
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  //******************************************
  // WRITE FUNCTION
  //******************************************

  void writeData(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;        // A0-A7
    PORTK = (addr >> 8) & 0xFF; // A8-A15
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    DDRC = 0xFF; // Set to Output
    PORTC = data;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // Set /WE(PH5) to LOW
    PORTH &= ~(1 << 5); // /WE LOW
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // Set /WE(PH5) to HIGH
    PORTH |= (1 << 5);
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    DDRC = 0x00; // Reset to Input
  }

  //******************************************
  // READ ROM
  //******************************************

  // CARTRIDGE GROMS 3-7 (GROMS 0-2 ARE IN THE CONSOLE)
  // GROM 3 = $6000-$77FF
  // GROM 4 = $8000-$97FF
  // GROM 5 = $A000-$B7FF
  // GROM 6 = $C000-$D7FF
  // GROM 7 = $E000-$F7FF

  // GROM ACCESS DETAILS
  // INTERNAL POINTER AUTO INCREMENTED EACH TIME CHIP ACCESSED
  // SET VALUE OF POINTER BY PASSING TWO BYTES TO THE CHIP

  void readGROM()
  {
    if (gromSize == 0) return;

    // Split into Individual GROM Files
    for (uint8_t x = 3; x < 8; x++)
    {
      if (((gromMap >> x) & 0x1) == 1)
      {
        printHeader();

        OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::TI99), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? ti99CRDB->record()->data()->name : fileName), x);

        cartOn();

        OSCR::UI::print(F("Reading GROM "));
        OSCR::UI::printLineSync(x);

        // Normal GROM 6KB/12KB/18KB/24KB/30K
        // MBX GROM 6KB/12KB/18KB
        DISABLE_ROM;
        ENABLE_GROM;

        setupGROM();

        switch (x)
        {
        case 3:
          readSegmentGROM(0x6000, 0x7800); // 6K
          break;

        case 4:
          readSegmentGROM(0x8000, 0x9800); // +6K = 12K
          break;

        case 5:
          readSegmentGROM(0xA000, 0xB800); // +6K = 18K
          break;

        case 6:
          readSegmentGROM(0xC000, 0xD800); // +6K = 24K
          break;

        case 7:
          readSegmentGROM(0xE000, 0xF800); // +6K = 30K
          break;

        default:
          break;
        }

        DISABLE_GROM;

        cartOff();

        OSCR::Storage::Shared::close();

        OSCR::Databases::Standard::matchCRC();
      }
    }
  }

  // CARTRIDGE ROM
  // EACH CART UP TO 8K MAPPED AT $6000-$7FFF

  void readCROM()
  {
    if (romSize == 0) return;

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::TI99), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? ti99CRDB->record()->data()->name : fileName), FS(OSCR::Strings::FileType::Raw));

    cartOn();

    if (mapperDetail->id == 1) // MBX
    {
      // MBX Cart ROM 16K
      // Fixed Bank 0 at 0x6000 (RAM at 0x6C00)
      // Bankswitch 0x7000-0x7FFF using write 1/2/3 to 0x6FFE
      DISABLE_GROM;
      ENABLE_ROM;

      readSegment(0x6000, 0x6C00); // 3K

      for (int w = 0; w < 1024; w += 512) // 1K RAM at 0x6C00
      {
        for (int x = 0; x < 512; x++)
        {
          OSCR::Storage::Shared::buffer[x] = 0xFF; // Replace Random RAM Contents with 0xFF
        }

        OSCR::Storage::Shared::writeBuffer();
      }

      for (int y = 1; y < 4; y++)
      {
        writeData(0x6FFE, y); // Set Bank 1/2/3
        readSegment(0x7000, 0x8000); // 4K x 3 = +12K = 16K
      }
    }
    else if (mapperDetail->id == 2) // TI-CALC [UNTESTED]
    {
      // TI-CALC ROM 16K
      // Fixed Bank 0 at 0x6000-0x6FFF
      // Bankswitch 0x7000-0x7FFF using write to 0x7000/0x7002/0x7004/0x7006
      DISABLE_GROM;
      ENABLE_ROM;
      writeData(0x7000, 0x00); // Set Bank 0
      readSegment(0x7000, 0x8000); // 4K
      writeData(0x7002, 0x00); // Set Bank 1
      readSegment(0x7000, 0x8000); // +4K = 8K
      writeData(0x7004, 0x00); // Set Bank 2
      readSegment(0x7000, 0x8000); // +4K = 12K
      writeData(0x7006, 0x00); // Set Bank 3
      readSegment(0x7000, 0x8000); // +4K = 16K
    }
    else
    {
      // Normal Cart ROM 4K/8K/12K/16K
      // ROM Space 0x6000-0x7FFF
      // Bankswitch 0x7000-0x7FFF using write to 0x6000/0x6002
      DISABLE_GROM;
      ENABLE_ROM;

      writeData(0x6000, 0x00); // Set Bank 0
      readSegment(0x6000, 0x7000); // 4K

      if (romSize > 1)
      {
        readSegment(0x7000, 0x8000); // +4K = 8K

        if (romSize > 2)
        {
          writeData(0x6002, 0x00); // Set Bank 1

          if (romSize > 3) // 16K
            readSegment(0x6000, 0x7000); // +4K = 16K

          readSegment(0x7000, 0x8000); // +4K = 12K
        }
      }
    }

    DISABLE_ROM;

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();
  }

  //******************************************
  // CHECK STATUS
  //******************************************

  void checkStatus()
  {
    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::MAPPER));
    OSCR::UI::printLine(mapperDetail->name);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(romSizes[romSize] * 1024);

    OSCR::UI::print(F("G"));
    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(gromSizes[gromSize] * 1024);

    for (uint8_t i = 3; i < 8; i++)
    {
      bool enabled = !!(gromMap & (1 >> i));
      char buff[7] = {};

      snprintf_P(BUFFN(buff), PSTR(" %2" PRIu8 "[%S]"), (enabled ? OSCR::Strings::Symbol::Asterisk : OSCR::Strings::Symbol::Space));

      if (i != 5)
      {
        OSCR::UI::print(buff);
        continue;
      }

      OSCR::UI::printLine(buff);
    }

    OSCR::UI::waitButton();
  }

  //******************************************
  // MAPPER CODE
  //******************************************
  void setMapper()
  {
    OSCR::Databases::Basic::browseMappers();

    if (
      mapperDetail->sizeLow > romSizeHigh ||
      mapperDetail->sizeHigh > romSizeHigh ||
      mapperDetail->meta1 > gromSizeHigh ||
      mapperDetail->meta2 > gromSizeHigh ||
      mapperDetail->sizeLow > mapperDetail->sizeHigh
    )
    {
      OSCR::UI::fatalErrorInvalidData();
    }

    if (mapperDetail->sizeLow == mapperDetail->sizeHigh)
    {
      romSize = mapperDetail->sizeLow;
    }
    else
    {
      romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), gromSizes + mapperDetail->sizeLow, mapperDetail->sizeHigh - mapperDetail->sizeLow);
    }

    if (mapperDetail->meta1 == mapperDetail->meta2)
    {
      gromSize = mapperDetail->meta1;
    }
    else
    {
      gromSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes + mapperDetail->meta1, mapperDetail->meta2 - mapperDetail->meta1);
    }
  }

  void setCart()
  {
    // ti99CRDB->record()->data()

    OSCR::crdbBrowser(FS(OSCR::Strings::Headings::SelectCRDBEntry), ti99CRDB);

    if (!OSCR::Databases::Basic::matchMapper(ti99CRDB->record()->data()->mapper))
    {
      OSCR::Databases::Basic::printMapperNotFound(ti99CRDB->record()->data()->mapper);
      return;
    }

    romSize = ti99CRDB->record()->data()->size;
    gromSize = ti99CRDB->record()->data()->gsize;

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::TI99 */
#endif /* HAS_TI99 */
