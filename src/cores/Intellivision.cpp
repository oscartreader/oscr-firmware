//******************************************
// INTELLIVISION MODULE
//******************************************
#include "config.h"

#if HAS_INTV
# include "cores/include.h"

namespace OSCR::Cores::Intellivision
{
  // Mattel Intellivision
  // Cartridge Pinout
  // 44P 2.54mm pitch connector
  //
  //    TOP SIDE             BOTTOM SIDE
  //      (EVEN)             (ODD)
  //              +-------+
  //         GND -| 2   1 |- GND
  //       CBLNK -| 4   3 |- /MSYNC
  //     EXT AUD -| 6   5 |- DB7
  //     EXT VID -| 8   7 |- DB8
  //        MCLK -| 10  9 |- DB6
  //       RESET -| 12 11 |- DB9
  //         SR1 -| 14 13 |- DB5
  //       VOICE -| 16 15 |- DB10
  //       VOICE -| 18 17 |- DB4
  //         GND -| 20 19 |- DB11
  //         GND -| 22 21 |- DB3
  //         GND -| 24 23 |- DB12
  //         GND -| 26 25 |- DB13
  //         GND -| 28 27 |- DB2
  //      /BUSAK -| 30 29 |- DB14
  //         BC1 -| 32 31 |- DB1
  //         BC2 -| 34 33 |- DB0
  //        BDIR -| 36 35 |- DB15
  //        BDIR -| 38 37 |- BDIR
  //         BC2 -| 40 39 |- BC2
  //         BC1 -| 42 41 |- BC1
  //         GND -| 44 43 |- VCC(+5V)
  //              +-------+

  // CONTROL PINS:
  // /MSYNC(PH3)- PIN 3  [ROM PIN 14]
  // BC1(PH4)   - PIN 41 [ROM PIN 28]
  // BC2(PH5)   - PIN 39 [ROM PIN 27]
  // BDIR(PH6)  - PIN 37 [ROM PIN 26]
  // NOTE: BDIR/BC1/BC2 ONLY CONNECTED TO BOTTOM (ODD) PINS

  // NOT CONNECTED:
  // RESET(PH0) - N/C - GND IN CART
  // MCLK(PH1)  - N/C - GND IN CART

  // BUS PROTOCOL COMMANDS - BDIR/BC1/BC2
  // NO ACTION (NACT):           0/0/0
  // ADDR TO REG (ADAR):         0/1/0
  // INT ADDR TO BUS (IAB):      0/0/1
  // DATA TO BUS (DTB):          0/1/1
  // BUS TO ADDR (BAR):          1/0/0
  // DATA WRITE (DW):            1/1/0
  // DATA WRITE STROBE (DWS):    1/0/1
  // INTERRUPT ACK (INTAK):      1/1/1

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  using OSCR::Databases::Basic::mapperDetail;

  // Cart Configurations
  // Format = {mapper,romlo,romhi,ramsize}
  constexpr uint8_t const PROGMEM intvmapsize[] = {
    0, 0, 3, 0,  // default mattel up to 32K (8K/12K/16K/24K/32K)
    1, 2, 4, 0,  // demo cart 16K, championship tennis 32K, wsml baseball 48K
    2, 2, 4, 0,  // up to 48K (16K/32K/48K)
    3, 5, 5, 0,  // tower of doom 48K
    4, 2, 2, 1,  // uscf chess 16K + RAM 1K
    5, 3, 4, 0,  // congo bongo/defender/pac-man 24K, dig dug 32K
    6, 2, 2, 0,  // centipede 16K
    7, 2, 2, 0,  // imagic carts 16K
    8, 2, 2, 0,  // mte-201 test cart 16K
    9, 4, 4, 2,  // triple challenge 32K + RAM 2K
  };

  uint8_t intvmapcount = 10;  // (sizeof(mapsize)/sizeof(mapsize[0])) / 4;
  bool intvmapfound = false;
  uint8_t intvmapselect;
  int intvindex;

