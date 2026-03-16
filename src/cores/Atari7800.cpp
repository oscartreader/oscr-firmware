//******************************************
// ATARI 7800 MODULE
//******************************************
#include "config.h"

#if HAS_7800
# include "cores/include.h"
# include "cores/Atari7800.h"

namespace OSCR::Cores::Atari7800
{
  // Atari 7800
  // Cartridge Pinout
  // 32P 2.54mm pitch (USE 36P CONNECTOR)
  //
  //              RIGHT
  //            +-------+
  //       R/W -|  1 32 |- CLK
  //      HALT -|  2 31 |- IRQ
  //            |-------|
  //        D3 -|  3 30 |- GND
  // L      D4 -|  4 29 |- D2     B
  // A      D5 -|  5 28 |- D1     O
  // B      D6 -|  6 27 |- D0     T
  // E      D7 -|  7 26 |- A0     T
  // L     A12 -|  8 25 |- A1     O
  //       A10 -|  9 24 |- A2     M
  // S     A11 -| 10 23 |- A3
  // I      A9 -| 11 22 |- A4     S
  // D      A8 -| 12 21 |- A5     I
  // E     +5V -| 13 20 |- A6     D
  //       GND -| 14 19 |- A7     E
  //            |-------|
  //       A13 -| 15 18 |- AUD
  //       A14 -| 16 17 |- A15
  //            +-------+
  //              LEFT
  //
  //                                     LABEL SIDE
  //
  //         A14 A13  GND +5V  A8  A9 A11 A10 A12  D7  D6  D5  D4  D3  HLT R/W
  //       +--------------------------------------------------------------------+
  //       |  16  15 | 14  13  12  11  10   9   8   7   6   5   4   3 |  2   1  |
  // LEFT  |         |                                                |         | RIGHT
  //       |  17  18 | 19  20  21  22  23  24  25  26  27  28  29  30 | 31  32  |
  //       +--------------------------------------------------------------------+
  //         A15 AUD   A7  A6  A5  A4  A3  A2  A1  A0  D0  D1  D2 GND  IRQ CLK
  //
  //                                    BOTTOM SIDE

  // CONTROL PINS:
  // CLK(PH1) - SNES CPUCLK
  // R/W(PH6) - SNES /RD
  // HALT(PH0) - SNES /RESET

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  using OSCR::Databases::Basic::mapperDetail;

  //******************************************
  //  Menu
  //******************************************
  // Base Menu
  constexpr char const * const menuOptions7800[] PROGMEM = {
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Atari7800), menuOptions7800, sizeofarray(menuOptions7800)))
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Atari7800));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // Atari 7800 uses A0-A15 [A16-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       HALT(PH0)  ---(PH3)   ---(PH4)   ---(PH5)   R/W(PH6)
    DDRH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       HALT(PH0)   ---(PH3)   ---(PH4)   ---(PH5)   R/W(PH6)
    PORTH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTL = 0xFF;       // A16-A23
    PORTJ |= (1 << 0);  // TIME(PJ0)

#ifdef ENABLE_CLOCKGEN
    // Set Eeprom clock to 1Mhz
    OSCR::ClockGen::clockgen.set_freq(200000000ULL, SI5351_CLK1);

    // Start outputting Eeprom clock
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 1);  // Eeprom clock

    // Wait for clock generator
    OSCR::ClockGen::clockgen.update_status();
#else
    // Set CLK(PH1) to Output
    DDRH |= (1 << 1);

    // Output a high signal CLK(PH1)
    PORTH |= (1 << 1);
