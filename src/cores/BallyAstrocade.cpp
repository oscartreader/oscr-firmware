//******************************************
// BALLY ASTROCADE MODULE
//******************************************
#include "config.h"

#if HAS_BALLY
# include "cores/include.h"
# include "cores/BallyAstrocade.h"

namespace OSCR::Cores::BallyAstrocade
{
  // Bally Astrocade
  // Cartridge Pinout
  // 26P 2.54mm pitch connector
  //
  //    BOTTOM
  //            +-------+
  //       GND -| 1     |
  //        A7 -| 2     |
  //        A6 -| 3     |
  //        A5 -| 4     |
  //        A4 -| 5     |
  //        A3 -| 6     |
  //        A2 -| 7     |
  //        A1 -| 8     |
  //        A0 -| 9     |
  //        D0 -| 10    |
  //        D1 -| 11    |
  //        D2 -| 12    |
  //       GND -| 13    |
  //        D3 -| 14    |
  //        D4 -| 15    |
  //        D5 -| 16    |
  //        D6 -| 17    |
  //        D7 -| 18    |
  //       A11 -| 19    |
  //       A10 -| 20    |
  //   /ENABLE -| 21    |
  //       A12 -| 22    |
  //        A9 -| 23    |
  //        A8 -| 24    |
  //  VCC(+5V) -| 25    |
  //       GND -| 26    |
  //            +-------+
  //
  //                                                         TOP SIDE
  //       +-----------------------------------------------------------------------------------------------------------+
  // LEFT  |                                                                                                           | RIGHT
  //       |   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18   19   20  21  22  23  24  25  26 |
  //       +-----------------------------------------------------------------------------------------------------------+
  //         GND  A7  A6  A5  A4  A3  A2  A1  A0  D0  D1  D2 GND  D3  D4  D5  D6  D7  A11  A10 /EN A12  A9  A8 +5V GND
  //
  //                                                        BOTTOM SIDE

  // CONTROL PINS:
  // /ENABLE(PH3) - SNES /CS

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // VARIABLES
  //******************************************

  constexpr uint8_t const BALLY[] = {
    2,
    4,
    8,
  };

  constexpr uint8_t const ballylo = 0; // Lowest Entry
  constexpr uint8_t const ballyhi = sizeofarray(BALLY) - 1; // Highest Entry

  // EEPROM MAPPING
  // 08 ROM SIZE

  //******************************************
  // MENU
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
    romSize = 0;

    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::BallyAstrocade), menuOptions, sizeofarray(menuOptions)))
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

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::BallyAstrocade));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::BallyAstrocade));
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
    // Bally Astrocade uses A0-A12 [A13-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       ---(PH0)   ---(PH1) /ENABLE(PH3) ---(PH4)   ---(PH5)   ---(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       ---(PH0)   ---(PH1) /ENABLE(PH3) ---(PH4)   ---(PH5)   ---(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTL = 0xFF; // A16-A23
    PORTJ |= (1 << 0); // TIME(PJ0)

    checkStatus();
    useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  //******************************************
  // READ FUNCTIONS
  //******************************************

  uint8_t readData(uint16_t addr)
  {
    PORTF = addr & 0xFF;        // A0-A7
    PORTK = (addr >> 8) & 0xFF; // A8-A15

    PORTC = 0xFF; // Input Pullup
    PORTH &= ~(1 << 3); // /ENABLE LOW
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;
    PORTH |= (1 << 3); // /ENABLE HIGH

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

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::BallyAstrocade), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    readSegment(0x0000, 0x0800); // 2K

    if (romSize > 0)
    {
      readSegment(0x0800, 0x1000); // +2K = 4K

      if (romSize > 1)
      {
        readSegment(0x1000, 0x2000); // +4K = 8K
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();

    OSCR::UI::waitButton();
  }

  //******************************************
  // ROM SIZE
  //******************************************
  void setROMSize()
  {
    if (ballylo == ballyhi)
    {
      romSize = ballylo;
      return;
    }

    romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), BALLY, sizeofarray(BALLY));
  }

  void checkStatus()
  {
    if (romSize > ballyhi)
    {
      romSize = 0; // default 2K
    }

    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(BALLY[romSize] * 1024);

    OSCR::UI::waitButton();
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
} /* namespace OSCR::Cores::BallyAstrocade */

#endif /* HAS_BALLY */
