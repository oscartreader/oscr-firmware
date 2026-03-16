//******************************************
// MAGNAVOX ODYSSEY 2 MODULE
//******************************************
#include "config.h"

#if HAS_ODY2
# include "cores/include.h"
# include "cores/Odyssey2.h"

namespace OSCR::Cores::Odyssey2
{
  // Magnavox Odyssey 2
  // Philips Videopac/Videopac+
  // Cartridge Pinout
  // 30P 3.96mm pitch connector
  //
  //       FRONT            BACK
  //        SIDE            SIDE
  //              +------+
  //          T0 -| 1  A |- /WR
  //          D0 -| 2  B |- GND
  //          D1 -| 3  C |- GND
  //          D2 -| 4  D |- VCC (+5V)
  //          D3 -| 5  E |- CS3
  //          D4 -| 6  F |- /PSEN (/CE)
  //          D5 -| 7  G |- A0
  //          D6 -| 8  H |- A1
  //          D7 -| 9  J |- A2
  //   A10 (P22) -| 10 K |- A3
  //  /CS1 (P14) -| 11 L |- A4
  //         P11 -| 12 M |- A5
  //         P10 -| 13 N |- A7
  //   A11 (P23) -| 14 P |- A6
  //    A9 (P21) -| 15 R |- A8 (P20)
  //              +------+
  //
  // NOTE:  ADDRESS A7/A6 PIN ORDER ON PINS N & P.
  // NOTE:  MOST CARTS DO NOT CONNECT A10 ON PIN 10.
  //
  //                           BACK
  //      +---------------------------------------------+
  //      | A  B  C  D  E  F  G  H  J  K  L  M  N  P  R |
  // LEFT |                                             | RIGHT
  //      | 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 |
  //      +---------------------------------------------+
  //                           FRONT
  //
  // CONTROL PINS:
  // T0(PH0)    - SNES RESET
  // CS3(PH3)   - SNES /CS
  // /CS1(PH4)  - SNES /IRQ
  // /WR(PH5)   - SNES /WR
  // /PSEN(PH6) - SNES /RD

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  constexpr uint8_t const romSizes[] = {
    2,
    4,
    8,
    12,
    16,
  };

  constexpr uint8_t const romSizeHigh = sizeofarray(romSizes) - 1; // Highest Entry

  uint8_t ody2mapper;
  uint8_t ody2size;

  // EEPROM MAPPING
  // 07 MAPPER
  // 08 ROM SIZE

  //******************************************
  //  Menu
  //******************************************
  // Base Menu
  constexpr char const * const menuOptions[] PROGMEM = {
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Odyssey2), menuOptions, sizeofarray(menuOptions)))
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Odyssey2));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // Odyssey 2 uses A0-A13 [A14-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //        T0(PH0)   ---(PH1)   CS3(PH3)   /CS1(PH4)  /WR(PH5)   /RD(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //        T0(PH0)   ---(PH1)   /CS1(PH4)  /WR(PH5)   /RD(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set CS3(PH3) to LOW
    PORTH &= ~(1 << 3);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTL = 0xFF;       // A16-A23
    PORTJ |= (1 << 0);  // TIME(PJ0)

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Odyssey2));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ CODE
  //******************************************

  uint8_t readData(uint16_t addr)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A13

    // Set /PSEN (/CE) to LOW
    PORTH &= ~(1 << 6);  // /PSEN LOW (ENABLE)
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

    // Pull /PSEN (/CE) to HIGH
    PORTH |= (1 << 6);  // /PSEN HIGH (DISABLE)

    return ret;
  }

  void readSegment(uint16_t startaddr, uint16_t endaddr)
  {
    for (uint16_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (int w = 0; w < 512; w++)
      {
        uint8_t temp = readData(addr + w);
        OSCR::Storage::Shared::buffer[w] = temp;
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  void bankSwitch(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A13
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // Set /CS1(PH4) to LOW
    PORTH &= ~(1 << 4);
    // Set /WR(PH5) to LOW
    PORTH &= ~(1 << 5);
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    DDRC = 0xFF;  // Set to Output
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    PORTC = data;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // Set /WR(PH5) to HIGH
    PORTH |= (1 << 5);
    // Set /CS1(PH4) to HIGH
    PORTH |= (1 << 4);

    DDRC = 0x00;  // Reset to Input
  }

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Odyssey2), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::Raw));

    cartOn();

    if (ody2mapper == 1) // A10 CONNECTED
    {
      // Videopac 31:  Musician
      // Videopac 40:  4 in 1 Row/4 en 1 Ligne
      readSegment(0x0000, 0x1000);  // 4K
    }
    else if (ody2size > 2) // A10 NOT CONNECTED
    {
      // 12K/16K (BANKSWITCH)
      // Videopac+ 55:  Neutron Star         12K = 2K x 6 Banks
      // Videopac+ 58:  Norseman             12K = 2K x 6 Banks
      // Videopac+ 59:  Helicopter Rescue    16K = 2K x 8 Banks
      // Videopac+ 60:  Trans American Rally 16K = 2K x 8 Banks
      uint8_t ody2banks = (ody2size * 4) / 2;

      for (int x = (ody2banks - 1); x >= 0; x--)
      {
        bankSwitch(0x80, ~x);
        readSegment(0x0400, 0x0C00);  // 2K x 6/8 = 12K/16K
      }
    }
    else // STANDARD SIZES
    {
      readSegment(0x0400, 0x0C00);  // 2K
      if (ody2size > 0)
      {
        readSegment(0x1400, 0x1C00);  // +2K = 4K

        if (ody2size > 1)
        {
          readSegment(0x2400, 0x2C00);  // +2K = 6K
          readSegment(0x3400, 0x3C00);  // +2K = 8K
        }
      }
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
    fromCRDB = false;

    ody2size = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeK), romSizes, sizeofarray(romSizes));

    ody2mapper = 0;

    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::MAPPER));
    OSCR::UI::printLine(ody2mapper);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(romSizes[ody2size] * 1024);
  }

  //******************************************
  // CART SELECT CODE
  //******************************************
  void setCart()
  {
    OSCR::Databases::Standard::browseDatabase();

    romSize = romDetail->size;
    ody2mapper = romDetail->mapper;

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::Odyssey2 */

#endif /* HAS_ODY2 */
