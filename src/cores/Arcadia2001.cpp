//******************************************
// EMERSON ARCADIA 2001 MODULE
//******************************************
#include "config.h"

#if HAS_ARC
# include "cores/include.h"
# include "cores/Arcadia2001.h"

namespace OSCR::Cores::Arcadia2001
{
  // Emerson Arcadia 2001
  // Cartridge Pinout
  // 30P 2.54mm pitch connector
  //
  //       FRONT             BACK
  //        SIDE             SIDE
  //              +-------+
  //         GND -| 2   1 |- A13 (A12)
  //   VCC (+5V) -| 4   3 |- D3
  //          A0 -| 6   5 |- D4
  //          A1 -| 8   7 |- D5
  //          A2 -| 10  9 |- D6
  //          A3 -| 12 11 |- D7
  //          A4 -| 14 13 |- D0
  //          A5 -| 16 15 |- D2
  //          A6 -| 18 17 |- D1
  //          A7 -| 20 19 |- NC
  //          A8 -| 22 21 |- NC
  //          A9 -| 24 23 |- NC
  //         A10 -| 26 25 |- GND
  //         A11 -| 28 27 |- GND
  //   A12 (/EN) -| 30 29 |- NC
  //              +-------+
  //
  //                                    BACK
  //       +------------------------------------------------------------+
  //       | 1   3   5   7   9   11  13  15  17  19  21  23  25  27  29 |
  // LEFT  |                                                            | RIGHT
  //       | 2   4   6   8   10  12  14  16  18  20  22  24  26  28  30 |
  //       +------------------------------------------------------------+
  //                                    FRONT
  //

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  constexpr uint8_t const romSizes[] = {
    2,
    4,
    6,
    8,
  };
  constexpr uint8_t const arclo = 0;  // Lowest Entry
  constexpr uint8_t const archi = sizeofarray(romSizes) - 1;  // Highest Entry

  // EEPROM MAPPING
  // 08 ROM SIZE

  //******************************************
  //  Menu
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Arcadia2001), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        checkStatus();
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Arcadia2001));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // Arcadia 2001 uses A0-A13 [A14-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Unused Control Pins to HIGH
    //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

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
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Arcadia2001));
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
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

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

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Arcadia2001), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    readSegment(0x0000, 0x0800);  // 2K
    if (romSize > 0)
    {
      readSegment(0x0800, 0x1000);  // +2K = 4K
      if (romSize > 1)
      {
        readSegment(0x2000, 0x2800);  // +2K = 6K
        if (romSize > 2)
        {
          readSegment(0x2800, 0x3000);  // +2K = 8K
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
    if (arclo == archi) romSize = arclo;
    else romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes, sizeofarray(romSizes));
  }

  void checkStatus()
  {
    if (romSize > 3)
    {
      romSize = 0;
    }

    printHeader();
    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(romSizes[romSize] * 1024);
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
} /* namespace OSCR::Cores::Arcadia2001 */

#endif /* HAS_ARC */
