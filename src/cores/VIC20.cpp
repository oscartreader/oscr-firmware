//******************************************
// COMMODORE VIC-20 MODULE
//******************************************
#include "config.h"

#if HAS_VIC20
# include "cores/include.h"
# include "cores/VIC20.h"

namespace OSCR::Cores::VIC20
{
  // Commodore VIC-20
  // Cartridge Pinout
  // 44P 3.96mm pitch connector
  //
  //   FRONT             BACK
  //    SIDE             SIDE
  //          +-------+
  //     GND -|  1  A |- GND
  //      D0 -|  2  B |- A0
  //      D1 -|  3  C |- A1
  //      D2 -|  4  D |- A2
  //      D3 -|  5  E |- A3
  //      D4 -|  6  F |- A4
  //      D5 -|  7  H |- A5
  //      D6 -|  8  J |- A6
  //      D7 -|  9  K |- A7
  //   /BLK1 -| 10  L |- A8
  //   /BLK2 -| 11  M |- A9
  //   /BLK3 -| 12  N |- A10
  //   /BLK5 -| 13  P |- A11
  //   /RAM1 -| 14  R |- A12
  //   /RAM2 -| 15  S |- A13
  //   /RAM3 -| 16  T |- IO2
  //    VR/W -| 17  U |- IO3
  //    CR/W -| 18  V |- PHI2
  //     IRQ -| 19  W |- /NMI
  //      NC -| 20  X |- /RESET
  //     +5V -| 21  Y |- NC
  //     GND -| 22  Z |- GND
  //          +-------+
  //
  //                                              BACK
  //       GND NC /RST /NMI PH2 I3  I2 A13 A12 A11 A10  A9  A8 A7 A6 A5 A4 A3 A2 A1 A0 GND
  //      +-------------------------------------------------------------------------------+
  //      |  Z   Y   X   W   V   U   T   S   R   P   N   M   L  K  J  H  F  E  D  C  B  A |
  // LEFT |                                                                               | RIGHT
  //      | 22  21  20  19  18  17  16  15  14  13  12  11  10  9  8  7  6  5  4  3  2  1 |
  //      +-------------------------------------------------------------------------------+
  //       GND +5V  NC IRQ CRW VRW /R3 /R2 /R1 /B5 /B3 /B2 /B1 D7 D6 D5 D4 D3 D2 D1 D0 GND
  //                                              FRONT

  // CONTROL PINS:
  // /BLK1(PH3) - SNES /CS  - [$2000-$3FFF]
  // /BLK2(PH4) - SNES /IRQ - [$4000-$5FFF]
  // /BLK3(PH5) - SNES /WR  - [$6000-$7FFF]
  // /BLK5(PH6) - SNES /RD  - [$A000-$BFFF]

  using OSCR::Databases::Extended::crdbRecord;
  using OSCR::Databases::Extended::crdb;
  using OSCR::Databases::Extended::romDetail;
  using OSCR::Databases::Extended::romRecord;

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const VIC20MAP[] = {
    0x20, // 0x2000
    0x24, // 0x2000/0x4000
    0x2A, // 0x2000/0xA000
    0x46, // 0x4000/0x6000 - Adventure Games
    0x60, // 0x6000
    0x6A, // 0x6000/0xA000 - Standard 16K
    0x70, // 0x7000
    0xA0, // 0xA000        - Standard 8K
    0xB0  // 0xB000
  };

  constexpr uint8_t const vic20mapcount = sizeofarray(VIC20MAP);
  constexpr uint8_t const vic20maplo = 0; // Lowest Entry
  constexpr uint8_t const vic20maphi = vic20mapcount - 1; // Highest Entry
  uint8_t vic20mapselect;
  uint8_t vic20map = 0;
  uint8_t newvic20map;

  constexpr uint8_t const VIC20SIZE[] = {
    0x20, // 2K/0K 0x800
    0x40, // 4K/0K 0x1000
    0x80, // 8K/0K 0x2000
    0x44, // 4K/4K 0x1000/0x1000
    0x48, // 4K/8K 0x1000/0x2000
    0x84, // 8K/4K 0x2000/0x1000
    0x88  // 8K/8K 0x2000/0x2000
  };

  constexpr uint8_t const vic20lo = 0; // Lowest Entry
  constexpr uint8_t const vic20hi = sizeofarray(VIC20SIZE) - 1; // Highest Entry

  // EEPROM MAPPING
  // 07 MAPPER
  // 08 ROM SIZE

