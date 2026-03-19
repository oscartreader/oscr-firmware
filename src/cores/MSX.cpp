//******************************************
// MSX COMPUTER MODULE
//******************************************
#include "config.h"

#if HAS_MSX
# include "cores/include.h"
# include "cores/MSX.h"

namespace OSCR::Cores::MSX
{
  // MSX
  // Cartridge Pinout
  // 50P 2.54mm pitch connector
  //
  //       FRONT            BACK
  //        SIDE            SIDE
  //              +-------+
  //        /CS2 -| 2   1 |- /CS1
  //      /SLTSL -| 4   3 |- /CS12
  //       /RFSH -| 6   5 |- RSV(NC)
  //        /INT -| 8   7 |- /WAIT
  //     /BUSDIR -| 10  9 |- /M1
  //       /MERQ -| 12 11 |- /IORQ
  //         /RD -| 14 13 |- /WR
  //     RSV(NC) -| 16 15 |- /RESET
  //         A15 -| 18 17 |- A9
  //         A10 -| 20 19 |- A11
  //          A6 -| 22 21 |- A7
  //          A8 -| 24 23 |- A12
  //          A13-| 26 25 |- A14
  //          A0 -| 28 27 |- A1
  //          A2 -| 30 29 |- A3
  //          A4 -| 32 31 |- A5
  //          D0 -| 34 33 |- D1
  //          D2 -| 36 35 |- D3
  //          D4 -| 38 37 |- D5
  //          D6 -| 40 39 |- D7
  //       CLOCK -| 42 41 |- GND
  //         SW1 -| 44 43 |- GND
  //         SW2 -| 46 45 |- +5V
  //        +12V -| 48 47 |- +5V
  //        -12V -| 50 49 |- SOUNDIN
  //              +-------+
  //
  //                                           BACK
  //      +----------------------------------------------------------------------------+
  //      | 49 47 45 43 41 39 37 35 33 31 29 27 25 23 21 19 17 15 13 11  9  7  5  3  1 |
  // LEFT |                                                                            | RIGHT
  //      | 50 48 46 44 42 40 38 36 34 32 30 28 26 24 22 20 18 16 14 12 10  8  6  4  2 |
  //      +----------------------------------------------------------------------------+
  //                                           FRONT
  //
  // CONTROL PINS:
  // /RESET(PH0) - SNES RESET
  // CLOCK(PH1)  - SNES CPUCLK
  // /SLTSL(PH3) - SNES /CS
  // /MERQ(PH4)  - SNES /IRQ
  // /WR(PH5)    - SNES /WR
  // /RD(PH6)    - SNES /RD
  // /CS1(PL0)   - SNES A16
  // /CS2(PL1)   - SNES A17
  // /CS12(PL2)  - SNES A18

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  using OSCR::Databases::Basic::mapperDetail;

  //******************************************
  //  Defines
  //******************************************
  #define CS1_DISABLE PORTL |= (1 << 0)  // ROM SELECT 4000-7FFF
  #define CS1_ENABLE PORTL &= ~(1 << 0)
  #define CS2_DISABLE PORTL |= (1 << 1)  // ROM SELECT 8000-BFFF
  #define CS2_ENABLE PORTL &= ~(1 << 1)
  #define CS12_DISABLE PORTL |= (1 << 2)  // ROM SELECT 4000-BFFF
  #define CS12_ENABLE PORTL &= ~(1 << 2)
  #define MERQ_DISABLE PORTH |= (1 << 4)
  #define MERQ_ENABLE PORTH &= ~(1 << 4)

  // MSX1 = 8,16,32,128,256
  // MSX2 = 32,64,128,256,512,1024
  constexpr uint16_t const romSizes[] = {
    0,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
  };

  uint8_t romSizeHigh = sizeofarray(romSizes) - 1;  // Highest Entry

  constexpr uint8_t const ramSizes[] = {
    0,
    2,
    8,
    16,
    32,
  };

  uint8_t ramSizeHigh = sizeofarray(ramSizes) - 1;  // Highest Entry

  uint8_t msxmapper;
  uint8_t chipselect;

  bool srambit5 = false;
  bool srambit6 = false;
  bool srambit7 = false;

  // EEPROM MAPPING
  // 07 MAPPER
  // 08 ROM SIZE
  // 10 RAM SIZE

