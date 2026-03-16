//******************************************
// ATARI 5200 MODULE
//******************************************
#include "config.h"

#if HAS_5200
# include "cores/include.h"
# include "cores/Atari5200.h"

namespace OSCR::Cores::Atari5200
{
  // Atari 5200
  // Cartridge Pinout
  // 36P 2.54mm pitch connector
  //
  //                        RIGHT
  //                      +-------+
  //           INTERLOCK -| 18 19 |- A0
  //                  A2 -| 17 20 |- A1
  //    L             A5 -| 16 21 |- A3
  //    A             A6 -| 15 22 |- A4         B
  //    B            GND -| 14 23 |- GND        O
  //    E            GND -| 13 24 |- GND        T
  //    L            GND -| 12 25 |- GND        T
  //                 N/C -| 11 26 |- +5V        O
  //   /ENABLE 4000-7FFF -| 10 27 |- A7         M
  //   /ENABLE 8000-BFFF -|  9 28 |- N/C
  //                  D7 -|  8 29 |- A8         S
  //    S             D6 -|  7 30 |- AUD        I
  //    I             D5 -|  6 31 |- A9         D
  //    D             D4 -|  5 32 |- A13        E
  //    E             D3 -|  4 33 |- A10
  //                  D2 -|  3 34 |- A12
  //                  D1 -|  2 35 |- A11
  //                  D0 -|  1 36 |- INTERLOCK
  //                      +-------+
  //                        LEFT
  //
  //                                        LABEL SIDE
  //
  //                                         /EN /EN
  //          D0  D1  D2  D3  D4  D5  D6  D7  80  40  -- GND GND GND  A6  A5  A2  INT
  //       +--------------------------------------------------------------------------+
  //       |   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  |
  // LEFT  |                                                                          | RIGHT
  //       |  36  35  34  33  32  31  30  29  28  27  26  25  24  23  22  21  20  19  |
  //       +--------------------------------------------------------------------------+
  //         INT A11 A12 A10 A13  A9 AUD  A8  --  A7 +5V GND GND GND  A4  A3  A1  A0
  //
  //                                        BOTTOM SIDE

  // CONTROL PINS:
  // /4000(PH5) - SNES /WR
  // /8000(PH6) - SNES /RD

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  using OSCR::Databases::Basic::mapperDetail;

  //******************************************
  //  Defines
  //******************************************
  #define DISABLE_4000 PORTH |= (1 << 5)  // ROM SELECT 4000-7FFF
  #define ENABLE_4000 PORTH &= ~(1 << 5)
  #define DISABLE_8000 PORTH |= (1 << 6)  // ROM SELECT 8000-BFFF
  #define ENABLE_8000 PORTH &= ~(1 << 6)

  // EEPROM MAPPING
  // 07 MAPPER
  // 08 ROM SIZE

