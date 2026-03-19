//******************************************
// ATARI 2600 MODULE
//******************************************
#include "config.h"

#if HAS_2600
# include "cores/include.h"
# include "cores/Atari2600.h"

namespace OSCR::Cores::Atari2600
{
  // Atari 2600
  // Cartridge Pinout
  // 24P 2.54mm pitch connector
  //
  //                            LABEL SIDE
  //
  //         GND +5V  A8  A9  A11 A10 A12 D7  D6  D5  D4  D3
  //       +--------------------------------------------------+
  //       |  24  23  22  21  20  19  18  17  16  15  14  13  |
  // LEFT  |                                                  | RIGHT
  //       |   1   2   3   4   5   6   7   8   9  10  11  12  |
  //       +--------------------------------------------------+
  //          A7  A6  A5  A4  A3  A2  A1  A0  D0  D1  D2  GND
  //
  //                           BOTTOM SIDE

  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;
  using OSCR::Databases::Basic::mapperDetail;

  //******************************************
  //  Menu
  //******************************************
  // Core Main Menu
  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::SelectCart,
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::Headings::SelectMapper,
    OSCR::Strings::MenuOptions::Back,
  };

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Atari2600));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    // Atari 2600 uses A0-A12 [A13-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output [UNUSED]
    //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH [UNUSED]
    //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
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

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Atari2600), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        checkStatus();
        break;

      case 1: // Read ROM
        checkStatus();
        readROM();
        break;

      case 2: // Set Mapper
        setMapperMenu();
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

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Atari2600));
    OSCR::Databases::Basic::setupMapper(F("a2600-mappers"));
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
    PORTK = (addr >> 8) & 0xFF;  // A8-A12
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    DDRC = 0x00;   // Set to Input
    PORTC = 0xFF;  // Input Pullup
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
      readDataArray(addr, 512);
    }
  }

  void readDataArray(uint16_t addr, uint16_t size)
  {
    for (uint16_t w = 0; w < size; w++)
    {
      OSCR::Storage::Shared::buffer[w] = readData(addr + w);
    }

    OSCR::Storage::Shared::writeBuffer(size);
  }

  void readSegmentF8(uint16_t startaddr, uint16_t endaddr, uint16_t bankaddr)
  {
    for (uint16_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (uint16_t w = 0; w < 512; w++)
      {
        if (addr > 0x1FF9)  // SET BANK ADDRESS FOR 0x1FFA-0x1FFF
          readData(bankaddr);

        OSCR::Storage::Shared::buffer[w] = readData(addr + w);
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  void readSegmentE7(uint8_t start, uint8_t end)
  {
    for (uint8_t x = start; x <= end; x++)
    {
      readData(0x1FE0 + x);
      readSegment(0x1000, 0x1800);
    }
  }

  void readSegmentFx(bool hasRAM, uint16_t size)
  {
    if (hasRAM) {
      outputFF(0x100); // Skip 0x1000-0x10FF RAM
      readDataArray(0x1100, 0x100);
    } else {
      readSegment(0x1000, 0x1200);
    }
    readSegment(0x1200, 0x1E00);
    // Split Read of Last 0x200 bytes
    readDataArray(0x1E00, size);
  }

  void readSegmentTigervision(uint8_t banks)
  {
    for (uint8_t x = 0; x < banks; x++)
    {
      writeData3F(0x3F, x);
      readSegment(0x1000, 0x1800);
    }

    readSegment(0x1800, 0x2000);
  }

  void outputFF(uint16_t size)
  {
    memset(OSCR::Storage::Shared::buffer, 0xFF, size * sizeof(OSCR::Storage::Shared::buffer[0]));
    OSCR::Storage::Shared::writeBuffer(size);
  }

  void writeData(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A12
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

    DDRC = 0x00;  // Reset to Input
  }

  void writeData3F(uint16_t addr, uint8_t data)
  {
    PORTF = addr & 0xFF;         // A0-A7
    PORTK = (addr >> 8) & 0xFF;  // A8-A12
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

    // Address (0x1000);
    PORTF = 0x00;  // A0-A7
    PORTK = 0x10;  // A8-A12
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

  // E7 Mapper Check - Check Bank for FFs
  bool checkE7(uint16_t bank)
  {
    writeData(0x1800, 0xFF);
    readData(0x1FE0 + bank);
    uint32_t testdata = ((uint32_t)readData(0x1000) << 24) | ((uint32_t)readData(0x1001) << 16) | (readData(0x1002) << 8) | (readData(0x1003));
    return (testdata == 0xFFFFFFFF);
  }

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Atari2600), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    // ROM Start 0xF000
    // Address A12-A0 = 0x1000 = 1 0000 0000 0000 = 4KB
    // Read Start 0x1000

    switch (mapperDetail->id)
    {
    case 0x20:  // 2K Standard 2KB
      readSegment(0x1000, 0x1800);
      break;

    case 0x3F:  // 3F Mapper 8KB
      readSegmentTigervision(3);
      break;

    case 0x3E:  // 3E Mapper 32KB ROM 32K RAM
      readSegmentTigervision(15);
      break;

    case 0x40:  // 4K Default 4KB
      readSegment(0x1000, 0x2000);
      break;

    case 0xC0:  // CV Mapper 2KB
      readSegment(0x1800, 0x2000);
      break;

    case 0xD0:  // DPC Mapper 10KB
      // 8K ROM
      for (int x = 0; x < 0x2; x++)
      {
        readData(0x1FF8 + x);

        // Split Read of 1st 0x200 bytes
        // 0x0000-0x0080 are DPC Registers (Random on boot)
        outputFF(0x80); // Output 0xFFs for Registers
        readDataArray(0x1080, 0x180);

        // Read Segment
        readSegment(0x1200, 0x1800);

        // 0x1000-0x1080 are DPC Registers (Random on boot)
        outputFF(0x80); // Output 0xFFs for Registers
        readDataArray(0x1880, 0x180);

        // Read Segment
        readSegment(0x1A00, 0x1E00);

        // Split Read of Last 0x200 bytes
        readDataArray(0x1E00, 0x1F8);

        for (int z = 0; z < 8; z++)
        {
          // Set Bank to ensure 0x1FFA-0x1FFF is correct
          readData(0x1FF8 + x);
          OSCR::Storage::Shared::buffer[z] = readData(0x1FF8 + z);
        }

        OSCR::Storage::Shared::writeBuffer(8);
      }

      // 2K DPC Internal Graphics ROM
      // Read Registers 0x1008-0x100F (Graphics 0x1008-0x100C)
      // Write Registers LSB 0x1050-0x1057 AND MSB 0x1058-0x105F

      // Set Data Fetcher 0 Limits
      writeData(0x1040, 0xFF);  // MAX for Data Fetcher 0
      writeData(0x1048, 0x00);  // MIN for Data Fetcher 0

      // Set Data Fetcher 0 Counter (0x7FF)
      writeData(0x1050, 0xFF);  // LSB for Data Fetcher 0
      writeData(0x1058, 0x07);  // MSB for Data Fetcher 0

      // Set Data Fetcher 1 Counter (0x7FF)
      writeData(0x1051, 0xFF);  // LSB for Data Fetcher 1
      writeData(0x1059, 0x07);  // MSB for Data Fetcher 1

      for (int x = 0; x < 0x800; x += 512)
      {
        for (int y = 0; y < 512; y++)
        {
          OSCR::Storage::Shared::buffer[y] = readData(0x1008);  // Data Fetcher 0
          readData(0x1009);                // Data Fetcher 1
        }

        OSCR::Storage::Shared::writeBuffer();
      }

      break;

    case 0xE0: // E0 Mapper 8KB
      for (int x = 0; x < 0x7; x++)
      {
        readData(0x1FE0 + x);
        readSegment(0x1000, 0x1400);
      }

      readSegment(0x1C00, 0x2000);
      break;

    case 0xE7: // E7 Mapper 8KB/12KB/16KB
      writeData(0x1800, 0xFF);
      // Check Bank 0 - If 0xFFs then Bump 'n' Jump
      if (checkE7(0)) { // Bump 'n' Jump 8K
        readSegmentE7(4, 6); // Banks 4-6
      }
      // Check Bank 3 - If 0xFFs then BurgerTime
      else if (checkE7(3)) { // BurgerTime 12K
        readSegmentE7(0, 1); // Banks 0+1
        readSegmentE7(4, 6); // Banks 4-6
      }
      else { // Masters of the Universe (or Unknown Cart) 16K
        readSegmentE7(0, 6); // Banks 0-6
      }
      readSegment(0x1800, 0x2000); // Bank 7
      break;

    case 0xF0:  // F0 Mapper 64KB
      for (int x = 0; x < 0x10; x++) {
        readData(0x1FF0);
        readSegment(0x1000, 0x2000);
      }
      break;

    case 0x04: // F4SC Mapper 32KB w/RAM
    case 0xF4: // F4 Mapper 32KB
      for (int x = 0; x < 8; x++)
      {
        readData(0x1FF4 + x);
        readSegmentFx(mapperDetail->id == 0x04, 0x1F4);

        for (int z = 0; z < 12; z++)
        {
          // Set Bank to ensure 0x1FFC-0x1FFF is correct
          readData(0x1FF4 + x);
          OSCR::Storage::Shared::buffer[z] = readData(0x1FF4 + z);
        }

        OSCR::Storage::Shared::writeBuffer(12);
      }
      break;

    case 0x06: // F6SC Mapper 16KB w/RAM
    case 0xF6: // F6 Mapper 16KB
      for (int w = 0; w < 4; w++)
      {
        readData(0x1FF6 + w);
        readSegmentFx(mapperDetail->id == 0x06, 0x1F6);

        // Bank Registers 0x1FF6-0x1FF9
        for (int y = 0; y < 4; y++)
        {
          readData(0x1FFF); // Reset Bank
          OSCR::Storage::Shared::buffer[y] = readData(0x1FF6 + y);
        }

        // End of Bank 0x1FFA-0x1FFF
        readData(0x1FFF); // Reset Bank
        readData(0x1FF6 + w); // Set Bank

        for (int z = 4; z < 10; z++)
        {
          OSCR::Storage::Shared::buffer[z] = readData(0x1FF6 + z); // 0x1FFA-0x1FFF
        }

        OSCR::Storage::Shared::writeBuffer(10);
      }
      readData(0x1FFF); // Reset Bank
      break;

    case 0x08: // F8SC Mapper 8KB w/RAM
    case 0xF8: // F8 Mapper 8KB
      for (int w = 0; w < 2; w++)
      {
        readData(0x1FF8 + w);
        readSegmentFx(mapperDetail->id == 0x08, 0x1F8);

        // Bank Registers 0x1FF8-0x1FF9
        for (int y = 0; y < 2; y++)
        {
          readData(0x1FFF); // Reset Bank
          OSCR::Storage::Shared::buffer[y] = readData(0x1FF8 + y);
        }

        // End of Bank 0x1FFA-0x1FFF
        readData(0x1FFF); // Reset Bank
        readData(0x1FF8 + w); // Set Bank

        for (int z = 2; z < 8; z++)
        {
          OSCR::Storage::Shared::buffer[z] = readData(0x1FF8 + z); // 0x1FFA-0x1FFF
        }

        OSCR::Storage::Shared::writeBuffer(8);
      }

      readData(0x1FFF); // Reset Bank
      break;

    case 0xF9: // Time Pilot Mapper 8KB
      // Bad implementation of the F8 Mapper
      // kevtris swapped the bank order - swapped banks may not match physical ROM data
      // Bankswitch code uses 0x1FFC and 0x1FF9
      for (int w = 3; w >= 0; w -= 3)
      {
        readData(0x1FF9 + w);
        readSegment(0x1000, 0x1E00);

        // Split Read of Last 0x200 bytes
        readDataArray(0x1E00, 0x1F9);
        readData(0x1FFF); // Reset Bank

        OSCR::Storage::Shared::buffer[0] = readData(0x1FF9);

        // End of Bank 0x1FFA-0x1FFF
        readData(0x1FFF); // Reset Bank
        readData(0x1FF9 + w); // Set Bank

        for (int z = 1; z < 7; z++)
        {
          OSCR::Storage::Shared::buffer[z] = readData(0x1FF9 + z); // 0x1FFA-0x1FFF
        }

        OSCR::Storage::Shared::writeBuffer(7);
      }

      // Reset Bank
      readData(0x1FF9);
      readData(0x1FFF);
      readData(0x1FFC);

      break;

    case 0xFA:  // FA Mapper 12KB
      for (int x = 0; x < 0x3; x++)
      {
        writeData(0x1FF8 + x, 0x1);  // Set Bank with D0 HIGH
        readSegment(0x1000, 0x1E00);

        // Split Read of Last 0x200 bytes
        readDataArray(0x1E00, 0x1F8);

        for (int z = 0; z < 8; z++)
        {
          // Set Bank to ensure 0x1FFB-0x1FFF is correct
          writeData(0x1FF8 + x, 0x1);  // Set Bank with D0 HIGH

          OSCR::Storage::Shared::buffer[z] = readData(0x1FF8 + z);
        }

        OSCR::Storage::Shared::writeBuffer(8);
      }

      break;

    case 0xFE:  // FE Mapper 8KB
      for (int x = 0; x < 0x2; x++)
      {
        writeData(0x01FE, 0xF0 ^ (x << 5));
        writeData(0x01FF, 0xF0 ^ (x << 5));
        readSegment(0x1000, 0x2000);
      }
      break;

    case 0x0A:  // UA Mapper 8KB
      readData(0x220);
      readSegment(0x1000, 0x2000);
      readData(0x240);
      readSegment(0x1000, 0x2000);
      break;

    case 0x07:  // X07 Mapper 64K
      for (int x = 0; x < 16; x++)
      {
        readData(0x080D | (x << 4));
        readSegment(0x1000, 0x2000);
      }
      break;

    case 0xDF:  // DFSC 128K
      for (int x = 0; x < 0x20; x++)
      {
        readData(0x1FC0 + x);
        readSegment(0x1000, 0x1FBF);
      }
      break;

    case 0xBA:  // SUPERbank 128K
      for (int x = 0; x < 0x20; x++)
      {
        readData(0x0800 + x);
        readSegment(0x1000, 0x2000);
      }
      break;

    case 0xBB:  // SUPERbank 256K
      for (int x = 0; x < 0x40; x++)
      {
        readData(0x0800 + x);
        readSegment(0x1000, 0x2000);
      }
      break;

    default:
      break;
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();
  }

  //******************************************
  // ROM SIZE
  //******************************************

  void checkStatus()
  {
    printHeader();
    if (fromCRDB)
    {
      OSCR::UI::printValue(OSCR::Strings::Common::Name, romDetail->name);
    }

    OSCR::UI::printValue(OSCR::Strings::Common::Mapper, mapperDetail->name);

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, mapperDetail->sizeLow * 1024);
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

    fromCRDB = true;
  }
} /* namespace OSCR::Cores::Atari2600 */

#endif /* HAS_2600 */
