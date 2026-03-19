//******************************************
// BENESSE POCKET CHALLENGE W MODULE
//******************************************
#include "config.h"

#if HAS_PCW
# include "cores/include.h"
# include "cores/PocketChallengeW.h"

namespace OSCR::Cores::PocketChallengeW
{
  // Benesse Pocket Challenge W
  // Cartridge Pinout
  // 38P 1.27mm pitch connector
  //
  // CART            CART
  // TOP             EDGE
  //     +---------+
  //     |   1   |-- VCC
  //     |   2   |-- GND
  //     |   3   |-- AD0 (A0 IN/D0 OUT)
  //     |   4   |-- AD1 (A1 IN/D1 OUT)
  //     |   5   |-- AD2 (A2 IN/D2 OUT)
  //     |   6   |-- AD3 (A3 IN/D3 OUT)
  //     |   7   |-- AD4 (A4 IN/D4 OUT)
  //     |   8   |-- AD5 (A5 IN/D5 OUT)
  //     |   9   |-- AD6 (A6 IN/D6 OUT)
  //     |  10   |-- AD7 (A7 IN/D7 OUT)
  //     |  11   |-- A08
  //     |  12   |-- A09
  //     |  13   |-- A10
  //     |  14   |-- A11
  //     |  15   |-- A12
  //     |  16   |-- A13
  //     |  17   |-- A14
  //     |  18   |-- A15
  //     |  19   |-- A16
  //     |  20   |-- A17
  //     |  21   |-- A18
  //     |  22   |-- A19
  //     |  23   |-- A20
  //     |  24   |-- A21
  //     |  25   |-- NC [CPU-INT2/T15(P47)]
  //     |  26   |-- 1B (7W00F) -> HIGH -+
  //     |  27   |-- 1A (7W00F) -> HIGH -+-> OE (ROM) = LOW
  //     |  28   |-- OE (LH5164A)
  //     |  29   |-- WE (LH5164A)
  //     |  30   |-- LE (HC573) [LATCH ENABLE FOR AD0-AD7]
  //     |  31   |-- NC [CPU-TO5(P43)]
  //     |  32   |-- NC [CPU-RESET]
  //     |  33   |-- VCC
  //     |  34   |-- GND
  //     |  35   |-- NC
  //     |  36   |-- NC [CPU-VCC]
  //     |  37   |-- NC [CONSOLE-GND]
  //     |  38   |-- NC
  //     +---------+

  // CONTROL PINS:
  // LE - (PH0) - PCW PIN 30 - SNES RST
  // 1A - (PH3) - PCW PIN 27 - SNES /CS
  // 1B - (PH4) - PCW PIN 26 - SNES /IRQ
  // WE - (PH5) - PCW PIN 29 - SNES /WR
  // OE - (PH6) - PCW PIN 28 - SNES /RD

  // NOT CONNECTED:
  // CLK(PH1) - N/C

  // ADDRESS PINS:
  // ADDR/DATA [AD0-AD7] - PORTC
  // ADDR A8-A15 - PORTK
  // ADDR A16-A21 - PORTL

  //******************************************
  // DEFINES
  //******************************************

  // CONTROL PINS - LE/1A/1B/WE/OE
  #define LE_HIGH PORTH |= (1 << 0)
  #define LE_LOW PORTH &= ~(1 << 0)
  #define NAND_1A_HIGH PORTH |= (1 << 3)
  #define NAND_1A_LOW PORTH &= ~(1 << 3)
  #define NAND_1B_HIGH PORTH |= (1 << 4)
  #define NAND_1B_LOW PORTH &= ~(1 << 4)  // Built-in RAM + I/O
  #define WE_HIGH PORTH |= (1 << 5)
  #define WE_LOW PORTH &= ~(1 << 5)
  #define OE_HIGH PORTH |= (1 << 6)
  #define OE_LOW PORTH &= ~(1 << 6)

  #define MODE_READ DDRC = 0      // [INPUT]
  #define MODE_WRITE DDRC = 0xFF  //[OUTPUT]

  #define DATA_READ \
    { \
      DDRC = 0; \
      PORTC = 0xFF; \
    }                             // [INPUT PULLUP]
  #define ADDR_WRITE DDRC = 0xFF  // [OUTPUT]
  #define DETECTION_SIZE 64

  bool multipack;
  uint8_t bank0;
  uint8_t bank1;