#endif

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Atari7800));
    OSCR::Databases::Basic::setupMapper(F("a7800-mappers"));
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
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    //  DDRC = 0x00; // Set to Input
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
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

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

  void readSegmentBank(uint8_t startbank, uint8_t endbank)
  {
    for (uint8_t x = startbank; x < endbank; x++)
    {
      writeData(0x8000, x);
      readSegment(0x8000, 0xC000);
    }
  }

  void readStandard()
  {
    if (romSize > 32)
      readSegment(0x4000, 0x8000); // +16K = 48K

    if (romSize > 16)
      readSegment(0x8000, 0xC000); // +16K = 32K

    // Base 16K
    readSegment(0xC000, 0x10000);  // 16K
  }

  void readSupergame()
  {
    readSegmentBank(0, 7);         // Bank 0-6 16K * 7 = 112K
    readSegment(0xC000, 0x10000);  // Bank 7  +16K = 128K
  }

  void writeData(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
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

    PORTH &= ~(1 << 6);  // R/W(PH6) LOW = WRITE
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    PORTH |= (1 << 6);  // R/W(PH6) HIGH = READ
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    DDRC = 0x00;  // Reset to Input
  }

  // Activision Bankswitch - Double Dragon/Rampage 128K
  void bankSwitch(uint16_t addr)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    PORTH &= ~(1 << 6);            // R/W(PH6) LOW = WRITE
    for (int y = 0; y < 8; y++) {  // Pulse Clock to latch data into PAL16 Chip
      PORTH &= ~(1 << 1);          // CLK(PH1) LOW
      NOP;
      PORTH |= (1 << 1);  // CLK(PH1) HIGH
      NOP;
    }

    PORTH |= (1 << 6);  // R/W(PH6) HIGH = READ

    DDRC = 0x00;  // Reset to Input
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Atari7800), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    switch (mapperDetail->id)
    {
    case 0: // Standard 16K/32K/48K [7816/7832/7848]
      readStandard();
      break;

    case 1: // SuperGame 128K [78SG]
      readSupergame();
      break;

    case 2: // SuperGame - Alien Brigade/Crossbow 144K [78S9]
      readSegment(0x4000, 0x8000);  // 16K
      readSegmentBank(0, 8);        // 16K * 8 = +128K = 144K
      break;

    case 3: // F-18 Hornet 64K [78AB]
      for (int x = 1; x >= 0; x--)    // Bank Order Inverted = Bank 1 then Bank 0
      {
        writeData(0x8000, x);         // Banks 0-1
        readSegment(0x4000, 0x8000);  // 16K * 2 = 32K
      }
      readSegment(0x8000, 0x10000);   // +32K = 64K
      break;

    case 4: // Double Dragon/Rampage 128K [78AC]
    /*
      // THIS CODE OUTPUTS ROMS THAT SHOULD MATCH THE CURRENT NO-INTRO DATABASE
      // BUT THE ROMS INCLUDED IN THE NO-INTRO DATABASE WERE DONE INCORRECTLY
      // THE UPPER AND LOWER HALVES OF EACH 16K BANK ARE IN THE WRONG ORDER
      // THE OLD BANKSWITCH INFORMATION IS INCORRECT/INCOMPLETE
      // Double Dragon (NTSC) = CRC32 AA265865 BAD
      // Double Dragon (PAL)  = CRC32 F29ABDB2 BAD
      // Rampage (NTSC)       = CRC32 39A316AA BAD
      for (int x = 0; x < 8; x++)
      {
        bankSwitch(0xFF80 + x); // Activision Bankswitch
        readSegment(0xA000, 0xE000); // 16K * 8 = 128K
      }
    */
      // THIS CODE OUTPUTS PROPER ROMS THAT SHOULD MATCH MAME
      // THE UPPER AND LOWER HALVES OF EACH 16K BANK ARE IN THE CORRECT ORDER
      // Double Dragon (NTSC) = CRC32 F20773D5
      // Double Dragon (PAL)  = CRC32 4D634BF5
      // Rampage (NTSC)       = CRC32 D2876EE2
      for (int x = 0; x < 8; x++)
      {
        bankSwitch(0xFF80 + x);       // Activision Bankswitch
        readSegment(0xC000, 0xE000);  // 8K * 8 = 64K
        readSegment(0xA000, 0xC000);  // 8K * 8 = +64K = 128K
      }
      break;

    case 5: // Realsports Baseball/Tank Command/Tower Toppler/Waterski 64K [78S4]
      readSegmentBank(0, 4); // 16K * 4 = 64K
      break;

    case 6: // Karateka (PAL) 64K [78S4 Variant]
      readSegmentBank(4, 8); // 16K * 4 = 64K
      break;

    case 7: // Bankset switching
      if (romSize > 64)
      {
        setHalt(1);
        readSupergame();
        setHalt(0);
        readSupergame();
      }
      else
      {
        setHalt(1);
        readStandard();
        setHalt(0);
        readStandard();
      }
      break;

    case 8: // Bentley Bear's Crystal Quest/Bounty Bob Strikes Back!
      readSegment(0x4000, 0x8000);   //            16K
      readSegmentBank(0, 7);         // Bank 0-6 +112K
      readSegment(0xC000, 0x10000);  // Bank 7   + 16K = 144K
      break;
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();
  }

  void setHalt(uint8_t on)
  {
    if (on == 1)
    {
      PORTH |= (1 << 0);  // HALT(PH0) HIGH = SALLY
    }
    else
    {
      PORTH &= ~(1 << 0); // HALT(PH0) LOW = MARIA
    }

    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
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
} /* namespace OSCR::Cores::Atari7800 */
#endif /* HAS_7800 */
