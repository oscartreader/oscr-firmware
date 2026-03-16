/******************************************
  SUPER FAMICOM SUFAMI TURBO MODULE
******************************************/
#include "config.h"

#if HAS_ST
# include "cores/include.h"
# include "cores/ST.h"

namespace OSCR::Cores::ST
{
  /******************************************
    Menu
  *****************************************/
  // Sufami Turbo menu items
  constexpr char const PROGMEM stMenuItem1[] = "Read Slot A";
  constexpr char const PROGMEM stMenuItem2[] = "Read Slot B";
  constexpr char const * const PROGMEM menuOptions[] = {
    stMenuItem1,
    stMenuItem2,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    if (!checkAdapter()) return;

    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::ST), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Read Slot A
        readSlot(0);
        break;

      case 1: // Read Slot B
        readSlot(1);
        break;

      case 2: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::ST));
  }

  /******************************************
     Setup
  *****************************************/
  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set cicrstPin(PG1) to Output
    DDRG |= (1 << 1);
    // Output a high signal until we're ready to start
    PORTG |= (1 << 1);
    // Set cichstPin(PG0) to Input
    DDRG &= ~(1 << 0);

    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //BA0-BA7
    DDRL = 0xFF;
    //PA0-PA7
    DDRA = 0xFF;

    // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
    DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
    // Switch RST(PH0) and WR(PH5) to HIGH
    PORTH |= (1 << 0) | (1 << 5);
    // Switch CS(PH3) and RD(PH6) to LOW
    PORTH &= ~((1 << 3) | (1 << 6));
    // Set Refresh(PE5) to Output
    DDRE |= (1 << 5);
    // Switch Refresh(PE5) to LOW (needed for SA-1)
    PORTE &= ~(1 << 5);
    // Set CPU Clock(PH1) to Output
    DDRH |= (1 << 1);
    // Set IRQ(PH4) to Input
    DDRH &= ~(1 << 4);
    // Set expand(PG5) to Input
    DDRG &= ~(1 << 5);
    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
    // Unused pins
    // Set wram(PE4) to Output
    DDRE |= (1 << 4);
    // Set pawr(PJ1) to Output
    DDRJ |= (1 << 1);
    // Set pard(PJ0) to Output
    DDRJ |= (1 << 0);
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::ST));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  // Check if Sufami Turbo adapter is inserted
  bool checkAdapter()
  {
    if (!getHeader(0))
    {
      printHeader();
      OSCR::UI::error(FS(OSCR::Strings::Errors::HeaderNotFound));
      return false;
    }

    return true;
  }

  /******************************************
    ROM Functions
  ******************************************/
  // Verify if 'BANDAI' header is present in specified bank
  bool getHeader(uint16_t bank)
  {
    uint8_t snesHeader[16];

    PORTL = bank;

    // Read first bytes
    for (uint16_t c = 0, currByte = 0; c < 16; c++, currByte++)
    {
      PORTF = (currByte & 0xFF);
      PORTK = ((currByte >> 8) & 0xFF);
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      snesHeader[c] = PINC;
    }

    // Format internal name
    setOutName((char *)snesHeader, 14);

    // Check if 'BANDAI' header is present
    return (strncmp_P(fileName, PSTR("BANDAI SFC-ADX"), 14) == 0);
  }

  // Select the slot to use and detect cart size
  void readSlot(bool cartSlot)
  {
    cartOn();

    // Set control
    OSCR::Cores::SNES::dataIn();
    OSCR::Cores::SNES::controlIn();

    uint16_t bankStart = cartSlot ? 64 : 32;
    uint16_t bankEnd = bankStart + (getHeader(bankStart + 16) ? 16 : 32);

    if (getHeader(bankStart))
    {
      readRom(bankStart, bankEnd);
      return;
    }

    cartOff();

    OSCR::UI::error(FS(OSCR::Strings::Errors::HeaderNotFound));
  }

  // Read ST rom to SD card
  void readRom(uint16_t bankStart, uint16_t bankEnd)
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::ST), FS(OSCR::Strings::Directory::ROM), fileName);

    OSCR::Cores::SNES::readLoRomBanks(bankStart + 0x80, bankEnd + 0x80, &OSCR::Storage::Shared::sharedFile);

    cartOff();

    // Close file:
    OSCR::Storage::Shared::close();

    // Compare dump CRC with db values
    OSCR::Databases::Basic::matchCRC();
  }
} /* namespace OSCR::Cores::ST */

#endif /* HAS_ST */