  //******************************************
  //  MENU
  //******************************************
  constexpr char const * const menuOptions[] PROGMEM = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::PocketChallengeW), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Read ROM
        check_multi();

        if (multipack)
          readMultiROM();
        else
          readSingleROM();

        break;

      case 1: // Read SRAM
        readSRAM();
        break;

      case 2: // Write SRAM
        writeSRAM();
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::PocketChallengeW));
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
    //A8-A15
    DDRK = 0xFF;
    //A16-A21
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       LE(PH0)    --(PH1)    1A(PH3)    1B(PH4)    WE(PH5)    OE(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Address/Data Pins AD0-AD7 (PC0-PC7) to Input
    MODE_READ;  // DDRC = 0

    // Setting Control Pins to HIGH
    //        LE(PH0)   ---(PH1)    1A(PH3)    1B(PH4)    WE(PH5)    OE(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Pins HIGH
    PORTJ |= (1 << 0);  // TIME(PJ0)

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::PocketChallengeW));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  //  LOW LEVEL FUNCTIONS
  //******************************************

  // Max ROM Size 0x400000 (Highest Address = 0x3FFFFF) - 3F FFFF
  // NAND 1A + 1B HIGH = LOW = ROM Output Enabled
  void read_setup()
  {
    NAND_1A_HIGH;
    NAND_1B_HIGH;
    OE_HIGH;
    WE_HIGH;
    LE_LOW;
  }

  // READ ROM BYTE WITH ADDITIONAL DELAY
  // NEEDED FOR PROBLEM CARTS TO SWITCH FROM ADDRESS TO DATA
  uint8_t read_rom_byte(uint32_t address)
  {
    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    // Latch Address on AD0-AD7
    ADDR_WRITE;
    LE_HIGH;                 // Latch Enable
    PORTC = address & 0xFF;  // A0-A7
    LE_LOW;                  // Address Latched
    __asm__("nop\n\t"
            "nop\n\t");
    // Read Data on AD0-AD7
    OE_LOW;
    DATA_READ;
    delayMicroseconds(5);  // 3+ Microseconds for Problem Carts
    uint8_t data = PINC;
    OE_HIGH;

    return data;
  }

  // SRAM Size 0x8000 (Highest Address = 0x7FFF)
  // NAND 1A LOW = SRAM Enabled [ROM DISABLED]
  uint8_t read_ram_byte_1A(uint32_t address)
  {
    NAND_1A_LOW;
    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    // Latch Address on AD0-AD7
    ADDR_WRITE;
    LE_HIGH;                 // Latch Enable
    PORTC = address & 0xFF;  // A0-A7
    LE_LOW;                  // Address Latched
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    // Read Data on AD0-AD7
    OE_LOW;
    DATA_READ;
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    uint8_t data = PINC;
    OE_HIGH;
    NAND_1A_HIGH;
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    return data;
  }

  // Toshiba TMP90C845A
  // 0xFEC0-0xFFBF Built-in RAM (256 bytes)
  // 0xFFC0-0xFFF7 Built-in I/O (56 bytes)
  // 0xFF00-0xFFBF (192 byte area available in direct addressing mode)
  // 0xFF18-0xFF68 (Micro DMA parameters (if used))

  // TEST CODE TO READ THE CPU BUILT-IN RAM + I/O
  // NAND 1B LOW = Built-In RAM + I/O Enabled [ROM DISABLED]
  uint8_t read_ram_byte_1B(uint32_t address)
  {
    NAND_1B_LOW;
    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    // Latch Address on AD0-AD7
    ADDR_WRITE;
    LE_HIGH;                 // Latch Enable
    PORTC = address & 0xFF;  // A0-A7
    LE_LOW;                  // Address Latched
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    // Read Data on AD0-AD7
    OE_LOW;
    DATA_READ;
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    uint8_t data = PINC;
    OE_HIGH;
    NAND_1B_HIGH;
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    return data;
  }

  // WRITE SRAM 32K
  void write_ram_byte_1A(uint32_t address, uint8_t data)
  {
    NAND_1A_LOW;
    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    // Latch Address on AD0-AD7
    ADDR_WRITE;
    LE_HIGH;                 // Latch Enable
    PORTC = address & 0xFF;  // A0-A7
    LE_LOW;                  // Address Latched
    // Write Data on AD0-AD7 - WE LOW ~240-248ns
    WE_LOW;
    PORTC = data;
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    WE_HIGH;
    NAND_1A_HIGH;
  }

  // WRITE CPU BUILT-IN RAM + I/O AREA
  // MODIFIED TO MATCH WORKING BANK SWITCH ROUTINE
  void write_ram_byte_1B(uint32_t address, uint8_t data)
  {
    NAND_1A_LOW;
    NAND_1A_HIGH;
    NAND_1B_LOW;
    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    // Latch Address on AD0-AD7
    ADDR_WRITE;
    LE_HIGH;                 // Latch Enable
    PORTC = address & 0xFF;  // A0-A7
    LE_LOW;                  // Address Latched
    // Write Data on AD0-AD7 - WE LOW ~740ns
    WE_LOW;
    PORTC = data;
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    WE_HIGH;
    NAND_1B_HIGH;
  }

  //******************************************
  //  SINGLE-PACK FUNCTIONS
  //******************************************

  uint32_t detect_rom_size()
  {
    uint8_t read_byte;
    uint8_t current_byte;
    uint8_t detect_1m, detect_2m;

    //Initialize variables
    detect_1m = 0;
    detect_2m = 0;

    //Confirm where mirror address starts from (1MB, 2MB or 4MB)
    for (current_byte = 0; current_byte < DETECTION_SIZE; current_byte++)
    {
      if ((current_byte != detect_1m) && (current_byte != detect_2m))
      {
        //If none matched, size is 4MB
        break;
      }

      read_byte = read_rom_byte(current_byte);

      if (current_byte == detect_1m)
      {
        if (read_rom_byte(0x100000 + current_byte) == read_byte)
        {
          detect_1m++;
        }
      }

      if (current_byte == detect_2m)
      {
        if (read_rom_byte(0x200000 + current_byte) == read_byte)
        {
          detect_2m++;
        }
      }
    }

    //ROM size detection
    if (detect_1m == DETECTION_SIZE)
    {
      romSize = 0x100000;
    }
    else if (detect_2m == DETECTION_SIZE)
    {
      romSize = 0x200000;
    }
    else
    {
      romSize = 0x400000;
    }

    return romSize;
  }

  void readSingleROM()
  {
    // Setup read mode
    read_setup();

    // Detect rom size
    romSize = detect_rom_size();

    OSCR::UI::print(F("READING "));
    OSCR::Lang::printBytes(romSize);
    OSCR::UI::printLine(F(" SINGLE-PACK"));

    // Create file
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::PocketChallengeW), FS(OSCR::Strings::Directory::ROM), fileName);

    // Init progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(romSize));

    for (uint32_t address = 0; address < romSize; address += 512) {  // 4MB
      for (uint16_t x = 0; x < 512; x++) {
        OSCR::Storage::Shared::buffer[x] = read_rom_byte(address + x);
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();

    // Compare CRC32 to database and rename ROM if found
    OSCR::Databases::Basic::matchCRC();
  }

  //******************************************
  //  MULTI-PACK FUNCTIONS
  //******************************************

  // Known Multi-Pack Carts (Yellow Label Carts)
  // 0BD400 [PS] (2MB Version)
  // 0BD400 [PS] (4MB Version)
  // 0BF400 [PL]
  // 1BF400 [PZ]
  // 8BD400 [CR]
  // 8BF400 [LP]
  // 9BF400 [SLP]

  // Per Overload, identify multi-pack cart by reading 0x3FFA-0x3FFE. Multi-Pack carts are non-zero.
  // 0x3FFA - Current Cartridge Bank
  // 0x3FFC - Value to Switch to Cartridge Bank 0
  // 0x3FFD - Value to Switch to Cartridge Bank 1
  // 0x3FFE - Last Value written to 0xFFFF

  // Bank Settings for 2MB
  // Write 0x28 to 0xFFFF to read 1st half of ROM
  // Write 0x2E to 0xFFFF to read 2nd half of ROM

  // Bank Settings for 4MB
  // Write 0x20 to 0xFFFF to read 1st half of ROM
  // Write 0x31 to 0xFFFF to read 2nd half of ROM

  void check_multi()
  {
    cartOn();

    // init variables
    read_setup();

    multipack = 0;
    bank0 = 0;
    bank1 = 0;

    uint8_t tempbyte = read_rom_byte(0x3FFC); // Check for a bank 0 switch value
    if (tempbyte)
    {
      bank0 = tempbyte; // Store bank 0 switch value
      tempbyte = read_rom_byte(0x3FFD); // Check for a bank 1 switch value

      if (tempbyte)
      {
        bank1 = tempbyte; // Store bank 1 switch value
        if (!read_rom_byte(0x3FFB) && !read_rom_byte(0x3FFF)) // Check for 00s
        {
          multipack = 1; // Flag as multi-pack

          OSCR::UI::clear();

          if ((bank0 == 0x28) && (bank1 == 0x2E)) // 2MB multi-pack cart
          {
            romSize = 0x200000;
          }
          else if ((bank0 == 0x20) && (bank1 == 0x31)) // 4MB multi-pack cart
          {
            romSize = 0x400000;
          }
          else // Warn for unknown bank switch values, size set to 4MB
          {
            OSCR::UI::printLine(F("Warning: Unknown cart size"));
            romSize = 0x400000;
          }
        }
      }
    }
  }

  void write_bank_byte(uint8_t data)
  {
    NAND_1A_LOW;
    NAND_1A_HIGH;
    NAND_1B_LOW;
    // Write to Address 0xFFFF
    PORTL = 0x00;
    PORTK = 0xFF;  // A8-A15
    // Latch Address on AD0-AD7
    ADDR_WRITE;
    LE_HIGH;       // Latch Enable
    PORTC = 0xFF;  // A0-A7
    LE_LOW;        // Address Latched
    // Write Data on AD0-AD7 - WE LOW ~728-736ns
    WE_LOW;
    PORTC = data;

    for (uint16_t x = 0; x < 40; x++)
      __asm__("nop\n\t");

    WE_HIGH;
    NAND_1B_HIGH;
  }

  void switchBank(int bank)
  {
    if (bank == 1) {  // Upper Half
      write_bank_byte(bank1);
    } else {  // Lower Half (default)
      write_bank_byte(bank0);
    }
  }

  void readMultiROM()
  {
    OSCR::UI::print(F("READING "));
    OSCR::Lang::printBytesLine(romSize);
    OSCR::UI::printLine(F(" MULTI-PACK"));

    // Create file
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::PocketChallengeW), FS(OSCR::Strings::Directory::ROM), fileName);

    // Init progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(romSize));

    // Lower Half
    read_setup();
    switchBank(0);
    for (uint32_t address = 0; address < (romSize / 2); address += 512)
    {
      for (uint16_t x = 0; x < 512; x++)
      {
        OSCR::Storage::Shared::buffer[x] = read_rom_byte(address + x);
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    // Upper Half
    read_setup();
    switchBank(1);

    for (uint32_t address = 0x200000; address < (0x200000 + (romSize / 2)); address += 512)
    {
      for (uint16_t x = 0; x < 512; x++)
      {
        OSCR::Storage::Shared::buffer[x] = read_rom_byte(address + x);
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();

    // Reset Bank
    switchBank(0);

    // Compare CRC32 to database and rename ROM if found
    OSCR::Databases::Basic::matchCRC();
  }

  //******************************************
  // SRAM FUNCTIONS
  //******************************************

  void readSRAM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::PocketChallengeW), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::SaveRAM));

    cartOn();

    read_setup();

    for (uint32_t address = 0x0; address < 0x8000; address += 512) // 32K
    {
      for (uint16_t x = 0; x < 512; x++)
      {
        OSCR::Storage::Shared::buffer[x] = read_ram_byte_1A(address + x);
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    cartOff();

    OSCR::Storage::Shared::close();
  }

  // SRAM
  void writeSRAM()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    read_setup();

    for (uint16_t address = 0x0; address < 0x8000; address += 512) // 32K
    {
      OSCR::Storage::Shared::fill();

      for (uint16_t x = 0; x < 512; x++)
      {
        write_ram_byte_1A(address + x, OSCR::Storage::Shared::buffer[x]);
      }
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    writeErrors = 0;

    OSCR::Storage::Shared::sharedFile.rewind();

    read_setup();

    for (uint16_t address = 0x0; address < 0x8000; address += 512) // 32K
    {
      for (uint16_t x = 0; x < 512; x++)
      {
        uint8_t myByte = read_ram_byte_1A(address + x);
        OSCR::Storage::Shared::buffer[x] = myByte;
      }

      for (int i = 0; i < 512; i++)
      {
        if (OSCR::Storage::Shared::sharedFile.read() != OSCR::Storage::Shared::buffer[i])
        {
          writeErrors++;
        }
      }
    }

    OSCR::Storage::Shared::close();

    if (writeErrors != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }
  }
} /* namespace OSCR::Cores::PCW */

#undef LE_HIGH
#undef LE_LOW
#undef NAND_1A_HIGH
#undef NAND_1A_LOW
#undef NAND_1B_HIGH
#undef NAND_1B_LOW
#undef WE_HIGH
#undef WE_LOW
#undef OE_HIGH
#undef OE_LOW
#undef MODE_READ
#undef MODE_WRITE
#undef DATA_REA
#undef ADDR_WRITE
#undef DETECTION_SIZE

#endif /* HAS_PCW */
