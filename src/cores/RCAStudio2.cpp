//******************************************
// RCA STUDIO II MODULE
//******************************************
#include "config.h"

#if HAS_RCA
# include "cores/include.h"
# include "cores/RCAStudio2.h"

namespace OSCR::Cores::RCAStudio2
{
  // RCA Studio II
  // Cartridge Pinout
  // 22P 3.96mm pitch connector
  //
  //        FRONT
  //               +-------+
  //           D7 -| 1     |
  //           D6 -| 2     |
  //           D5 -| 3     |
  //           D4 -| 4     |
  //           D3 -| 5     |
  //  ROM_DISABLE -| 6     |
  //          GND -| 7     |
  //           D2 -| 8     |
  //           D1 -| 9     |
  //           D0 -| 10    |
  //           A0 -| 11    |
  //           A1 -| 12    |
  //           A2 -| 13    |
  //           A3 -| 14    |
  //     VCC(+5V) -| 15    |
  //           A4 -| 16    |
  //           A5 -| 17    |
  //           A6 -| 18    |
  //          TPA -| 19    |
  //           A7 -| 20    |
  //         /MRD -| 21    |
  //        ROMCS -| 22    |
  //               +-------+
  //
  //                                                BACK SIDE
  //       +-------------------------------------------------------------------------------------------+
  // LEFT  |                                                                                           | RIGHT
  //       |   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20   21  22  |
  //       +-------------------------------------------------------------------------------------------+
  //          D7  D6  D5  D4  D3 DIS GND  D2  D1  D0  A0  A1  A2  A3 +5V  A4  A5  A6 TPA  A7 /MRD  CS
  //
  //                                                FRONT SIDE

  // CONTROL PINS:
  // /MRD(PH3) - SNES /CS
  // TPA(PH6)  - SNES /RD

  using OSCR::Databases::Standard::crdbRecord;
  using OSCR::Databases::Standard::crdb;
  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  // VARIABLES
  //******************************************
  constexpr uint8_t const romSizes[] = {
    1,
    2,
  };

  constexpr uint8_t const romSizeLow = 0; // Lowest Entry
  constexpr uint8_t const romSizeHigh = sizeofarray(romSizes) - 1; // Highest Entry

  uint8_t romSize;

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
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::RCAStudio2), menuOptions, sizeofarray(menuOptions)))
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
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::RCAStudio2));
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
    // RCA Studio II uses A0-A7 [A8-A23 UNUSED]
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //       ---(PH0)   ---(PH1)  /MRD(PH3)   ---(PH4)   ---(PH5)   TPA(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Set Pins (D0-D7) to Input
    DDRC = 0x00;

    // Setting Control Pins to HIGH
    //       ---(PH0)   ---(PH1)  /MRD(PH3)   ---(PH4)   ---(PH5)   TPA(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTK = 0xFF; // A8-A15
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
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::RCAStudio2));
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
    // Setup TPA + /MRD
    PORTH &= ~(1 << 6); // TPA LOW
    delayMicroseconds(4);
    PORTH |= (1 << 3); // /MRD HIGH;
    delayMicroseconds(4);

    // Set HIGH Address
    PORTF = (addr >> 8) & 0xFF;
    delayMicroseconds(4);

    // Latch HIGH Address
    PORTH |= (1 << 6); // TPA HIGH
    delayMicroseconds(4);
    PORTH &= ~(1 << 3); // /MRD LOW
    delayMicroseconds(4);

    // Switch TPA LOW
    PORTH &= ~(1 << 6); // TPA LOW
    delayMicroseconds(4);

    // Set LOW Address
    PORTF = addr & 0xFF;
    delayMicroseconds(4);
    uint8_t ret = PINC;
    // Reset /MRD
    PORTH |= (1 << 3); // /MRD HIGH;

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

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::RCAStudio2), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::Raw));

    cartOn();

    readSegment(0x0400, 0x0600); // 512B

    if (romSize > 0)
    {
      readSegment(0x0600, 0x0800); // +512B = 1K
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
    fromCRDB = false;

    romSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeKB), romSizes, sizeofarray(romSizes));
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
} /* namespace OSCR::Cores::RCAStudio2 */

#endif /* HAS_RCA */
