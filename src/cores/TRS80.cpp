//******************************************
// TRS-80 COLOR COMPUTER MODULE
//******************************************
#include "config.h"

#if HAS_TRS80
# include "cores/include.h"
# include "cores/TRS80.h"

namespace OSCR::Cores::TRS80
{
  // TRS-80
  // Color Computer
  // Cartridge Pinout
  // 40P 2.54mm pitch connector
  //
  //      TOP            BOTTOM
  //     SIDE            SIDE
  //          +-------+
  //      NC -|  1 2  |- NC
  //   /HALT -|  3 4  |- /NMI
  //  /RESET -|  5 6  |- E
  //       Q -|  7 8  |- /CART
  //     +5V -|  9 10 |- D0
  //      D1 -| 11 12 |- D2
  //      D3 -| 13 14 |- D4
  //      D5 -| 15 16 |- D6
  //      D7 -| 17 18 |- R/W
  //      A0 -| 19 20 |- A1
  //      A2 -| 21 22 |- A3
  //      A4 -| 23 24 |- A5
  //      A6 -| 25 26 |- A7
  //      A8 -| 27 28 |- A9
  //     A10 -| 29 30 |- A11
  //     A12 -| 31 32 |- /CTS
  //     GND -| 33 34 |- GND
  //     SND -| 35 36 |- /SCS
  //     A13 -| 37 38 |- A14
  //     A15 -| 39 40 |- /SLENB
  //          +-------+
  //
  //                                                       TOP
  //       A15  A13  SND  GND  A12  A10   A8   A6   A4   A2   A0   D7   D5   D3   D1  +5V   Q /RST /HLT NC
  //      +-----------------------------------------------------------------------------------------------+
  //      | 39   37   35   33   31   29   27   25   23   21   19   17   15   13   11    9   7   5   3   1 |
  // LEFT |                                                                                               | RIGHT
  //      | 40   38   36   34   32   30   28   26   24   22   20   18   16   14   12   10   8   6   4   2 |
  //      +-----------------------------------------------------------------------------------------------+
  //       /SLB A14 /SCS  GND /CTS  A11   A9   A7   A5   A3   A1   RW   D6   D4   D2   D0 /CRT  E  /NMI NC
  //                                                      BOTTOM

  // CONTROL PINS:
  // /RESET(PH0) - SNES RESET
  // E(PH1)      - SNES CPUCLK
  // /CTS(PH3)   - SNES /CS
  // /SCS(PH4)   - SNES /IRQ
  // R/W(PH5)    - SNES /WR
  // NOTE:  CARTS CONNECT /CART TO Q

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const romSizes[] = {
    2,
    4,
    8,
    10,
    16,
    32,
    64,
    128,
  };

  // EEPROM MAPPING
  // 08 ROM SIZE

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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::TRS80), menuOptions, sizeofarray(menuOptions)))
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::TRS80));
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
    // TRS-80 uses A0-A15 [A16-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //      /RST(PH0)    E(PH1)   /CTS(PH3)  /SCS(PH4)   R/W(PH5)   ---(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //      /RST(PH0)    E(PH1)   /CTS(PH3)  /SCS(PH4)   R/W(PH5)   ---(PH6)
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
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::TRS80));
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

    // Set /CTS to LOW
    PORTH &= ~(1 << 3); // /CTS LOW
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

    // Pull /CTS to HIGH
    PORTH |= (1 << 3); // /CTS HIGH

    return ret;
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
  // BANKSWITCH
  //******************************************

  // Bankswitch is a combination of IC1 74LS175 (Quad Latch) and IC3 74LS10 (NAND Gate)
  // IC1 latches D0-D3 using /RESET and control (CP) from IC3
  // IC3 controls latch into IC1 using CP output based on R/W, E and /SCS
  // IC3 CP LOW only when R/W LOW, /SCS LOW, and E HIGH
  void bankSwitch(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;        // A0-A7
    PORTK = (addr >> 8) & 0xFF; // A8-A15
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    DDRC = 0xFF; // Set to Output
    PORTC = data;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // SET CP (LATCH INPUT INTO 74LS175) TO LOW
    PORTH &= ~(1 << 4); // /SCS(PH4) LOW
    PORTH &= ~(1 << 5); // R/W(PH5) LOW = WRITE
    // Pulse E to Latch Data
    PORTH &= ~(1 << 1); // E(PH1) LOW
    NOP;
    PORTH |= (1 << 1); // E(PH1) HIGH
    NOP;

    // SET CP TO HIGH
    PORTH |= (1 << 4); // /SCS(PH5) HIGH
    PORTH |= (1 << 5); // R/W(PH5) HIGH = READ

    DDRC = 0x00; // Reset to Input
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::TRS80), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    // Set /RESET to LOW
    PORTH &= ~(1 << 0); // /RESET LOW
    delay(100);
    // Set /RESET to HIGH
    PORTH |= (1 << 0); // /RESET HIGH
    // Set R/W to READ
    PORTH |= (1 << 5); // R/W HIGH

    if (romSize > 5) { // Bankswitch Carts - Predator 64K/Robocop 128K
      // Predator 64K = (2^6) / 16 = 4
      // Robocop 128K = (2^7) / 16 = 8
      int banks = (OSCR::Util::power<2>(romSize)) / 16;
      for (int x = 0; x < banks; x++) {
        bankSwitch(0xFF40, x); // Bankswitch
        readSegment(0xC000, 0x10000); // 16K * 8 = 128K
      }
    }
    else // Normal Carts 2K/4K/8K/10K/16K/32K
    {
      readSegment(0xC000, 0xC800); // 2K

      if (romSize > 0)
      {
        readSegment(0xC800, 0xD000); // +2K = 4K

        if (romSize > 1)
        {
          readSegment(0xD000, 0xE000); // +4K = 8K

          if (romSize > 2)
          {
            readSegment(0xE000, 0xE800); // +2K = 10K

            if (romSize > 3)
            {
              readSegment(0xE800, 0x10000); // +6K = 16K

              if (romSize == 5) // 32K
              {
                // Second Chip Select - Switch to Upper 16K (Mind-Roll)
                PORTH &= ~(1 << 4); // /SCS LOW
                NOP; NOP; NOP; NOP; NOP;
                PORTH |= (1 << 4); // /SCS HIGH
                readSegment(0x8000, 0xC000); // +16K = 32K
              }
            }
          }
        }
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
} /* namespace OSCR::Cores::TRS80 */

#endif /* HAS_TRS80 */
