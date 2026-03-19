//******************************************
// WSV MODULE
//******************************************
#include "config.h"

#if HAS_WSV
# include "cores/include.h"
# include "cores/Supervision.h"

namespace OSCR::Cores::Supervision
{
  // Watara Supervision
  // Cartridge Pinout
  // 40P 2.5mm pitch connector
  //
  //       BACK            LABEL
  //       SIDE            SIDE
  //            +-------+
  //       /RD -| 1  40 |- +5V
  //        A0 -| 2  39 |- nc
  //        A1 -| 3  38 |- nc
  //        A2 -| 4  37 |- nc
  //        A3 -| 5  36 |- nc
  //        A4 -| 6  35 |- /WR
  //        A5 -| 7  34 |- D0
  //        A6 -| 8  33 |- D1
  //        A7 -| 9  32 |- D2
  //        A8 -| 10 31 |- D3
  //        A9 -| 11 30 |- D4
  //       A10 -| 12 29 |- D5
  //       A11 -| 13 28 |- D6
  //       A12 -| 14 27 |- D7
  //       A13 -| 15 26 |- nc
  //       A14 -| 16 25 |- nc
  //       A15 -| 17 24 |- L1
  //       A16 -| 18 23 |- L2
  //        L3 -| 19 22 |- GND
  //        L0 -| 20 21 |- PWR GND
  //            +-------+
  //
  // L3 - L0 are the Link Port's I/O - only the 'MAGNUM' variant
  // routed these to the cartridge slot as additional banking bits.
  //
  // CONTROL PINS:
  // /WR - (PH5)
  // /RD - (PH6)

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  constexpr uint16_t const romSizes[] = {
    32,
    64,
    512,
  };

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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Supervision), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        break;

      case 1: // Read Rom
        readROM();
        break;

      case 2: // Cart Config
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Supervision));
  }

  void cartOn()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //BA0-BA7 (BA5-BA7 UNUSED)
    DDRL = 0xFF;
    //PA0-PA7 (UNUSED)
    DDRA = 0xFF;

    // Set Data Pins (D0-D7) to Input
    // D0 - D7
    DDRC = 0x00;

    // Set Control Pins to Output
    //       WR(PH5)    RD(PH6)
    //  DDRH |= (1 << 5) | (1 << 6);
    //      ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   /WR(PH5)   /RD(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Switch WR(PH5) to HIGH
    //  PORTH |= (1 << 5);
    // Switch RD(PH6) to LOW
    //  PORTH &= ~(1 << 6);
    // Setting Control Pins to HIGH
    //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   /WR(PH5)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);

    // Switch RD(PH6) to LOW
    PORTH &= ~(1 << 6);

    // Set Unused Pins HIGH
    PORTL = 0xE0;
    PORTA = 0xFF;
    PORTJ |= (1 << 0);  // TIME(PJ0)

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Supervision));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  //  LOW LEVEL FUNCTIONS
  //******************************************

  // WRITE
  void controlOut()
  {
    // Switch RD(PH6) to HIGH
    PORTH |= (1 << 6);
    // Switch WR(PH5) to LOW
    PORTH &= ~(1 << 5);
  }

  // READ
  void controlIn()
  {
    // Switch WR(PH5) to HIGH
    PORTH |= (1 << 5);
    // Switch RD(PH6) to LOW
    PORTH &= ~(1 << 6);
  }

  void dataIn()
  {
    DDRC = 0x00;
  }

  void dataOut()
  {
    DDRC = 0xFF;
  }

  uint8_t readByte(uint32_t addr)
  {
    PORTF = addr & 0xFF;
    PORTK = (addr >> 8) & 0xFF;
    PORTL = (addr >> 16) & 0xFF;

    // Wait for data bus
    // 6 x 62.5ns = 375ns
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;
    NOP;

    return ret;
  }

  //******************************************
  // READ CODE
  //******************************************

  void readROM()
  {
    uint32_t romStart = 0;

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Supervision), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    // start reading rom
    dataIn();
    controlIn();

    romSize = romSizes[romSize];

    if (romSize < 64)
    {
      romStart = 0x8000;
    }

    for (uint32_t addr = 0, romEnd = (uint32_t)romSize * 0x400; addr < romEnd; addr += 512)
    {
      for (uint16_t w = 0; w < 512; w++)
      {
        OSCR::Storage::Shared::buffer[w] = readByte(romStart + addr + w);
      }

      OSCR::Storage::Shared::writeBuffer();
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
} /* namespace OSCR::Cores::Supervision */

#endif /* HAS_WSV */
