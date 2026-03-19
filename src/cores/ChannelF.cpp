//******************************************
// FAIRCHILD CHANNEL F MODULE
//******************************************
#include "config.h"

#if HAS_FAIRCHILD
# include "cores/include.h"
# include "cores/ChannelF.h"

namespace OSCR::Cores::ChannelF
{
  // Fairchild Channel F
  // Cartridge Pinout
  // 22P (27P Width) 2.54mm pitch connector
  //
  //   TOP            BOTTOM
  //  SIDE            SIDE
  //        +-------+
  //        |    == |
  //        |     1 |- GND
  //        |     2 |- GND
  //        |     3 |- D0
  //        |     4 |- D1
  //        |     5 |- /INTREQ
  //        |     6 |- ROMC0
  //        |     7 |- ROMC1
  //        |     8 |- ROMC2
  //        |     9 |- D2
  //        |    10 |- ROMC3
  //        |    11 |- D3
  //        |    == |
  //        |    == |
  //        |    == |
  //        |    12 |- ROMC4
  //        |    13 |- PHI
  //        |    14 |- D4
  //        |    15 |- WRITE
  //        |    16 |- D5
  //        |    17 |- D6
  //        |    18 |- D7
  //        |    19 |- VDD(+5V)
  //        |    20 |- VDD(+5V)
  //        |    21 |- NC
  //        |    22 |- VGG(+12V)
  //        |    == |
  //        +-------+
  //
  //                                               TOP
  //       +----------------------------------------------------------------------------------+
  //       |                                                                                  |
  // LEFT  |                                                                                  | RIGHT
  //       | == 22 21 20 19 18 17 16 15 14 13 12 == == == 11 10  9  8  7  6  5  4  3  2  1 == |
  //       +----------------------------------------------------------------------------------+
  //                                              BOTTOM
  //

  // CONTROL PINS:
  // PHI(PH3)     - SNES /CS
  // /INTREQ(PH4) - SNES /IRQ
  // WRITE(PH5)   - SNES /WR
  // ROMC0(PF0)   - SNES A0
  // ROMC1(PF1)   - SNES A1
  // ROMC2(PF2)   - SNES A2
  // ROMC3(PF3)   - SNES A3
  // ROMC4(PF4)   - SNES A4

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  /******************************************
    Defines
  *****************************************/
  #define PHI_HI PORTH |= (1 << 3)
  #define PHI_LOW PORTH &= ~(1 << 3)
  #define WRITE_HI PORTH |= (1 << 5)
  #define WRITE_LOW PORTH &= ~(1 << 5)

  constexpr uint8_t const romSizes[] = {
    2,
    3,
    4,
    6,
  };

  // EEPROM MAPPING
  // 08 ROM SIZE