  constexpr uint8_t const romSizes[] = {
    8,
    12,
    16,
    24,
    32,
    48,
  };

  constexpr uint8_t const romSizeHigh = sizeofarray(romSizes) - 1;  // Highest Entry

  // EEPROM MAPPING
  // 07 MAPPER
  // 08 ROM SIZE

  //******************************************
  //  Menu
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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Intellivision), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        break;

      case 1: // Read ROM
        readROM();
        break;

      case 2: // Set Mapper
        setMapper();
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Intellivision));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output (UNUSED)
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //      ---(PH0)   ---(PH1)  /MSYNC(PH3) BC1(PH4)   BC2(PH5)  BDIR(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (DB0-DB15) to Input
    DDRC = 0x00;
    DDRA = 0x00;

    // Setting Control Pins to HIGH
    //       ---(PH0)   ---(PH1)  /MSYNC(PH3) BC1(PH4)   BC2(PH5)  BDIR(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Pins HIGH
    PORTF = 0xFF;       // A0-A7
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
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::Intellivision));
    OSCR::Databases::Basic::setupMapper(F("int-mappers"));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // INTELLIVISION BUS PROTOCOL COMMANDS
  //******************************************
  // CONTROL PINS -         BDIR/BC1/BC2
  // NO ACTION (NACT):         0/0/0
  // ADDR TO REG (ADAR):       0/1/0 [NOTE: ORDER BDIR/BC1/BC2]
  // DATA TO BUS (DTB):        0/1/1
  // BUS TO ADDR (BAR):        1/0/0
  // DATA WRITE (DW):          1/1/0 [NOTE: ORDER BDIR/BC1/BC2]
  // DATA WRITE STROBE (DWS):  1/0/1 [NOTE: ORDER BDIR/BC1/BC2]

  // IMMEDIATE MODE DATA READ SEQUENCE: BAR-NACT-DTB-NACT
  // IMMEDIATE MODE DATA WRITE SEQUENCE: BAR-NACT-DW-DWS-NACT
  // DIRECT ADDRESSING MODE READ SEQUENCE: BAR-NACT-ADAR-NACT-DTB-NACT

  // NO ACTION (NACT) - 0/0/0
  void NACT_INT()
  {
    // Switch BC1(PH4) + BC2(PH5) + BDIR(PH6) to LOW
    PORTH &= ~(1 << 4) & ~(1 << 5) & ~(1 << 6);

    // DB0..DB15 INPUT
    DDRC = 0x00;
    DDRA = 0x00;
  }

  // SET ADDRESS - BUS TO ADDR (BAR) - 1/0/0
  void BAR_INT()
  {
    // Switch BDIR(PH6) to HIGH
    PORTH |= (1 << 6);

    // Switch BC1(PH4) + BC2(PH5) to LOW
    PORTH &= ~(1 << 4) & ~(1 << 5);

    // Address OUT
    DDRC = 0xFF;
    DDRA = 0xFF;
  }

  // READ DATA - DATA TO BUS (DTB) - 0/1/1
  void DTB_INT()
  {
    // Switch BDIR(PH6) to LOW
    PORTH &= ~(1 << 6);

    // Switch BC1(PH4) + BC2(PH5) to HIGH
    PORTH |= (1 << 4) | (1 << 5);

    // Data IN
    DDRC = 0x00;
    DDRA = 0x00;
  }

  // ADDRESS DATA TO ADDRESS REGISTER (ADAR) - 0/1/0
  void ADAR_INT()
  {
    // Switch BC2(PH5) + BDIR(PH6) to LOW
    PORTH &= ~(1 << 5) & ~(1 << 6);

    // Switch BC1(PH4) to HIGH
    PORTH |= (1 << 4);
  }

  // DATA WRITE PAIRED COMMAND - DW + DWS
  // DATA SHOULD BE STABLE ACROSS BOTH

  // DATA WRITE (DW) - 1/1/0
  void DW_INT()
  {
    // Switch BC1(PH4) + BDIR(PH6) to HIGH
    PORTH |= (1 << 4) | (1 << 6);

    // Switch BC2(PH5) to LOW
    PORTH &= ~(1 << 5);
  }

  // DATA WRITE STROBE (DWS) - 1/0/1
  void DWS_INT()
  {
    // Switch BC2(PH5) + BDIR(PH6) to HIGH
    PORTH |= (1 << 5) | (1 << 6);

    // Switch BC1(PH4) to LOW
    PORTH &= ~(1 << 4);
  }

  //******************************************
  // READ CODE
  //******************************************

  uint16_t readData(uint32_t addr)
  {
    PORTC = addr & 0xFF;
    PORTA = (addr >> 8) & 0xFF;

    BAR_INT();

    // Wait for bus
    // 5 x 62.5ns = 312.5ns
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    NACT_INT();
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    DTB_INT();
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    uint16_t ret = (((PINA & 0xFF) << 8) | (PINC & 0xFF));

    NACT_INT();
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    return ret;
  }

  // MAPPER ROM ADDRESSES
  // 0: 0x50-0x70,0xD0-0xE0,0xF0-0x100,           // default mattel up to 32K (8K/16K/24K/32K)
  // 1: 0x50-0x70,0xD0-0x100,                     // demo cart 16K, championship tennis 32K, wsml baseball 48K
  // 2: 0x50-0x70,0x90-0xC0,0xD0-0xE0,            // up to 48K (16K/32K/48K)
  // 3: 0x50-0x70,0x90-0xB0,0xD0-0xE0,0xF0-0x100, // tower of doom 48K
  // 4: 0x50-0x70,                                // uscf chess 16K + RAM 1K
  // 5: 0x50-0x80,0x90-0xC0,                      // congo bongo/defender/pac-man 24K, dig dug 32K
  // 6: 0x60-0x80,                                // centipede 16K
  // 7: 0x48-0x68,                                // imagic carts 16K
  // 8: 0x50-0x60,0x70-0x80,                      // mte-201 test cart 16K
  // 9: 0x50-0x70,0x90-0xB0,[0xC0-0xC8,0xD0-0xD8] // triple challenge 32K + RAM 2K [0xC0 + 0xD0 segments are not needed]

  void readSegment(uint32_t startaddr, uint32_t endaddr)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += 256)
    {
      for (uint16_t w = 0; w < 256; w++)
      {
        uint16_t temp = readData(addr + w);
        uint16_t pos = w * 2;

        OSCR::Storage::Shared::buffer[pos] = (temp >> 8) & 0xFF;
        OSCR::Storage::Shared::buffer[++pos] = temp & 0xFF;
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  // MODIFIED READ ROUTINE FOR ALL 10 MAPPERS
  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Intellivision), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    switch (mapperDetail->id)
    {
    case 0: //default mattel up to 32K (8K/12K/16K/24K/32K)
      readSegment(0x5000, 0x6000);  // 8K

      if (romSize > 0)
      {
        readSegment(0x6000, 0x6800);  // +4K = 12K

        if (romSize > 1)
        {
          readSegment(0x6800, 0x7000);  // +4K = 16K

          if (romSize > 2)
          {
            readSegment(0xD000, 0xE000);  // +8K = 24K

            if (romSize > 3)
            {
              readSegment(0xF000, 0x10000);  // +8K = 32K
            }
          }
        }
      }
      break;

    case 1: // demo cart/championship tennis/wsml baseball
      readSegment(0x5000, 0x7000);  // 16K Demo Cart

      if (romSize > 2)
      {
        readSegment(0xD000, 0xE000);  // +8K = 24K [NONE]

        if (romSize > 3)
        {
          readSegment(0xE000, 0xF000);  // +8K = 32K Championship Tennis

          if (romSize > 4)
          {
            readSegment(0xF000, 0x10000);  // +8K = 40K WSML Baseball [MISSING 8K ECS BANK]

            // ecs bank switch
            ecsBank(0xFFFF, 0x1);               // switch ecs page 1 to 0xF000
            readSegment(0xF000, 0x10000);  // + 8K = 48K WSML Baseball
            ecsBank(0xFFFF, 0x0);               // reset ecs page 0 to 0xF000
          }
        }
      }
      break;

    case 2: // up to 48K (16K/32K/48K)
      readSegment(0x5000, 0x7000);  // 16K
      if (romSize > 2)
      {
        readSegment(0x9000, 0xA000);  // +8K = 24K [NONE]

        if (romSize > 3)
        {
          readSegment(0xA000, 0xB000);  // +8K = 32K

          if (romSize > 4)
          {
            readSegment(0xB000, 0xC000);  // +8K = 40K
            readSegment(0xD000, 0xE000);  // +8K = 48K
          }
        }
      }
      break;

    case 3: // tower of doom 48K
      readSegment(0x5000, 0x7000); // 16K
      readSegment(0x9000, 0xB000); // +16K = 32K
      readSegment(0xD000, 0xE000); // +8K = 40K
      readSegment(0xF000, 0x10000); // +8K = 48K
      break;

    case 4: // chess 16K
      PORTH &= ~(1 << 3); // /MSYNC to LOW
      readSegment(0x5000, 0x6000); // 8K

      PORTH |= (1 << 3); // /MSYNC to HIGH
      readSegment(0x6000, 0x7000); // 8K
      break;

    case 5: // congo bongo/defender/pac-man/dig dug
      readSegment(0x5000, 0x7000); // 16K
      readSegment(0x7000, 0x8000); // +8K = 24K Congo Bongo/Defender/Pac-Man

      if (romSize > 3)
      {
        readSegment(0x9000, 0xA000); // +8K = 32K Dig Dug
        //readSegment(0xA000,0xC000); // +16K = 48K [UNUSED]
      }
      break;

    case 6: // centipede 16K
      readSegment(0x6000, 0x8000); // 16K
      break;

    case 7: // imagic carts 16K
      readSegment(0x4800, 0x6800); // 16K
      break;

    case 8: //mte-201 test cart 16K
      readSegment(0x5000, 0x6000); // 8K
      readSegment(0x7000, 0x8000); // +8K = 16K
      break;

    case 9: // triple challenge 32K [KNOWN ROM 44K BAD!]
      readSegment(0x5000, 0x7000); // 16K
      readSegment(0x9000, 0xB000); // +16K = 32K

      // 0xC000 + 0xD000 SEGMENTS ARE NOT NEEDED (PER INTVNUT POST)
      //   readSegment(0xC000,0xC800); // +4K = 36K
      //   readSegment(0xD000,0xE000); // +8K = 44K
      break;
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();
  }

  // ECS BANKSWITCH FOR WSML BASEBALL
  // write $xA5y to $xFFF
  // x = rom location ($x000 - $xFFF)
  // y = page (up to 16 - WSML Baseball only uses 0/1)
  void ecsBank(uint32_t addr, uint8_t bank)
  {
    uint16_t ecsdata = (addr & 0xF000) + 0x0A50 + bank;  // $xA5y

    // Data OUT
    DDRA = 0xFF;
    DDRC = 0xFF;

    // Set Address
    PORTA = (addr >> 8) & 0xFF;
    PORTC = addr & 0xFF;

    BAR_INT();
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    NACT_INT();
    NOP;

    // Data OUT
    DDRA = 0xFF;
    DDRC = 0xFF;
    PORTA = (ecsdata >> 8) & 0xFF;  // $xA
    PORTC = ecsdata & 0xFF;         // $5y

    DW_INT();
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    DWS_INT();
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    NACT_INT();
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
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

    if (mapperDetail->sizeLow == mapperDetail->sizeHigh)
    {
      romSize = mapperDetail->sizeLow;
    }
    else
    {
      romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes, sizeofarray(romSizes));
    }

    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::MAPPER));
    OSCR::UI::printLine(mapperDetail->id);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(romSizes[romSize] * 1024);
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
} /* namespace OSCR::Cores::Intellivision */

#endif /* HAS_INTV */
