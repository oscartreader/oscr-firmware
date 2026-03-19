//******************************************
// VECTREX MODULE
//******************************************
#include "config.h"

#if HAS_VECTREX
# include "cores/include.h"
# include "cores/Vectrex.h"

namespace OSCR::Cores::Vectrex
{
  // Vectrex
  // Cartridge Pinout
  // 36P 2.54mm pitch connector
  //
  //                RIGHT
  //              +-------+
  //         +5V -|  2  1 |- /HALT
  //         +5V -|  4  3 |- A7
  //          A8 -|  6  5 |- A6
  //          A9 -|  8  7 |- A5     B
  //    L    A11 -| 10  9 |- A4     O
  //    A    /OE -| 12 11 |- A3     T
  //    B    A10 -| 14 13 |- A2     T
  //    E    A15 -| 16 15 |- A1     O
  //    L     D7 -| 18 17 |- A0     M
  //          D6 -| 20 19 |- D0
  //    S     D5 -| 22 21 |- D1     S
  //    I     D4 -| 24 23 |- D2     I
  //    D     D3 -| 26 25 |- GND    D
  //    E    GND -| 28 27 |- GND    E
  //         R/W -| 30 29 |- A12
  //       /CART -| 32 31 |- A13
  //        /NMI -| 34 33 |- A14
  //        /IRQ -| 36 35 |- PB6
  //              +-------+
  //                LEFT
  //
  //                                                LABEL SIDE
  //
  //        /IRQ /NMI /CART R/W  GND   D3   D4   D5   D6   D7  A15  A10  /OE  A11   A9   A8  +5V  +5V
  //       +-------------------------------------------------------------------------------------------+
  //       |  36   34   32   30   28   26   24   22   20   18   16   14   12   10    8    6    4    2  |
  // LEFT  |                                                                                           | RIGHT
  //       |  35   33   31   29   27   25   23   21   19   17   15   13   11    9    7    5    3    1  |
  //       +-------------------------------------------------------------------------------------------+
  //         PB6  A14  A13  A12  GND  GND   D2   D1   D0   A0   A1   A2   A3   A4   A5   A6   A7  /HALT
  //
  //                                                BOTTOM SIDE

  // CONTROL PINS:
  // /OE(PH1)   - SNES CPUCLK
  // /CART(PH3) - SNES /CS
  // PB6(PH5)   - SNES /WR
  // R/W(PH6)   - SNES /RD

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // DEFINES
  //******************************************
  #define CLK_ENABLE PORTH |= (1 << 1)    // /E HIGH
  #define CLK_DISABLE PORTH &= ~(1 << 1)  // /E LOW
  #define PB6_ENABLE PORTH |= (1 << 5)    // PB6 HIGH
  #define PB6_DISABLE PORTH &= ~(1 << 5)  // PB6 LOW

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const romSizes[] = {
    4,
    8,
    12,
    16,
    32,
    64,
  };

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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Vectrex), menuOptions, sizeofarray(menuOptions)))
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Vectrex));
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
    // Vectrex uses A0-A15 [A16-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       ---(PH0)   /OE(PH1)  /CART(PH3)  ---(PH4)  PB6(PH5)   R/W(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       ---(PH0)   /OE(PH1)  /CART(PH3)  ---(PH4)  PB6(PH5)   R/W(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTL = 0xFF;       // A16-A23
    PORTJ |= (1 << 0);  // TIME(PJ0)

    // Set /CART LOW
    PORTH &= ~(1 << 3);  // Enable Cart

    // Set /OE LOW
    PORTH &= ~(1 << 1);  // Output Enable

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Vectrex));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ CODE
  //******************************************

  uint8_t readData(uint16_t addr) // Add Input Pullup
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    PORTC = 0xFF;  // Input Pullup
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;  // Added delay for better read
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
        OSCR::Storage::Shared::buffer[w] = readData(addr + w);
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

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Vectrex), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    PB6_DISABLE;  // PB6 LOW - Switch Bank

    // Standard Carts 4K/8K
    readSegment(0x0000, 0x1000);  // 4K

    if (romSize > 0)
    {
      readSegment(0x1000, 0x2000);  // +4K = 8K

      // 12K (Dark Tower)
      if (romSize > 1)
      {
        readSegment(0x2000, 0x3000);  // +4K = 12K
      }

      // 16K Carts
      if (romSize > 2)
      {
        readSegment(0x3000, 0x4000);  // +4K = 16K
      }

      // Oversize 32K Carts
      if (romSize > 3)
      {
        readSegment(0x4000, 0x8000);  // +16K = 32K
      }

      // Oversize 64K Carts
      if (romSize > 4)
      {
        PB6_ENABLE; // PB6 HIGH - Switch Bank
        readSegment(0x0000, 0x8000);  // +32K = 64K
        PB6_DISABLE; // Reset PB6
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
    romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes, sizeofarray(romSizes));

    printHeader();

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, romSizes[romSize] * 1024);
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
} /* namespace OSCR::Cores::Vectrex */

#endif /* HAS_VECTREX */