  //******************************************
  //  Menu
  //******************************************
  // Base Menu
  constexpr char const PROGMEM OPTION_03[] = "Read 16K";

  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::SelectCart,
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::SetSize,
    OPTION_03,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::ChannelF), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        break;

      case 1: // Read ROM
        readROM();
        break;

      case 2:
        // Set Size
        setROMSize();
        break;

      case 3: // Read 16K
        read16K();
        break;

      case 4: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::ChannelF));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // Channel F uses A0-A4 [A5-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       ---(PH0)   ---(PH1)   PHI(PH3) /INTREQ(PH4) WRITE(PH5) ---(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Unused Control Pins to HIGH
    //       ---(PH0)   ---(PH1)   PHI(PH3) /INTREQ(PH4) WRITE(PH5)   ---(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTK = 0xFF;       // A8-A15
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
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::ChannelF));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ CODE
  //******************************************

  // Sean Riddle Dumper Routine
  // clear PC0 with ROMC state 8
  // loop 256 times
  // fetch 16 bytes into buffer with ROMC state 0
  // dump buffer to serial port
  // clear PC0

  // Clear PC0
  void clearRegister()
  {
    PHI_LOW;
    WRITE_LOW;
    PORTF = 0;  // ROMC3 LOW

    delay(2000);

    PHI_HI;
    WRITE_HI;
    NOP;
    NOP;
    NOP;

    PHI_LOW;
    NOP;
    NOP;
    NOP;

    WRITE_LOW;
    PHI_HI;
    PORTF = 0;  // ROMC3 LOW
    NOP;
    NOP;
    NOP;

    PHI_LOW;
    NOP;
    NOP;
    NOP;

    PORTF = 0;  // ROMC3 LOW
    PHI_HI;
    PORTF = 0x8;  // this puts us in ROMC state 8 - clear PC0
    NOP;
    NOP;
    NOP;

    PHI_LOW;
    NOP;
    NOP;
    NOP;

    PORTF = 0x08;  // ROMC3 HIGH
    PHI_HI;
    PORTF = 0x08;  // ROMC3 HIGH
    NOP;
    NOP;
    NOP;

    PHI_LOW;
    NOP;
    NOP;
    NOP;

    PHI_HI;
    WRITE_HI;
    NOP;
    NOP;
    NOP;

    PHI_LOW;
    NOP;
    NOP;
    NOP;

    WRITE_LOW;
    PHI_HI;
    PORTF = 0;  // ROMC3 LOW
    NOP;
    NOP;
    NOP;

    PHI_LOW;
    NOP;
    NOP;
    NOP;

    WRITE_LOW;
    PHI_HI;
    PORTF = 0;  // ROMC3 LOW
    NOP;
    NOP;
    NOP;

    PHI_LOW;
    NOP;
    NOP;
    NOP;

    WRITE_LOW;
  }

  void setROMC(uint8_t command)
  {
    PHI_LOW;
    WRITE_LOW;
    NOP;

    WRITE_HI;
    PHI_HI;
    NOP;

    PHI_LOW;
    NOP;
    NOP;

    WRITE_LOW;
    PHI_HI;
    NOP;
    NOP;

    // PWs = 4 PHI Cycles
    // PWl = 6 PHI Cycles
    for (int x = 0; x < 2; x++) // 2 PHI
    {
      PHI_LOW;
      NOP;
      NOP;
      PHI_HI;
      NOP;
      NOP;
    }

    PORTF = command;  // ROMC3 = command

    for (int x = 0; x < 3; x++) // 4 PHI
    {
      PHI_LOW;
      NOP;
      NOP;
      PHI_HI;
      NOP;
      NOP;
    }

    PHI_LOW;
    NOP;
    NOP;

    PHI_HI;
    WRITE_HI;
    NOP;

    PHI_LOW;
    NOP;
    NOP;

    PHI_HI;
    WRITE_LOW;
    NOP;

    PHI_LOW;
    NOP;
    NOP;
  }

  void setREAD()
  {
    PHI_LOW;
    WRITE_LOW;
    NOP;

    WRITE_HI;
    PHI_HI;
    NOP;

    PHI_LOW;
    NOP;
    NOP;

    WRITE_LOW;
    PHI_HI;
    NOP;
    NOP;

    // PWs = 4 PHI Cycles
    // PWl = 6 PHI Cycles
    for (int x = 0; x < 2; x++) // 2 PHI
    {
      PHI_LOW;
      NOP;
      NOP;
      PHI_HI;
      NOP;
      NOP;
    }

    PORTF = 0;  // ROMC3 = 0 = Fetch Data
  }

  uint8_t readData()
  {
    for (int x = 0; x < 3; x++) // 4 PHI
    {
      PHI_LOW;
      NOP;
      NOP;
      PHI_HI;
      NOP;
      NOP;
    }

    PHI_LOW;
    NOP;
    NOP;

    PHI_HI;
    WRITE_HI;
    NOP;

    PHI_LOW;
    NOP;
    NOP;

    PHI_HI;
    WRITE_LOW;
    NOP;

    uint8_t ret = PINC;  // read databus into buffer

    PHI_LOW;
    NOP;
    NOP;

    return ret;
  }

  void readROM()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::ChannelF), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    uint32_t cartsize = romSizes[romSize] * 0x400;
    uint8_t blocks = cartsize / 0x200;
    setROMC(0x8);  // Clear PC0
    setREAD();

    // ROM Start Bytes
    // 0x55,0x08 - desert fox, muehle, space war, tic-tac-toe (all 2K)
    // 0x55,0x2B - most carts
    // 0x55,0xAA - alien invasion (4K)
    // 0x55,0xBB - video whizball (3K)
    for (uint16_t y = 0; y < 0x4800; y++)
    {
      uint8_t startbyte = readData();

      if (startbyte == 0x55) // Start Byte
      {
        OSCR::Storage::Shared::buffer[0] = startbyte;

        startbyte = readData();

        if ((startbyte == 0x08) || (startbyte == 0x2B) || (startbyte == 0xAA) || (startbyte == 0xBB))
        {
          OSCR::Storage::Shared::buffer[1] = startbyte;

          for (int w = 2; w < 512; w++)
          {
            startbyte = readData();

            OSCR::Storage::Shared::buffer[w] = startbyte;
          }

          OSCR::Storage::Shared::writeBuffer();

          delay(1);  // Added delay

          startbyte = OSCR::Storage::Shared::buffer[1]; // Restore byte for 3K Hangman Check

          for (int z = 1; z < blocks; z++)
          {
            if ((cartsize == 0x0C00) && (startbyte == 0x2B)) // 3K Hangman
            {
              // Skip SRAM Code for 3K Hangman Cart
              // Hangman uses an F21022PC 1K SRAM Chip at 0x0400
              // SRAM is NOT Battery Backed so contents change
              // Chips are organized: 1K ROM + 1K SRAM + 1K ROM + 1K ROM
              if (z == 2)
              {
                for (int x = 0; x < 0x0A00; x++) // Skip 1K SRAM at 0x0400
                {
                  readData();
                }
              }
            }
            else if (cartsize == 0x1000) // 4K
            {
              // Skip BIOS/Blocks Code for 4K Carts - Tested with Alien Invasion/Pro Football
              // Alien Invasion/Pro Football both use a DM74LS02N (Quad 2-Input NOR Gate) with two 2K ROM Chips
              uint16_t offset = 0x800 + (z * 0x200);

              for (uint16_t x = 0; x < offset; x++) // Skip BIOS/Previous Blocks
              {
                readData();
              }
            }

            for (int w = 0; w < 512; w++)
            {
              uint8_t temp = readData();
              OSCR::Storage::Shared::buffer[w] = temp;
            }

            OSCR::Storage::Shared::writeBuffer();

            delay(1); // Added delay
          }
          break;
        }
      }
    }

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();
  }

  // Read 16K Bytes
  void read16K()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::ChannelF), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    uint32_t cartsize = romSizes[romSize] * 0x400;

    for (uint16_t y = 0; y < 0x20; y++)
    {
      if (cartsize == 0x1000) // 4K
      {
        // Skip BIOS/Blocks Code for 4K Carts - Tested with Alien Invasion/Pro Football
        // Alien Invasion/Pro Football both use a DM74LS02N (Quad 2-Input NOR Gate) with two 2K ROM Chips
        // IF CASINO POKER DOES NOT DUMP PROPERLY USING READROM
        // TEST BY SETTING ROM SIZE TO 2K AND 4K THEN COMPARE 16K DUMPS

        uint16_t offset = 0x800 + (y * 0x200);

        for (uint16_t x = 0; x < offset; x++) // Skip BIOS/Previous Blocks
        {
          readData();
        }
      }

      for (int w = 0; w < 512; w++)
      {
        uint8_t temp = readData();

        OSCR::Storage::Shared::buffer[w] = temp;
      }

      OSCR::Storage::Shared::writeBuffer();

      delay(1);  // Added delay
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
    OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes, sizeofarray(romSizes));

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
} /* namespace OSCR::Cores::ChannelF */

#endif /* ENABLE_FAIRCHILD */