  //******************************************
  //  MENU
  //******************************************
  // Base Menu
  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::SelectCart,
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::SetMapper,
    OSCR::Strings::MenuOptions::WriteSave,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::MSX), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        break;

      case 1: // Read ROM + Read RAM
        if (romSize)
        {
          readROM();
        }

        if (sramSize)
        {
          readRAM();
        }
        break;

      case 2: // Set Mapper + Size
        setMapper();
        break;

      case 3: // Write RAM
        if (!sramSize)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        writeRAM();
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::MSX));
  }

  //******************************************
  //  SETUP
  //******************************************
  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // MSX uses A0-A15
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23 - Use A16-A18 for /CS1, /CS2, /CS12
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       /RST(PH0) CLOCK(PH1) /SLTSL(PH3) /MERQ(PH4) /WR(PH5)   /RD(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       /RST(PH0) CLOCK(PH1) /SLTSL(PH3) /MERQ(PH4) /WR(PH5)   /RD(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set /SLTSL(PH3) to LOW
    PORTH &= ~(1 << 3);

    // Set /CS1, /CS2, /CS12 to HIGH
    PORTL = 0xFF;  // A16-A23 (A16 = /CS1, A17 = /CS2, A18 = /CS12)

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
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
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::MSX));
    OSCR::Databases::Basic::setupMapper(F("msx-mappers"));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ DATA
  //******************************************
  uint8_t readData(uint16_t addr)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
    NOP;
    NOP;

    // Set /SLTSL(PH3) to LOW
    //  PORTH &= ~(1 << 3);

    // Set /RD to LOW
    PORTH &= ~(1 << 6);  // /RD LOW (ENABLE)
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

    // Pull /RD to HIGH
    PORTH |= (1 << 6);  // /RD HIGH (DISABLE)

    // Set /SLTSL(PH3) to HIGH
    //  PORTH |= (1 << 3);
    //  NOP; NOP;

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
  // WRITE DATA
  //******************************************
  void writeData(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A15
    NOP;
    NOP;

    DDRC = 0xFF;  // Set to Output
    PORTC = data;
    NOP;
    NOP;
    NOP;

    // Set /WR(PH5) to LOW
    PORTH &= ~(1 << 5);
    NOP;
    NOP;
    NOP;

    // Set /WR(PH5) to HIGH
    PORTH |= (1 << 5);
    NOP;
    NOP;

    DDRC = 0x00;  // Reset to Input
  }

  //******************************************
  // CS CODE
  //******************************************
  void setCS()  // Set CS Line
  {
    chipselect = 0;

    for (int x = 0; x < 4; x++)
    {
      uint8_t check0 = readData(0x4000);
      uint8_t check1 = readData(0x4001);

      if ((check0 == 0x41) && (check1 == 0x42))
        break;

      chipselect++;

      enableCS();
    }
  }

  void checkCS()  // Check for 2nd Chip
  {
    if (chipselect == 1)
    {
      uint8_t check0 = readData(0x8000);
      uint8_t check1 = readData(0x8001);

      if ((check0 == 0x41) && (check1 == 0x42))
      {
        disableCS();

        chipselect = 2;

        enableCS();
      }
    }
  }

  void enableCS()
  {
    if (chipselect == 1)
      CS1_ENABLE;
    else if (chipselect == 2)
      CS2_ENABLE;
    else if (chipselect == 3)
      CS12_ENABLE;
  }

  void disableCS()
  {
    CS1_DISABLE;
    CS2_DISABLE;
    CS12_DISABLE;
  }

  //******************************************
  // READ ROM
  //******************************************
  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::MSX), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::Raw));

    cartOn();

    switch (msxmapper)
    {
    case 0:  // No Mapper
      disableCS();

      if (romSize == 4)                   // 64K
        readSegment(0x0000, 0x4000);  // +16K

      setCS();

      readSegment(0x4000, 0x6000);  // 8K

      if (romSize > 1)
        readSegment(0x6000, 0x8000);  // +8K = 16K

      if (romSize > 2)
      {
        checkCS();                        // Check for 2nd Chip
        readSegment(0x8000, 0xC000);  // +16K = 32K
      }

      disableCS();

      if (romSize == 4)                    // 64K
        readSegment(0xC000, 0x10000);  // +16K

      break;

    case 1: // ASCII8 (64K/128K/256K/512K)
    case 7: // Koei (256K/512K/1024K)
      numBanks = OSCR::Util::power<2>(romSize - 1);

      for (uint32_t x = 0; x < numBanks; x += 4)
      {
        writeData(0x6000, x);
        readSegment(0x4000, 0x6000);  // 8K Init Bank 0

        writeData(0x6800, x + 1);
        readSegment(0x6000, 0x8000);  // 8K Init Bank 0

        writeData(0x7000, x + 2);
        readSegment(0x8000, 0xA000);  // 8K Init Bank 0

        writeData(0x7800, x + 3);
        readSegment(0xA000, 0xC000);  // 8K Init Bank 0
      }
      break;

    case 2: // ASCII16 (64K/128K/256K/512K)
      numBanks = OSCR::Util::power<2>(romSize - 1) / 2;

      for (uint32_t x = 0; x < numBanks; x += 2)
      {
        writeData(0x6000, x);
        readSegment(0x4000, 0x8000);  // 16K Init Bank 0

        writeData(0x7000, x + 1);
        readSegment(0x8000, 0xC000);  // 16K Init Bank 0
      }
      break;

    case 3:  // Cross Blaim (64K)
      CS1_ENABLE;
      readSegment(0x4000, 0x8000);  // 16K Fixed Bank 0
      CS1_DISABLE;

      CS2_ENABLE;

      for (uint8_t x = 1; x < 4; x++)
      {
        writeData(0x4045, x);
        readSegment(0x8000, 0xC000);  // 16K Init Bank 1
      }

      CS2_DISABLE;
      break;

    case 4: // Game Master 2 (128K)
      readSegment(0x4000, 0x6000);  // 8K Fixed Bank 0
      writeData(0x6000, 1);         // Set Bank 1 for subsequent reads
      readSegment(0x6000, 0x8000);  // 8K Init Bank 1

      for (uint8_t x = 2; x < 16; x += 2)
      {
        writeData(0x8000, x);
        readSegment(0x8000, 0xA000);  // 8K
        writeData(0xA000, x + 1);
        readSegment(0xA000, 0xC000);  // 8K
      }
      break;

    case 5: // HAL Note (1024K)
      MERQ_ENABLE;

      // Dummy Read - Needed to prevent random bytes
      for (uint8_t y = 0; y < 32; y++)
      {
        writeData(0x4FFF, y);

        for (uint32_t addr = 0x4000; addr < 0x6000; addr++)
          readData(addr);  // Dummy Read
      }

      // READ
      for (uint8_t x = 0; x < 128; x++)
      {
        writeData(0x4FFF, x);
        readSegment(0x4000, 0x6000);  // 8K Init Bank 0
      }

      MERQ_DISABLE;
      break;

    case 6: // Harry Fox (64K)
      CS1_ENABLE;
      writeData(0x6000, 0);
      readSegment(0x4000, 0x8000);  // 16K Init Bank 0
      CS1_DISABLE;

      CS2_ENABLE;
      writeData(0x7000, 0);
      readSegment(0x8000, 0xC000);  // 16K Init Bank 1
      CS2_DISABLE;

      CS1_ENABLE;
      writeData(0x6000, 1);
      readSegment(0x4000, 0x8000);  // 16K
      CS1_DISABLE;

      CS2_ENABLE;
      writeData(0x7000, 1);
      readSegment(0x8000, 0xC000);  // 16K
      CS2_DISABLE;
      break;

    case 8:  // Konami MegaROM without SCC
      readSegment(0x4000, 0x6000); // 8K Fixed Bank 0
      readSegment(0x6000, 0x8000); // 8K Init Bank 1

      numBanks = OSCR::Util::power<2>(romSize - 1);

      for (uint32_t x = 2; x < numBanks; x += 2)
      {
        writeData(0x8000, x);
        readSegment(0x8000, 0xA000);  // 8K

        writeData(0xA000, x + 1);
        readSegment(0xA000, 0xC000);  // 8K
      }
      break;

    case 9:  // Konami MegaROM with SCC
      numBanks = OSCR::Util::power<2>(romSize - 1);

      for (uint32_t x = 0; x < numBanks; x += 4)
      {
        writeData(0x5000, x);
        readSegment(0x4000, 0x6000);  // 8K

        writeData(0x7000, x + 1);
        readSegment(0x6000, 0x8000);  // 8K

        writeData(0x9000, x + 2);
        readSegment(0x8000, 0xA000);  // 8K

        writeData(0xB000, x + 3);
        readSegment(0xA000, 0xC000);  // 8K
      }
      break;

    case 10:  // MSX-DOS2 (64K)
      MERQ_ENABLE;
      CS1_ENABLE;

      for (int x = 0; x < 4; x++)
      {
        writeData(0x7FFE, x);         // Official v2.20
        readSegment(0x4000, 0x8000);  // 16K Init Bank 0
      }

      CS1_DISABLE;
      MERQ_DISABLE;
      break;

    case 11:  // FM-PAC (64K)
      CS1_ENABLE;

      for (int x = 0; x < 4; x++)
      {
        writeData(0x7FF7, x);
        readSegment(0x4000, 0x8000);  // 16K
      }

      CS1_DISABLE;
      break;

    case 12:  // R-TYPE (384K)
      for (int x = 0; x < 23; x++)
      {
        writeData(0x7000, x);
        readSegment(0x8000, 0xC000);  // 16K Init Bank 0
      }

      readSegment(0x4000, 0x8000);  // 16K Init Bank 0F
      break;

    case 13:  // Super Lode Runner (128K)
      MERQ_ENABLE;
      CS2_ENABLE;

      for (int x = 0; x < 8; x++)
      {
        writeData(0x0000, x);
        readSegment(0x8000, 0xC000);  // 16K Init Bank 0
      }

      CS2_DISABLE;
      MERQ_DISABLE;
      break;

    case 14: // Hudson Soft Bee Pack (16K/32K)
      CS1_ENABLE;
      readSegment(0x4000, 0x8000); // 16K Bank 0
      CS1_DISABLE;

      if (romSize == 3) // 32K
      {
        CS2_ENABLE;
        readSegment(0x8000, 0xC000); // +16K Bank 1
        CS2_DISABLE;
      }
      break;
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();
  }

  //******************************************
  // TEST/CHECK RAM
  //******************************************
  bool testRAM(uint8_t enable1, uint8_t enable2)
  {
    uint8_t test1, test2;

    for (int x = 0; x < 0x10; x++)
    {
      // Test 16 Bytes
      writeData(0x7000, enable1);
      test1 = readData(0x8000 + x);

      writeData(0x7000, enable2);
      test2 = readData(0x8000 + x);

      if (test1 != test2)
      {
        return false;
      }
    }

    return true;
  }

  bool checkRAM()
  {
    OSCR::UI::print(FS(OSCR::Strings::Common::RAM));
    OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    // Test carts to identify SRAM Enable Bits (Bit 5/6/7)
    // Bit 5 Test
    srambit5 = testRAM(0x20, 0xB0);

    if (!srambit5)
    {
      // Bit 6 Test
      srambit6 = testRAM(0x40, 0xC0);

      if (!srambit6)
      {
        // Bit 7 Test
        srambit7 = testRAM(0x80, 0xF0);

        if (!srambit7)
        {
          OSCR::UI::error(FS(OSCR::Strings::Common::FAIL));
          return false;
        }
      }
    }

    OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));

    return true;
  }

  //******************************************
  // READ RAM
  //******************************************
  void readRAM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::MSX), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::SaveRAM));

    cartOn();

    switch (msxmapper)
    {
    case 1:  // ASCII8 (2K/8K)
      // ASCII8 carts use different SRAM Enable Bits
      // Bit 4 (0x10) - Dires (2K)
      // Bit 5 (0x20) - Xanadu
      // Bit 7 (0x80) - Wizardry

      if (sramSize == 1) // Dires (2K)
      {
        writeData(0x7000, 0x10);      // Bit 4
        readSegment(0x8000, 0x8800);  // 2K
      }
      else
      {
        // Combine SRAM Enable Bit 5 + Bit 7 = 0xA0
        writeData(0x7000, 0xA0);      // Bit 5 + Bit 7
        readSegment(0x8000, 0xA000);  // 8K
      }

      writeData(0x7000, 0);  // SRAM Disable
      break;

    case 2:  // ASCII16 (2K/8K)
      writeData(0x7000, 0x10); // Bit 4 Enable
      readSegment(0x8000, 0x8800); // 2K - Hydlide 2 & Daisenryaku (2K)

      if (sramSize == 2) // A-Train (8K)
      {
        readSegment(0x8800, 0xA000); // +6K = 8K
      }

      writeData(0x7000, 0); // SRAM Disable
      break;

    case 4:  // Game Master 2 (8K)
      writeData(0xA000, 0x10); // Bit 4 Enable, Bit 5 SRAM Segment 0
      readSegment(0xB000, 0xC000); // 4K

      writeData(0xA000, 0x30); // Bit 4 Enable, Bit 5 SRAM Segment 1
      readSegment(0xB000, 0xC000); // 4K

      writeData(0xA000, 0); // SRAM Disable
      break;

    case 5:  // HAL Note (16K)
      MERQ_ENABLE;

      writeData(0x4FFF, 0x80);      // Bit 7 Enable
      readSegment(0x0000, 0x4000);  // 16K

      writeData(0x4FFF, 0);         // SRAM Disable

      MERQ_DISABLE;
      break;

    case 7:  // Koei (8K/32K) Nobunaga no Yabou - Zenkoku Ban (8K) & Sangokushi II (32K)
      // Koei carts use different SRAM Enable Bits
      // Bit 5 (0x20) - Nobunaga no Yabou - Zenkoku Ban MSX
      // Bit 6 (0x40) - Nobunaga no Yabou - Zenkoku Ban MSX2
      // Bit 7 (0x80) - Sangokushi II
      // Use Combined Bits:  0xA0 (Bit 5 + Bit 7) and 0xC0 (Bit 6 + Bit 7)
      // Note:  Combined 0xE0 (Bit 5 + Bit 6 + Bit 7) does not work
      if (!checkRAM()) return;

      if (srambit6)
      {
        writeData(0x7000, 0xC0);  // Bit 6 + Bit 7 Enable
      }
      else
      {
        writeData(0x7000, 0xA0);    // Bit 5 + Bit 7 Enable
      }

      readSegment(0x8000, 0xA000);  // 8K

      if (sramSize > 2)
      {
        for (int x = 1; x < 4; x++)
        {
          if (srambit6)
          {
            writeData(0x7000, 0xC0 + x);  // Bit 6 + Bit 7 Enable
          }
          else
          {
            writeData(0x7000, 0xA0 + x);  // Bit 5 + Bit 7 Enable
          }

          readSegment(0x8000, 0xA000);    // 8K
        }
      }

      writeData(0x7000, 0);  // SRAM Disable
      break;

    case 11:  // PAC/FM-PAC (8K)
      writeData(0x5FFE, 0x4D);      // SRAM Enable Step 1
      writeData(0x5FFF, 0x69);      // SRAM Enable Step 2

      readSegment(0x4000, 0x6000);  // 8K

      writeData(0x5FFE, 0);         // SRAM Disable
      break;
    }

    cartOff();

    OSCR::Storage::Shared::close();
  }

  //******************************************
  // WRITE RAM
  //******************************************
  void writeRAM()
  {
    printHeader();

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    switch (msxmapper)
    {
    case 1:  // ASCII8 (2K/8K)
      for (uint16_t address = 0x0; address < (0x800 * sramSize * sramSize); address += 512) // 2K/8K
      {
        if (sramSize == 1)
        {
          writeData(0x7000, 0x10);  // Bit 4
        }
        else
        {
          writeData(0x7000, 0xA0);  // Bit 5 + Bit 7
        }

        OSCR::Storage::Shared::fill();

        for (int x = 0; x < 512; x++)
        {
          writeData(0x8000 + address + x, OSCR::Storage::Shared::buffer[x]);
        }
      }

      writeData(0x7000, 0);  // SRAM Disable

      break;

    case 2:  // ASCII16 (2K/8K)
      writeData(0x7000, 0x10);  // Bit 4 Enable

      for (uint16_t address = 0x0; address < (0x800 * sramSize * sramSize); address += 512) // 2K/8K
      {
        OSCR::Storage::Shared::fill();

        for (int x = 0; x < 512; x++)
        {
          writeData(0x8000 + address + x, OSCR::Storage::Shared::buffer[x]);
        }
      }

      writeData(0x7000, 0);  // SRAM Disable

      break;

    case 4:  // Game Master 2 (8K)
      for (int y = 0; y < 2; y++)
      {
        writeData(0xA000, 0x10 + (y * 0x20));  // Bit 4 Enable, Bit 5 SRAM Segment 0/1

        for (uint16_t address = 0x0; address < 0x1000; address += 512) // 4K
        {
          OSCR::Storage::Shared::fill();

          for (int x = 0; x < 512; x++)
          {
            writeData(0xB000 + address + x, OSCR::Storage::Shared::buffer[x]);
          }
        }
      }

      writeData(0xA000, 0);  // SRAM Disable

      break;

    case 5:  // HAL Note (16K)
      MERQ_ENABLE;

      writeData(0x4FFF, 0x80);  // Bit 7 Enable

      for (uint16_t address = 0; address < 0x4000; address += 512) // 16K
      {
        OSCR::Storage::Shared::fill();

        for (int x = 0; x < 512; x++)
        {
          writeData(address + x, OSCR::Storage::Shared::buffer[x]);
        }
      }

      writeData(0x4FFF, 0);  // SRAM Disable

      MERQ_DISABLE;

      break;

    case 7:  // Koei (8K/32K)
      if (!checkRAM()) return;

      if (srambit6)
        writeData(0x7000, 0xC0);  // Bit 6 + Bit 7 Enable
      else
        writeData(0x7000, 0xA0);  // Bit 5 + Bit 7 Enable

      for (uint16_t address = 0x0; address < 0x2000; address += 512) // 8K
      {
        OSCR::Storage::Shared::fill();

        for (int x = 0; x < 512; x++)
        {
          writeData(0x8000 + address + x, OSCR::Storage::Shared::buffer[x]);
        }
      }

      if (sramSize > 2)
      {
        for (int y = 1; y < 4; y++)
        {
          if (srambit6)
            writeData(0x7000, 0xC0 + y);  // Bit 6 + Bit 7 Enable
          else
            writeData(0x7000, 0xA0 + y);  // Bit 5 + Bit 7 Enable

          for (uint16_t address = 0x0; address < 0x2000; address += 512) // 8K
          {
            OSCR::Storage::Shared::fill();

            for (int x = 0; x < 512; x++)
            {
              writeData(0x8000 + address + x, OSCR::Storage::Shared::buffer[x]);
            }
          }
        }
      }
      writeData(0x7000, 0);  // SRAM Disable
      break;

    case 11:  // PAC/FM-PAC (8K)
      writeData(0x5FFE, 0x4D);  // SRAM Enable Step 1
      writeData(0x5FFF, 0x69);  // SRAM Enable Step 2

      for (uint16_t address = 0x0; address < 0x2000; address += 512) // 8K
      {
        OSCR::Storage::Shared::fill();

        for (int x = 0; x < 512; x++)
        {
          writeData(0x4000 + address + x, OSCR::Storage::Shared::buffer[x]);
        }
      }
      writeData(0x5FFE, 0);  // SRAM Disable
      break;
    }

    OSCR::Storage::Shared::close();
  }

  //******************************************
  // MAPPER CODE
  //******************************************

  void setMapper()
  {
    fromCRDB = false;

    OSCR::Databases::Basic::browseMappers();

    if (mapperDetail->sizeLow > romSizeHigh || mapperDetail->sizeHigh > romSizeHigh || mapperDetail->sizeLow > mapperDetail->sizeHigh)
    {
      OSCR::UI::fatalErrorInvalidData();
    }

    msxmapper = mapperDetail->id;

    if (mapperDetail->sizeLow == mapperDetail->sizeHigh)
    {
      romSize = mapperDetail->sizeLow;
    }
    else
    {
      romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes + mapperDetail->sizeLow, mapperDetail->sizeHigh - mapperDetail->sizeLow);
    }

    if (mapperDetail->meta1 == mapperDetail->meta2)
    {
      sramSize = mapperDetail->meta1;
    }
    else
    {
      sramSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectSaveSize), FS(OSCR::Strings::Templates::SizeKB), ramSizes + mapperDetail->meta1, mapperDetail->meta2 - mapperDetail->meta1);
    }

    printHeader();

    OSCR::UI::printValue(OSCR::Strings::Common::Mapper, mapperDetail->id);
    OSCR::UI::printSize(OSCR::Strings::Common::ROM, romSizes[romSize] * 1024);
    OSCR::UI::printSize(OSCR::Strings::Common::RAM, ramSizes[sramSize] * 1024);
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
    msxmapper = romDetail->mapper;

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::MSX */
#endif
