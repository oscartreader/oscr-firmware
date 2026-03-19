//******************************************
// LITTLE JAMMER MODULE
//******************************************
#include "config.h"

#if HAS_LJ
# include "cores/include.h"
# include "cores/LittleJammer.h"

namespace OSCR::Cores::LittleJammer
{
  // Little Jammer
  // Cartridge Pinout
  // 48P 1.25mm pitch connector
  //
  // FORM FACTOR IS SAME AS BANDAI WONDERSWAN/BENESSE POCKET CHALLENGE V2/LITTLE JAMMER PRO
  // WIRING IS COMPLETELY DIFFERENT!
  //
  // LEFT SIDE
  // 1 VSS (GND)
  // 2 A-1
  // 3 A0
  // 4 A1
  // 5 A2
  // 6 A3
  // 7 A4
  // 8 A5
  // 9 A6
  // 10 A7
  // 11 A8
  // 12 A9
  // 13 A10
  // 14 A11
  // 15 A12
  // 16 A13
  // 17 A14
  // 18 A15
  // 19 A16
  // 20 A17
  // 21 A18
  // 22 A19
  // 23 A20
  // 24 VCC (+5V)
  // 25 VCC (+5V)
  // 26 D0
  // 27 D1
  // 28 D2
  // 29 D3
  // 30 D4
  // 31 D5
  // 32 D6
  // 33 D7
  // 34 /WE
  // 35 /RESET
  // 36 /CE
  // 37 /OE
  // 38 VSS (GND)
  // 39 NC
  // 40 NC
  // 41 NC
  // 42 NC
  // 43 NC
  // 44 NC
  // 45 NC
  // 46 NC
  // 47 VSS (GND)
  // 48 VSS (GND)
  // RIGHT SIDE

  // CONTROL PINS:
  // /RESET(PH0) - SNES RESET
  // /CE(PH3)    - SNES /CS
  // /WE(PH5)    - SNES /WR
  // /OE(PH6)    - SNES /RD

  // LITTLE JAMMER DIRECT ADDRESSING
  // 1 1111 1111 1111 1111 1111
  // 1 F    F    F    F    F = 0x1FFFFF
  // Size = 0x200000 = 2MB
  //
  // A20 connection on Pin 23 = 0x400000 = 4MB
  // 11 1111 1111 1111 1111 1111
  // 3 F    F    F    F    F = 0x3FFFFF
  // Size = 0x400000 = 4MB

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const LJ[] = {
    1,
    2,
    4,
  };

  constexpr uint8_t const ljlo = 0; // Lowest Entry
  constexpr uint8_t const ljhi = sizeofarray(LJ) - 1; // Highest Entry

  // EEPROM MAPPING
  // 08 ROM SIZE

  //******************************************
  // MENU
  //******************************************
  // Base Menu
  constexpr char const * const menuOptionsLJ[] PROGMEM = {
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::LittleJammer), menuOptionsLJ, sizeofarray(menuOptionsLJ)))
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
        checkStatus();
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::LittleJammer));
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
    // LITTLE JAMMER uses A(-1)-A19 wired to A0-A20 [A21-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //      /RST(PH0)   ---(PH1)   /CE(PH3)   ---(PH4)   /WR(PH5)   /OE(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //      /RST(PH0)   ---(PH1)   /CE(PH3)   ---(PH4)   /WR(PH5)   /OE(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTJ |= (1 << 0); // TIME(PJ0)

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::LittleJammer));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ FUNCTIONS
  //******************************************

  uint8_t readData(uint32_t addr)
  {
    PORTF = addr  & 0xFF;        // A(-1)-A6
    PORTK = (addr >> 8) & 0xFF;  // A7-A14
    PORTL = (addr >> 16) & 0xFF; // A15-A20

    NOP;
    NOP;

    // switch /CE(PH3) to LOW
    PORTH &= ~(1 << 3);

    // switch /OE(PH6) to LOW
    PORTH &= ~(1 << 6);

    NOP;
    NOP;

    uint8_t ret = PINC;

    // switch /CE(PH3) and /OE(PH6) to HIGH
    PORTH |= (1 << 3) | (1 << 6);

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

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::LittleJammer), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::Raw));

    cartOn();

    // Maximum Direct Address Size is 4MB
    readSegment(0x000000, 0x100000); // 1MB

    if (romSize > 0) // 2MB/4MB
    {
      readSegment(0x100000, 0x200000); // +1MB = 2MB

      if (romSize > 1) // 4MB
      {
        readSegment(0x200000, 0x400000); // +2MB = 4MB
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
    if (ljlo == ljhi)
    {
      romSize = ljlo;
      return;
    }

    romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), LJ, sizeofarray(LJ));
  }

  void checkStatus()
  {
    if (romSize > ljhi)
    {
      romSize = 1; // default 2M
    }

    printHeader();

    if (fromCRDB)
    {
      OSCR::UI::printValue(OSCR::Strings::Common::Name, romDetail->name);
    }

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, LJ[romSize] * 1024 * 1024);

    OSCR::UI::wait();
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
} /* namespace OSCR::Cores::LittleJammer */

#endif /* HAS_LJ */