  //******************************************
  //  Menu
  //******************************************
  // Base Menu
  constexpr char const * const menuOptions5200[] PROGMEM = {
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Atari5200), menuOptions5200, sizeofarray(menuOptions5200)))
      {
      case 0: // Select Cart
        setCart();
        checkStatus();
        break;

      case 1: // Read ROM
        checkStatus();
        readROM();
        break;

      case 2: // Set Mapper + Size
        setMapperMenu();
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Atari5200));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // Atari 5200 uses A0-A13 [A14-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)  /4000(PH5) /8000(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)  /4000(PH5) /8000(PH6)
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
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Atari5200));
    OSCR::Databases::Basic::setupMapper(F("a5200-mappers"));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ CODE
  //******************************************

  uint8_t readData(uint16_t addr)  // Add Input Pullup
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A13
    cycleDelay(5);

    // DDRC = 0x00; // Set to Input
    PORTC = 0xFF;  // Input Pullup
    cycleDelay(15); // Standard + extended delay for Vanguard

    uint8_t ret = PINC;
    cycleDelay(5);

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

  void readBankBountyBob(uint16_t startaddr)
  {
    for (int w = 0; w < 4; w++) {
      readData(startaddr + 0xFF6 + w);
      readSegment(startaddr, startaddr + 0xE00);

      // Split Read of Last 0x200 bytes
      for (int x = 0; x < 0x1F6; x++)
      {
        OSCR::Storage::Shared::buffer[x] = readData(startaddr + 0xE00 + x);
      }

      OSCR::Storage::Shared::writeBuffer(502);

      // Bank Registers 0xFF6-0xFF9
      for (int y = 0; y < 4; y++)
      {
        readData(startaddr + 0xFFF);  // Reset Bank
        OSCR::Storage::Shared::buffer[y] = readData(startaddr + 0xFF6 + y);
      }

      // End of Bank 0xFFA-0xFFF
      readData(startaddr + 0xFFF);      // Reset Bank
      readData(startaddr + 0xFF6 + w);  // Set Bank

      for (int z = 4; z < 10; z++)
      {
        OSCR::Storage::Shared::buffer[z] = readData(startaddr + 0xFF6 + z);  // 0xFFA-0xFFF
      }

      OSCR::Storage::Shared::writeBuffer(10);
    }

    readData(startaddr + 0xFFF);  // Reset Bank
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Atari5200), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    // 5200 A13-A0 = 10 0000 0000 0000
    switch (mapperDetail->id)
    {
    case 0: // Standard 4KB/8KB/16KB/32KB
      // Lower Half of 32K is at 0x4000
      if (romSize == 32)
      {
        ENABLE_4000;
        cycleDelay(15);
        readSegment(0x4000, 0x8000);  // +16K = 32K
        DISABLE_4000;
        cycleDelay(15);
      }

      // 4K/8K/16K + Upper Half of 32K
      ENABLE_8000;
      cycleDelay(15);

      if (romSize > 8)
        readSegment(0x8000, 0xA000);  // +8K = 16K

      if (romSize > 4)
        readSegment(0xA000, 0xB000);  // +4K = 8K

      // Base 4K
      readSegment(0xB000, 0xC000);  // 4K

      DISABLE_8000;

      cycleDelay(15);
      break;

    case 1: // Two Chip 16KB
      ENABLE_4000;

      cycleDelay(15);

      readSegment(0x4000, 0x6000);  // 8K

      DISABLE_4000;

      cycleDelay(15);

      ENABLE_8000;

      cycleDelay(15);

      readSegment(0x8000, 0xA000);  // +8K = 16K

      DISABLE_8000;

      cycleDelay(15);

      break;

    case 2: // Bounty Bob Strikes Back 40KB [UNTESTED]
      ENABLE_4000;

      cycleDelay(15);

      // First 16KB (4KB x 4)
      readBankBountyBob(0x4000);

      // Second 16KB (4KB x 4)
      readBankBountyBob(0x5000);

      DISABLE_4000;

      cycleDelay(15);

      ENABLE_8000;

      cycleDelay(15);

      readSegment(0x8000, 0xA000);  // +8K = 40K

      DISABLE_8000;

      cycleDelay(15);
      break;
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
    romSize = OSCR::UI::rangeSelect(FS(OSCR::Strings::Headings::SelectCartSize), mapperDetail->sizeLow, mapperDetail->sizeHigh, romSize);
  }

  void checkStatus()
  {
    printHeader();

    if (fromCRDB)
    {
      OSCR::UI::print(FS(OSCR::Strings::Labels::NAME));
      OSCR::UI::printLine(romDetail->name);
    }

    OSCR::UI::print(FS(OSCR::Strings::Labels::MAPPER));
    OSCR::UI::printLine(mapperDetail->name);

    OSCR::UI::print(FS(OSCR::Strings::Labels::SIZE));
    OSCR::Lang::printBytesLine(romSize * 1024);
  }

  //******************************************
  // SET MAPPER
  //******************************************

  void setMapperMenu()
  {
    fromCRDB = false;
    OSCR::Databases::Basic::browseMappers();
  }

  //******************************************
  // CART SELECT CODE
  //******************************************
  void setCart()
  {
    OSCR::Databases::Standard::browseDatabase();

    if (!OSCR::Databases::Basic::matchMapper(romDetail->mapper))
    {
      OSCR::Databases::Basic::printMapperNotFound(romDetail->mapper);
      return;
    }

    romSize = romDetail->size;

    fromCRDB = true;
  }

  // While not precise in terms of exact cycles for NOP due to the for-loop
  // overhead, it simplifies the code while still achieving a similar result.
  void cycleDelay(uint8_t cycleCount)
  {
    for (uint8_t i = 0; i < cycleCount; ++i)
    {
      NOP;
    }
  }
} /* namespace OSCR::Cores::Atari5200 */
#endif /* HAS_5200 */
