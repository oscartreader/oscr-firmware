//******************************************
// ATARI 8-bit (400/800/XL/XE) MODULE
//******************************************
#include "config.h"

#if HAS_ATARI8
# include "cores/include.h"
# include "cores/Atari8.h"

namespace OSCR::Cores::Atari8
{
  // Atari 8-bit (400/800/XL/XE)
  // Left Slot Cartridge Pinout
  // 30P 2.54mm pitch
  //
  //             RIGHT
  //            +------+
  //      PHI2 -| S 15 |- /CCTL
  //       R/W -| R 14 |- RD5
  // L     A10 -| P 13 |- +5V    B
  // A     A11 -| N 12 |- /S5    O
  // B      D7 -| M 11 |- D6     T
  // E      D3 -| L 10 |- D0     T
  // L     A12 -| K  9 |- D1     O
  //        A9 -| J  8 |- D2     M
  // S      A8 -| H  7 |- D5
  // I      A7 -| F  6 |- D4     S
  // D      A6 -| E  5 |- A0     I
  // E      A5 -| D  4 |- A1     D
  //        A4 -| C  3 |- A2     E
  //       GND -| B  2 |- A3
  //       RD4 -| A  1 |- /S4
  //            +------+
  //              LEFT
  //
  //                                LABEL SIDE
  //
  //        RD4 GND A4  A5  A6  A7  A8  A9  A12 D3  D7  A11 A10 R/W PHI2
  //      +-------------------------------------------------------------+
  //      |  A   B   C   D   E   F   H   J   K   L   M   N   P   R   S  |
  // LEFT |                                                             | RIGHT
  //      |  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  |
  //      +-------------------------------------------------------------+
  //        /S4 A3  A2  A1  A0  D4  D5  D2  D1  D0  D6 /S5 +5V RD5 /CCTL
  //
  //                                BOTTOM SIDE
  //
  // CONTROL PINS:
  // RD4(PH0)   - SNES RESET
  // PHI2(PH1)  - SNES CPUCLK
  // /S5(PH3)   - SNES /CS
  // RD5(PH4)   - SNES /IRQ
  // R/W(PH5)   - SNES /WR
  // /S4(PH6)   - SNES /RD
  // /CCTL(PL0) - SNES A16

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // DEFINES
  //******************************************
  #define DISABLE_S4  PORTH |= (1 << 6) // ROM SELECT $8000-$9FFF
  #define ENABLE_S4   PORTH &= ~(1 << 6)
  #define DISABLE_S5  PORTH |= (1 << 3) // ROM SELECT $A000-$BFFF
  #define ENABLE_S5   PORTH &= ~(1 << 3)
  #define DISABLE_CCTL PORTL |= (1 << 0) // CARTRIDGE CONTROL BLOCK $D500-$D5FF
  #define ENABLE_CCTL  PORTL &= ~(1 << 0)

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const romSizes[] = {
    8,
    16,
    32,
    40,
    64,
    128,
  };

  constexpr uint8_t const romSizeLow = 0; // Lowest Entry
  constexpr uint8_t const romSizeHigh = sizeofarray(romSizes) - 1; // Highest Entry
  uint8_t romSize;

  bool atari8right = 0; // 0 = LEFT Slot, 1 = RIGHT Slot

  // EEPROM MAPPING
  // 07 MAPPER [ATARI 8-BIT SLOT]
  // 08 ROM SIZE

  //******************************************
  // MENU
  //******************************************
  // Base Menu
  constexpr char const atari8MenuItem3[] PROGMEM = "Read RIGHT ROM";

  constexpr char const * const menuOptions[] PROGMEM = {
    OSCR::Strings::MenuOptions::SelectCart,
    OSCR::Strings::MenuOptions::ReadROM,
    atari8MenuItem3,
    OSCR::Strings::MenuOptions::SetSize,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Atari8), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        break;

      case 1: // Read LEFT Slot Cart
        atari8right = 0; // LEFT Slot
        readROM();
        break;

      case 2: // Read RIGHT Slot Cart
        atari8right = 1; // RIGHT Slot
        readROM();
        break;

      case 3: // Set Size
        setROMSize();
        break;

