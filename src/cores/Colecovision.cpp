//******************************************
// COLECOVISION MODULE
//******************************************
#include "config.h"

#if HAS_COLV
# include "cores/include.h"
# include "cores/Colecovision.h"

namespace OSCR::Cores::Colecovision
{
  // Coleco Colecovision
  // Cartridge Pinout
  // 30P 2.54mm pitch connector
  //
  //  FRONT SIDE             BACK SIDE
  //       (EVEN)            (ODD)
  //              +-------+
  //       /C000 -| 2   1 |- D2
  //          D3 -| 4   3 |- D1
  //          D4 -| 6   5 |- D0
  //          D5 -| 8   7 |- A0
  //          D6 -| 10  9 |- A1
  //          D7 -| 12 11 |- A2
  //         A11 -| 14 13 |- GND (SHLD)
  //         A10 -| 16 15 |- A3
  //       /8000 -| 18 17 |- A4
  //         A14 -| 20 19 |- A13
  //       /A000 -| 22 21 |- A5
  //         A12 -| 24 23 |- A6
  //          A9 -| 26 25 |- A7
  //          A8 -| 28 27 |- /E000
  //    VCC(+5V) -| 30 29 |- GND
  //              +-------+

  // CONTROL PINS:
  // CHIP SELECT PINS
  // /8000(PH3) - CHIP 0 - SNES /CS
  // /A000(PH4) - CHIP 1 - SNES /IRQ
  // /C000(PH5) - CHIP 2 - SNES /WR
  // /E000(PH6) - CHIP 3 - SNES /RD

  using OSCR::Databases::Extended::crdbRecord;
  using OSCR::Databases::Extended::crdb;
  using OSCR::Databases::Extended::romDetail;
  using OSCR::Databases::Extended::romRecord;

  constexpr uint8_t const COL[] = {
    8,
    12,
    16,
    20,
    24,
    32,
  };
  constexpr uint8_t const collo = 0;  // Lowest Entry
  constexpr uint8_t const colhi = sizeofarray(COL) - 1;  // Highest Entry

  // EEPROM MAPPING
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Colecovision), menuOptions, sizeofarray(menuOptions)))
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Colecovision));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // Colecovision uses A0-A14 [A15-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       ---(PH0)   ---(PH1)  /8000(PH3) /A000(PH4) /C000(PH5) /E000(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       ---(PH0)   ---(PH1)  /8000(PH3) /A000(PH4) /C000(PH5) /E000(PH6)
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
    OSCR::Databases::Extended::setup(FS(OSCR::Strings::FileType::Colecovision));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ CODE
  //******************************************
  // CHIP SELECT CONTROL PINS
  // /8000(PH3) - CHIP 0
  // /A000(PH4) - CHIP 1
  // /C000(PH5) - CHIP 2
  // /E000(PH6) - CHIP 3

  uint8_t readData(uint32_t addr)
  {
    // SELECT ROM CHIP - PULL /CE LOW
    uint8_t chipdecode = ((addr >> 13) & 0x3);
    if (chipdecode == 3)       // CHIP 3
      PORTH &= ~(1 << 6);      // /E000 LOW (ENABLE)
    else if (chipdecode == 2)  // CHIP 2
      PORTH &= ~(1 << 5);      // /C000 LOW (ENABLE)
    else if (chipdecode == 1)  // CHIP 1
      PORTH &= ~(1 << 4);      // /A000 LOW (ENABLE)
    else                       // CHIP 0
      PORTH &= ~(1 << 3);      // /8000 LOW (ENABLE)

    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15

    // LATCH ADDRESS - PULL /CE HIGH
    PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);  // ALL /CE HIGH (DISABLE)

    uint8_t ret = PINC;
    return ret;
  }

  void readSegment(uint16_t startaddr, uint32_t endaddr)
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

  void readROM()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Colecovision), FS(OSCR::Strings::Directory::ROM), fileName);

    cartOn();

    // RESET ALL CS PINS HIGH (DISABLE)
    PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    uint32_t const startAddr = 0x8000;
    uint32_t endAddr = startAddr + (COL[romSize] * 0x400);

    readSegment(startAddr, endAddr);

    cartOff();

    OSCR::Storage::Shared::close();

    // Compare CRC32 to database and rename ROM if found
    OSCR::Databases::Extended::matchCRC();
  }

  //******************************************
  // ROM SIZE
  //******************************************
  void setROMSize()
  {
    if (collo == colhi)
    {
      romSize = collo;
      return;
    }

    romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), COL, sizeofarray(COL));
  }

  void checkStatus()
  {
    if (romSize > 5)
    {
      romSize = 0;
    }

    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(COL[romSize] * 1024);
  }

  //******************************************
  // CART SELECT CODE
  //******************************************
  void setCart()
  {
    OSCR::Databases::Extended::browseDatabase();

    romSize = romDetail->size;

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::Colecovision */

#endif /* HAS_COLV */
