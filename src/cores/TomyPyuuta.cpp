//******************************************
// TOMY PYUUTA MODULE
//******************************************
#include "config.h"

#if HAS_PYUUTA
# include "cores/include.h"
# include "cores/TomyPyuuta.h"

namespace OSCR::Cores::TomyPyuuta
{
  // Tomy Pyuuta
  // Cartridge Pinout
  // 36P 2.54mm pitch connector
  //
  //       FRONT              BACK
  //        SIDE              SIDE
  //              +--------+
  //         GND -|  2   1 |- GND
  //      /RESET -|  4   3 |- D7
  //        J1-6 -|  6   5 |- D6
  //  A15/CRUOUT -|  8   7 |- D5
  //         A13 -| 10   9 |- D4
  //         A12 -| 12  11 |- D3
  //         A11 -| 14  13 |- D2
  //         A10 -| 16  15 |- D1
  //          A9 -| 18  17 |- D0
  //          A8 -| 20  19 |- VCC
  //          A7 -| 22  21 |- /CS1
  //          A3 -| 24  23 |- A14
  //          A6 -| 26  25 |- A2
  //          A5 -| 28  27 |- A1
  //          A4 -| 30  29 |- /DBIN
  //  /WE/CPUCLK -| 32  31 |- A0
  //   /INT4 /EC -| 34  33 |- SOUND
  //       CRUIN -| 36  35 |- /CS0
  //              +--------+
  //
  //                                           BACK
  //        /CS0 SND  A0 /DB  A1  A2 A14 /CS1 VCC D0  D1  D2  D3  D4  D5  D6  D7  GND
  //      +----------------------------------------------------------------------------+
  //      |   35  33  31  29  27  25  23  21  19  17  15  13  11   9   7   5   3   1   |
  // LEFT |                                                                            | RIGHT
  //      |   36  34  32  30  28  26  24  22  20  18  16  14  12  10   8   6   4   2   |
  //      +----------------------------------------------------------------------------+
  //        CRIN /INT /WE A4  A5  A6  A3  A7  A8  A9 A10 A11 A12 A13 A15 J1-6 /RST GND
  //                                           FRONT

  // CONTROL PINS:
  // /RESET(PH0) - SNES RESET
  // /CS0(PH3)   - SNES /CS
  // /DBIN(PH4)  - SNES /IRQ
  // /CS1(PH6)   - SNES /RD

  // NOTE: PYUUTA ADDRESS AND DATA BUS ARE BIG-ENDIAN
  // LEAST SIGNIFICANT IS BIT 7 AND MOST SIGNIFICANT IS BIT 0
  // PCB ADAPTER WIRED FOR DIFFERENCE

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const romSizes[] = {
    8,
    16,
    32,
  };

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
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::TomyPyuuta), menuOptions, sizeofarray(menuOptions)))
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::TomyPyuuta));
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
    // PYUUTA uses A0-A15 [A16-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //      /RST(PH0)   ---(PH1)  /CS0(PH3)  /DBIN(PH4)  ---(PH5)  /CS1(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //      /RST(PH0)   ---(PH1)  /CS0(PH3)  /DBIN(PH4)  ---(PH5)  /CS1(PH6)
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
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::TomyPyuuta));
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
    PORTK = (addr >> 8) & 0xFF; // A8-A15

    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    return PINC;
  }

  void readSegment(uint32_t startaddr, uint32_t endaddr)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += 512)
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

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::TomyPyuuta), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::Raw));

    cartOn();

    //8K $8000-$9FFF
    //16K $8000-$BFFF
    //32K $4000-$BFFF
    PORTH &= ~(1 << 4); // /DBIN(PH4) LOW

    if (romSize > 1) // 32K [3D CARTS]
    {
      PORTH &= ~(1 << 6); // /CS1(PH6) LOW
      readSegment(0x4000, 0x8000); // +16K = 32K
      PORTH |= (1 << 6); // /CS1(PH6) HIGH
    }

    PORTH &= ~(1 << 3); // /CS0(PH3) LOW
    readSegment(0x8000, 0xA000); // 8K
    PORTH |= (1 << 3); // /CS0(PH3) HIGH

    if (romSize > 0) // 16K
    {
      PORTH &= ~(1 << 3); // /CS0(PH3) LOW
      readSegment(0xA000, 0xC000); // +8K = 16K
      PORTH |= (1 << 3); // /CS0(PH3) HIGH
    }

    PORTH |= (1 << 4); // /DBIN(PH4) HIGH

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

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(romSizes[romSize] * 1024);

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
} /* namespace OSCR::Cores::TomyPyuuta */

#endif /* HAS_PYUUTA */