  //******************************************
  // MENU
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::VIC20), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        break;

      case 1: // Read ROM
        readROM();
        break;

      case 2: // Set ROM Map + Size
        setROMMap();
        setROMSize();
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::VIC20));
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
    // VIC-20 uses A0-A13 [A14-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //      /RST(PH0)   ---(PH1)  /BLK1(PH3) /BLK2(PH4) /BLK3(PH5) /BLK5(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //      /RST(PH0)   ---(PH1)  /BLK1(PH3) /BLK2(PH4) /BLK3(PH5) /BLK5(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTL = 0xFF; // A16-A23
    PORTJ |= (1 << 0); // TIME(PJ0)

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Extended::setup(F("vic20"));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ FUNCTIONS
  //******************************************

  uint8_t readData(uint16_t addr)
  {
    PORTF = addr & 0xFF;        // A0-A7
    PORTK = (addr >> 8) & 0xFF; // A8-A13
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

    return ret;
  }

  void readSegment(uint32_t startaddr, uint32_t endaddr)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (int w = 0; w < 512; w++)
      {
        uint8_t temp = readData(addr + w);

        OSCR::Storage::Shared::buffer[w] = temp;
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    uint8_t rommap = 0;
    uint8_t romsize = 0;

    // Split into Individual ROM Files
    for (int x = 0; x < 2; x++) // ROM0/ROM1
    {
      if (x == 1)
      {
        if ((VIC20MAP[vic20map] & 0x0F) == 0)
          break;

        rommap = ((VIC20MAP[vic20map] & 0x0F) << 4);
        romsize = VIC20SIZE[romSize] & 0x0F;
      }
      else
      {
        rommap = VIC20MAP[vic20map] & 0xF0;
        romsize = ((VIC20SIZE[romSize] & 0xF0) >> 4);
      }

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::VIC20), FS(OSCR::Strings::Directory::ROM), fileName, rommap);

      if (rommap == 0x20) // BLK1
      {
        PORTH &= ~(1 << 3); // BLK1(PH3) LOW
        readSegment(0x2000, 0x3000); // 4K
        if (romsize == 8)
          readSegment(0x3000, 0x4000); // +4K = 8K
        PORTH |= (1 << 3); // BLK1(PH3) HIGH
      }
      else if (rommap == 0x40) // BLK2
      {
        PORTH &= ~(1 << 4); // BLK2(PH4) LOW
        readSegment(0x4000, 0x5000); // 4K
        if (romsize == 8)
          readSegment(0x5000, 0x6000); // +4K = 8K
        PORTH |= (1 << 4); // BLK2(PH4) HIGH
      }
      else if (rommap == 0x60) // BLK3
      {
        PORTH &= ~(1 << 5); // BLK3(PH5) LOW
        readSegment(0x6000, 0x7000); // 4K
        if (romsize == 8)
          readSegment(0x7000, 0x8000); // +4K = 8K
        PORTH |= (1 << 5); // BLK3(PH5) HIGH
      }
      else if (rommap == 0x70) // BLK3 UPPER HALF
      {
        PORTH &= ~(1 << 5); // BLK3(PH5) LOW
        readSegment(0x7000, 0x8000);
        PORTH |= (1 << 5); // BLK3(PH5) HIGH
      }
      else if (rommap == 0xA0) // BLK5
      {
        PORTH &= ~(1 << 6); // BLK5(PH6) LOW

        readSegment(0xA000, 0xA800); // 2K

        if (romsize > 2)
        {
          readSegment(0xA800, 0xB000); // +2K = 4K
          if (romsize > 4)
            readSegment(0xB000, 0xC000); // +4K = 8K
        }

        PORTH |= (1 << 6); // BLK5(PH6) HIGH
      }
      else if (rommap == 0xB0) // BLK5 UPPER HALF
      {
        PORTH &= ~(1 << 6); // BLK5(PH6) LOW

        readSegment(0xB000, 0xB800); // 2K

        if (romsize > 2)
          readSegment(0xB800, 0xC000); // +2K = 4K

        PORTH |= (1 << 6); // BLK5(PH6) HIGH
      }

      OSCR::Storage::Shared::close();

      OSCR::UI::print(F("ROM"));
      OSCR::UI::print(x);
      OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));

      // Compare CRC32 to database and rename ROM if found
      OSCR::Databases::Extended::matchCRC();
    }

    cartOff();
  }

  //******************************************
  // ROM SIZE
  //******************************************

  void setROMSize()
  {
    if (vic20lo == vic20hi)
    {
      romSize = vic20lo;
      return;
    }

    romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), VIC20SIZE, sizeofarray(VIC20SIZE));

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(((VIC20SIZE[romSize] & 0xF0) >> 4) * 1024);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine((VIC20SIZE[romSize] & 0x0F) * 1024);

    OSCR::UI::update();

    delay(1000);
  }

  void checkStatus()
  {
    if (vic20map > 8)
    {
      vic20map = 7; // default 0xA000
    }

    if (romSize > vic20hi)
    {
      romSize = 2; // default 8K
    }

    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::MAPPER));
    OSCR::UI::printHexLine(VIC20MAP[vic20map]);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(((VIC20SIZE[romSize] & 0xF0) >> 4) * 1024);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine((VIC20SIZE[romSize] & 0x0F) * 1024);
  }

  //******************************************
  // SET ROM MAP
  //******************************************

  void setROMMap()
  {
    vic20map = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), VIC20MAP, sizeofarray(VIC20MAP) - 1);
    fromCRDB = false;
  }

  //******************************************
  // CART SELECT CODE
  //******************************************

  void setCart()
  {
    OSCR::Databases::Extended::browseDatabase();

    romSize = romDetail->size;
    vic20map = romDetail->mapper;

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::VIC20 */

#endif /* HAS_VIC20 */
