//******************************************
// CASIO PV-1000/PV-2000 MODULE
//******************************************
#include "config.h"

#if HAS_PV1000
# include "cores/include.h"
# include "cores/CasioPV1000.h"

namespace OSCR::Cores::CasioPV1000
{
  // Casio PV-1000/PV-2000
  // Cartridge Pinout
  // 36P 2.54mm pitch connector
  //
  //  FRONT               BACK
  //   SIDE               SIDE
  //         +---------+
  //    VCC -| B01 A01 |- GND
  //     NC -| B02 A02 |- NC
  //    A14 -| B03 A03 |- A15
  //    A12 -| B04 A04 |- A13
  //    A10 -| B05 A05 |- A11
  //     A8 -| B06 A06 |- A9
  //     A6 -| B07 A07 |- A7
  //     A4 -| B08 A08 |- A5
  //     A2 -| B09 A09 |- A3
  //     A0 -| B10 A10 |- A1
  //     D6 -| B11 A11 |- D7
  //     D4 -| B12 A12 |- D5
  //     D2 -| B13 A13 |- D3
  //     D0 -| B14 A14 |- D1
  //   /CS1 -| B15 A15 |- /CS2
  //    /WR -| B16 A16 |- /RD
  //    CON -| B17 A17 |- /IORQ
  //    VCC -| B18 A18 |- GND
  //         +---------+
  //
  //                                         BACK
  //        GND NC  A15 A13 A11 A9  A7  A5  A3  A1  D7  D5  D3  D1 /CS2 /RD /IO GND
  //      +-------------------------------------------------------------------------+
  //      | A01 A02 A03 A04 A05 A06 A07 A08 A09 A10 A11 A12 A13 A14 A15 A16 A17 A18 |
  // LEFT |                                                                         | RIGHT
  //      | B01 B02 B03 B04 B05 B06 B07 B08 B09 B10 B11 B12 B13 B14 B15 B16 B17 B18 |
  //      +-------------------------------------------------------------------------+
  //        VCC NC  A14 A12 A10 A8  A6  A4  A2  A0  D6  D4  D2  D0 /CS1 /WR CON VCC
  //                                         FRONT

  // CONTROL PINS:
  // /CS2(PH3) - SNES /CS
  // /CS1(PH4) - SNES /IRQ
  // /WR(PH5) - SNES /WR
  // /RD(PH6) - SNES /RD

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const PV1000[] = {
    8,
    16,
  };
  constexpr uint8_t const pv1000lo = 0; // Lowest Entry
  constexpr uint8_t const pv1000hi = sizeofarray(PV1000) - 1; // Highest Entry

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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::CasioPV1000), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        checkStatus();
        break;

      case 1: // Read ROM
        checkStatus();
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::CasioPV1000));
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
    // PV-1000 uses A0-A15 [A16-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       ---(PH0)   ---(PH1)  /CS2(PH3)  /CS1(PH4)   /WR(PH5)   /RD(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       ---(PH0)   ---(PH1)  /CS2(PH3)  /CS1(PH4)   /WR(PH5)   /RD(PH6)
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
    OSCR::Databases::Basic::setup(F("pv1000"));
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

    // Set /RD to LOW
    PORTH &= ~(1 << 6); // /RD LOW (ENABLE)
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

    // Pull /RD to HIGH
    PORTH |= (1 << 6); // /RD HIGH (DISABLE)

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

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::CasioPV1000), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    if (romSize == 0) // 8K
    {
      PORTH &= ~(1 << 4); // /CS1(PH4) LOW
      readSegment(0x0000, 0x2000); // 8K
      PORTH |= (1 << 4); // /CS1(PH4) HIGH
    }
    else // 16K
    {
      PORTH &= ~(1 << 3); // /CS2(PH3) LOW
      readSegment(0x0000, 0x4000); // 16K
      PORTH |= (1 << 3); // /CS2(PH3) HIGH
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Basic::matchCRC();
  }

  //******************************************
  // ROM SIZE
  //******************************************
  void setROMSize()
  {
    if (pv1000lo == pv1000hi)
    {
      romSize = pv1000lo;
      return;
    }

    romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), PV1000, sizeofarray(PV1000));
  }

  void checkStatus()
  {
    if (romSize > pv1000hi)
    {
      romSize = 0; // default 8K
    }

    printHeader();

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, PV1000[romSize] * 1024);
  }

  //******************************************
  // CART SELECT CODE
  //******************************************

  void setCart()
  {
    OSCR::Databases::Standard::browseDatabase();

    romSize = romDetail->size;
  }
} /* namespace OSCR::Cores::CasioPV1000 */

#endif /* HAS_PV1000 */
