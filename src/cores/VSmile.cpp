//******************************************
// VSMILE MODULE
//******************************************
#include "config.h"

#if HAS_VSMILE
# include "cores/include.h"
# include "cores/VSmile.h"

namespace OSCR::Cores::VSmile
{
  // V.Smile
  // Cartridge pinout
  // 48P 2.54mm pitch connector
  //
  // FRONT               BACK
  //  SIDE               SIDE
  //        +---------+
  //   VCC -| 25    1 |- WE
  // RAMCS -| 26    2 |- OE
  //   VCC -| 27    3 |- GND
  //  CSB2 -| 28    4 |- D3
  //    D2 -| 29    5 |- D4
  //    D1 -| 30    6 |- D5
  //    D0 -| 31    7 |- D6
  //    D7 -| 32    8 |- D11
  //   D10 -| 33    9 |- D12
  //    D9 -| 34   10 |- D13
  //    D8 -| 35   11 |- D15
  //   D14 -| 36   12 |- A1
  //   A17 -| 37   13 |- A2
  //    A3 -| 38   14 |- A4
  //        |---------|
  //    A5 -| 39   15 |- A6
  //    A7 -| 40   16 |- A8
  //   A18 -| 41   17 |- A19
  //    A9 -| 42   18 |- A10
  //   A11 -| 43   19 |- A12
  //   A13 -| 44   20 |- A14
  //   A15 -| 45   21 |- A16
  //   A20 -| 46   22 |- A21
  //   A22 -| 47   23 |- GND
  //  CSB1 -| 48   24 |- VCC
  //        +---------+
  //
  // DATA D0-D7 - PORTC
  // DATA D8-D15 - PORTA
  // ADDR A1-A8 - PORTF
  // ADDR A9-A16 - PORTK
  // ADDR A17-A22 - PORTL
  // CONTROL PINS - PORTH

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const romSizes[] = {
    4,
    6,
    8,
    16,
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::VSmile), menuOptions, sizeofarray(menuOptions)))
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::VSmile));
  }

  //******************************************
  // SETUP
  //******************************************

  void cartOn()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    // Control Pins OE(PH3), CSB2(PH4), CSB1(PH6)
    DDRH = 0x58;  // 0b01011000 - CSB1, CSB2, OE [OUTPUT] - Unused Pins [INPUT]
    PORTH = 0xFF; // 0b11111111 - CSB1, CSB2, OE [HIGH] - Unused Pins [HIGH]

    // Address Pins
    DDRF = 0xFF;  // Address A1-A8 [OUTPUT]
    DDRK = 0xFF;  // Address A9-A16 [OUTPUT]
    DDRL = 0x3F;  // Address A17-A22 [OUTPUT] 0b00111111

    // Data Pins
    DDRC = 0x00;  // D0-D7 [INPUT]
    DDRA = 0x00;  // D8-D15 [INPUT]

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::VSmile));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // READ FUNCTIONS
  //******************************************
  // Max Single ROM Size 0x800000 (Highest WORD Address = 0x3FFFFF)
  uint16_t read_rom_word(uint32_t address)
  {
    uint16_t data;

    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    PORTF = address & 0xFF;
    _delay_us(1); // Need longer delay
    VSMILE_CSB2_HIGH;
    VSMILE_CSB1_LOW;
    VSMILE_OE_LOW;

    data = (PINC << 8) | (PINA);

    VSMILE_OE_HIGH;
    VSMILE_CSB1_HIGH;

    return data;
  }

  // VSMILE 2ND EPOXY CHIP [+2MB]
  // CSB2 LOW ONLY
  uint16_t read_rom2_word(uint32_t address)
  {
    uint16_t data;

    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    PORTF = address & 0xFF;
    _delay_us(1); // Need longer delay
    VSMILE_CSB1_HIGH;
    VSMILE_CSB2_LOW;
    VSMILE_OE_LOW;
    _delay_us(1); // Need longer delay

    data = (PINC << 8) | (PINA);

    VSMILE_OE_HIGH;
    VSMILE_CSB2_HIGH;

    return data;
  }

  // VSMILE MOTION 16MB 2ND CHIP [+8MB]
  // CSB1 + CSB2 LOW
  uint16_t read_rom3_word(uint32_t address)
  {
    uint16_t data;

    PORTL = (address >> 16) & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    PORTF = address & 0xFF;
    VSMILE_CSB1_LOW;
    VSMILE_CSB2_LOW;
    VSMILE_OE_LOW;

    data = (PINC << 8) | (PINA);

    VSMILE_OE_HIGH;
    VSMILE_CSB1_HIGH;
    VSMILE_CSB2_HIGH;

    return data;
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::VSmile), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::Raw));

    cartOn();

    for (uint32_t address = 0; address < 0x200000; address += 256) // 4MB
    {
      for (uint16_t x = 0; x < 256; x++)
      {
        uint16_t tempword = read_rom_word(address + x); // CSB1 LOW [CSB2 HIGH]

        OSCR::Storage::Shared::buffer[x * 2] = (tempword >> 0x8) & 0xFF;
        OSCR::Storage::Shared::buffer[(x * 2) + 1] = tempword & 0xFF;
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    if (romSize == 1) // 6MB - 2 EPOXY CHIPS [4MB + 2MB] Alphabet Park/Care Bears
    {
      for (uint32_t address = 0; address < 0x100000; address += 256) // +2MB HIGH = 6MB
      {
        for (uint16_t x = 0; x < 256; x++)
        {
          uint16_t tempword = read_rom2_word(address + x); // CSB2 LOW [CSB1 HIGH]

          OSCR::Storage::Shared::buffer[x * 2] = (tempword >> 0x8) & 0xFF;
          OSCR::Storage::Shared::buffer[(x * 2) + 1] = tempword & 0xFF;
        }

        OSCR::Storage::Shared::writeBuffer();
      }
    }
    else if (romSize > 1) // Normal 8MB
    {
      for (uint32_t address = 0x200000; address < 0x400000; address += 256) // +4MB = 8MB
      {
        for (uint16_t x = 0; x < 256; x++)
        {
          uint16_t tempword = read_rom_word(address + x); // CSB1 LOW [CSB2 HIGH]

          OSCR::Storage::Shared::buffer[x * 2] = (tempword >> 0x8) & 0xFF;
          OSCR::Storage::Shared::buffer[(x * 2) + 1] = tempword & 0xFF;
        }

        OSCR::Storage::Shared::writeBuffer();
      }

      if (romSize > 2) // Motion 16MB [8MB + 8MB] - Cars 2/Shrek Forever After/Super WHY!/Toy Story 3
      {
        for (uint32_t address = 0; address < 0x400000; address += 256) // +8MB HIGH = 16MB
        {
          for (uint16_t x = 0; x < 256; x++)
          {
            uint16_t tempword = read_rom3_word(address + x); // CSB1 + CSB2 LOW

            OSCR::Storage::Shared::buffer[x * 2] = (tempword >> 0x8) & 0xFF;
            OSCR::Storage::Shared::buffer[(x * 2) + 1] = tempword & 0xFF;
          }

          OSCR::Storage::Shared::writeBuffer();
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
    OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeMB), romSizes, sizeofarray(romSizes));

    printHeader();

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, romSizes[romSize] * 1024 * 1024);
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
} /* namespace OSCR::Cores::VSmile */

#endif /* HAS_VSMILE */