      case 4: // Back
        closeCRDB();
        return;
      }
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Atari8));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // Atari 8-bit uses A0-A12 [A13-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF; // Use A16 for /CCTL Output

    // Set Control Pins to Output
    //      PHI2(PH1)   /S5(PH3)   R/W(PH5)   /S4(PH6)
    DDRH |=  (1 << 1) | (1 << 3) | (1 << 5) | (1 << 6);
    // Set RD4 & RD5 to Input
    //        RD4(PH0)   RD5(PH4)
    DDRH &= ~((1 << 0) | (1 << 4));

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //        PHI2(PH1)   /S5(PH3)   R/W(PH5)   /S4(PH6)
    //PORTH |= (1 << 1) | (1 << 3) | (1 << 5) | (1 << 6);
    //       /S5(PH3)   R/W(PH5)   /S4(PH6)
    PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTL = 0xFF; // A16-A23 (A16 used for /CCTL Output)
    PORTJ |= (1 << 0); // TIME(PJ0)

    checkStatus();
    useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Atari8));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ FUNCTIONS
  //******************************************

  uint8_t readData(uint16_t addr) // Add Input Pullup
  {
    PORTF = addr & 0xFF;        // A0-A7
    PORTK = (addr >> 8) & 0xFF; // A8-A12
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // DDRC = 0x00; // Set to Input
    PORTC = 0xFF; // Input Pullup
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    // Extended Delay
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

  void readSegment(uint16_t startaddr, uint16_t endaddr)
  {
    for (uint16_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (int w = 0; w < 512; w++)
      {
        OSCR::Storage::Shared::buffer[w] = readData(addr + w);
      }

      OSCR::Storage::Shared::sharedFile.write();
    }
  }

  void readBountyBobBank(uint16_t startaddr)
  {
    for (uint8_t w = 0; w < 4; w++)
    {
      readData(startaddr + 0x0FF6 + w);
      readSegment(startaddr, startaddr + 0x0E00);

      // Split Read of Last 0x200 bytes
      for (int x = 0; x < 0x1F6; x++)
      {
        OSCR::Storage::Shared::buffer[x] = readData(startaddr + 0x0E00 + x);
      }

      OSCR::Storage::Shared::sharedFile.write((size_t)502);

      // Bank Registers 0xFF6-0xFF9
      for (int y = 0; y < 4; y++)
      {
        readData(startaddr + 0x0FFF); // Reset Bank
        OSCR::Storage::Shared::buffer[y] = readData(startaddr + 0x0FF6 + y);
      }

      // End of Bank 0x8FFA-0x8FFF

      readData(startaddr + 0x0FFF); // Reset Bank
      readData(startaddr + 0x0FF6 + w); // Set Bank

      for (int z = 4; z < 10; z++)
      {
        OSCR::Storage::Shared::buffer[z] = readData(startaddr + 0x0FF6 + z); // 0xFFA-0xFFF
      }

      OSCR::Storage::Shared::sharedFile.write((size_t)10);
    }
    readData(startaddr + 0x0FFF); // Reset Bank
  }

  void bankSwitch(uint8_t bank)
  {
    // write to $D5XX using /CCTL
    // CCTL sets upper 3 bits of 16-bit address to 110
    // CCTL = [110] 1 0101 0000 0000 = $D500
    ENABLE_CCTL;
    // Set Address $D500
    PORTF = 0x00; // A0-A7
    PORTK = 0xD5; // A8-A12
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // Set Data to Output
    DDRC = 0xFF;
    // Set R/W to WRITE
    PORTH &= ~(1 << 5);

    // Set Bank
    PORTC = bank;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // Pulse Clock
    // CPU CLOCK 1.7897725MHz(NTSC)/1.7734470MHz(PAL/SECAM)
    // 1.7897725MHz = 1 cycle = 558.73023ns = 279.365115/279.365115
    for (int i = 0; i < 2; i++)
    {
      PORTH ^= (1 << 1);
      // NOP (62.5ns) x 4 = 250ns = 0.25us
      NOP;
      NOP;
      NOP;
      NOP; // 4 NOPs = 4 x 62.5ns = 250ns x 2 = 500ns = 2 MHz
    }

    // Set R/W to READ
    PORTH |= (1 << 5);
    // Reset Data to Input
    DDRC = 0x00;
    DISABLE_CCTL;
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Atari8), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    // ATARI 8-bit A12-A0 = 1 0000 0000 0000
    // S4 [100]/S5 [101] are the upper 3 bits for 16-bit address
    // S4 = [100] 1 1111 1111 1111 = $8000-$9FFF
    // S5 = [101] 1 1111 1111 1111 = $A000-$BFFF

    if (atari8right) // RIGHT Slot Cartridge 8K
    {
      // Right slot carts are 8K mapped to $8000-$9FFF
      // Right slot carts use /S4 assigned to Pin 12
      // Pin 12 = RIGHT Slot /S4 = LEFT Slot /S5
      ENABLE_S5;
      readSegment(0x8000, 0xA000); // 8K
      DISABLE_S5;
      // Correct Size to 8K
      romSize = 0; // 8K
    }
    else if (romSize == 3) // Bounty Bob Strikes Back 40K
    {
      ENABLE_S4;

      // First 16KB (4KB x 4)
      readBountyBobBank(0x8000);

      // Second 16KB (4KB x 4)
      readBountyBobBank(0x9000);

      DISABLE_S4;

      ENABLE_S5;
      readSegment(0xA000, 0xC000); // +8K = 40K
      DISABLE_S5;
    }
    else if (romSize > 1) // XE Carts 32K/64K/128K
    {
      // Bug Hunt and Lode Runner dump as 128K.  Trim beginning 64K as both carts start from Bank 8.
      int banks = (romSizes[romSize] / 8) - 1;

      for (int x = 0; x < banks; x++)
      {
        bankSwitch(x);

        ENABLE_S4;
        readSegment(0x8000, 0xA000); // 8K
        DISABLE_S4;
      }

      // Last Bank
      ENABLE_S5;
      readSegment(0xA000, 0xC000); // +8K
      DISABLE_S5;
    }
    else // Standard LEFT Cart 8K/16K
    {
      if (romSize == 1)
      {
        // Add XE Bankswitch for Necromancer 16K
        bankSwitch(0);

        // Standard 16K
        ENABLE_S4;
        readSegment(0x8000, 0xA000); // +8K = 16K
        DISABLE_S4;
      }

      ENABLE_S5;
      readSegment(0xA000, 0xC000); // 8K
      DISABLE_S5;
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
    if (romSizeLow == romSizeHigh)
    {
      romSize = romSizeLow;
    }
    else
    {
      romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes, sizeofarray(romSizes));
    }
  }

  void checkStatus()
  {
    printHeader();

    OSCR::UI::print(F("SLOT: "));
    OSCR::UI::printLine((atari8right) ? F("RIGHT") : F("LEFT"));

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, romSizes[romSize] * 1024);

    OSCR::UI::wait();
  }

  void setCart()
  {
    OSCR::Databases::Standard::browseDatabase();

    romSize = romDetail->size;
    atari8right = romDetail->mapper;

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::Atari8 */
#endif /* HAS_ATARI8 */
