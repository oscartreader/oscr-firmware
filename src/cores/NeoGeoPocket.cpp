//******************************************
// NGP MODULE
//******************************************
#include "config.h"

#if HAS_NGP
# include "cores/include.h"
# include "cores/NeoGeoPocket.h"

namespace OSCR::Cores::NeoGeoPocket
{
  constexpr char const * const PROGMEM menuOptionsNGP[] = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::SetSize,
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr char const * const PROGMEM ngpRomOptions[] = {
    OSCR::Strings::Units::Size512KB,
    OSCR::Strings::Units::Size1MB,
    OSCR::Strings::Units::Size2MB,
    OSCR::Strings::Units::Size4MB,
  };

  char ngpRomVersion[3];

  uint8_t manufacturerID = 0;
  uint8_t deviceID = 0;
  uint16_t appID = 0;

  void menu()
  {
    //openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::NeoGeoPocket), menuOptionsNGP, sizeofarray(menuOptionsNGP)))
      {
      case 0:
        readROM();
        break;

      case 1:
        changeSize();
        break;

      case 2: // Back
        //closeCRDB();
        resetGlobals();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::NeoGeoPocket));
  }

  void cartOn()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    // A0 - A7
    DDRF = 0xFF;
    // A8 - A15
    DDRK = 0xFF;
    // A16 - A20
    DDRL = 0xFF;

    // D0 - D7
    DDRC = 0x00;

    // controls
    // /CE0: PH3
    // /CE1: PH0
    // /OE:  PH6
    // /WE:  PH5
    // PWR:  PH4
    DDRH |= ((1 << 0) | (1 << 3) | (1 << 5) | (1 << 6));
    DDRH &= ~(1 << 4);

    PORTH |= ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  // Switch data pins to read
  void dataIn()
  {
    // Set to Input and activate pull-up resistors
    DDRC = 0x00;
    // Pullups
    PORTC = 0xFF;
  }

  // Switch data pins to write
  void dataOut()
  {
    DDRC = 0xFF;
  }

  bool getCartInfo()
  {
    uint8_t * tmp;

    // enter autoselect mode
    writeByte(0x555, 0xAA);
    writeByte(0x2AA, 0x55);
    writeByte(0x555, 0x90);

    cartSize = 0;

    // get chip manufacturer and device IDs
    manufacturerID = readByte(0);
    deviceID = readByte(1);

    tmp = (uint8_t*)&romSize;

    *(tmp + 0) = deviceID;
    *(tmp + 1) = manufacturerID;

    switch (romSize)
    {
    // 4 Mbits
    case 0x98AB:  // Toshiba
    case 0x204C:  // STMicroelectronics ?
      cartSize = 524288;
      break;

    // 8 Mbits
    case 0x982C:  // Toshiba
    case 0xEC2C:  // Samsung
      cartSize = 1048576;
      break;

    // 16 Mbits
    case 0x982F:  // Toshiba
    case 0xEC2F:  // Samsung
    case 0x4C7:   // Fujitsu (FlashMasta USB)
      cartSize = 2097152;
      break;

    // detection error (no cart inserted or hw problem)
    case 0xFFFF:
      return false;
    }

    // reset to read mode
    writeByte(0x0, 0xF0);

    for (uint32_t addr = 0; addr < 28; addr++)
    {
      OSCR::Storage::Shared::buffer[addr] = readByte(addr);
    }

    if (memcmp_P(OSCR::Storage::Shared::buffer, PSTR("COPYRIGHT BY SNK CORPORATION"), 28) != 0 && memcmp_P(OSCR::Storage::Shared::buffer, PSTR(" LICENSED BY SNK CORPORATION"), 28) != 0)
      return false;

    appID = readByte(0x21) << 8 | readByte(0x20);

    // force rom size to 32 Mbits for few titles
    if (appID == 0x0060 || appID == 0x0061 || appID == 0x0069)
      cartSize = 4194304;

    // get app version
    snprintf(ngpRomVersion, 3, "%02X", readByte(0x22));

    // get app system compatibility
    romType = readByte(0x23);

    // get app name
    for (uint32_t i = 0; i < 17; i++)
    {
      fileName[i] = readByte(0x24 + i);

      // replace '/' chars in game name to avoid path errors
      if (fileName[i] == '/')
        fileName[i] = '_';
    }

    // Set rom size manually if chip ID is unknown
    if (cartSize == 0)
    {
      changeSize();
    }

    return true;
  }

  void printCartInfo()
  {
    printHeader();

    OSCR::UI::printValue(OSCR::Strings::Common::Name, fileName);

    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    OSCR::UI::printHexLine(appID);

    OSCR::UI::printValue(OSCR::Strings::Common::Revision, ngpRomVersion);

    char const * cartType = OSCR::Strings::Common::Unknown;

    if (romType == 0x0)
    {
      cartType = OSCR::Strings::Common::Mono;
    }
    else if (romType == 0x10)
    {
      cartType = OSCR::Strings::Common::Color;
    }

    OSCR::UI::printType_P(OSCR::Strings::Common::Cart, cartType);

    if (cartSize == 0)
    {
      OSCR::UI::printSize_P(OSCR::Strings::Common::ROM, OSCR::Strings::Common::Unknown);
    }
    else
    {
      OSCR::UI::printSize(OSCR::Strings::Common::ROM, (cartSize >> 17) * 1024 * 1024);
    }

    OSCR::UI::waitButton();
  }

  void readROM()
  {
    bool retry;

    do
    {
      retry = false;

      cartOn();

      if (getCartInfo())
      {
        printCartInfo();
      }
      else
      {
        cartOff();

        switch(OSCR::Prompts::abortRetryContinue())
        {
        case OSCR::Prompts::AbortRetryContinue::Abort:
          // return = goes back to main menu
          return;

        case OSCR::Prompts::AbortRetryContinue::Retry:
          retry = true;
          break;

        case OSCR::Prompts::AbortRetryContinue::Continue:
          //retry = false; // already false
          cartOn(); // Turn the cart back on
          break;
        }
      }
    }
    while (retry);

    // generate fullname of rom file
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::NeoGeoPocket), FS(OSCR::Strings::Directory::ROM), fileName);

    writeByte(0x0, 0xF0);

    OSCR::UI::ProgressBar::init(cartSize);

    for (uint32_t addr = 0; addr < cartSize; addr += 512)
    {
      // read block
      for (uint32_t i = 0; i < 512; i++)
      {
        OSCR::Storage::Shared::buffer[i] = readByte(addr + i);
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();

    cartOff();
  }

  void changeSize()
  {
    uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetROMSize), ngpRomOptions, sizeofarray(ngpRomOptions));

    // wait for user choice to come back from the question box menu
    switch (menuSelection)
    {
      case 0: cartSize = 524288; break;
      case 1: cartSize = 1048576; break;
      case 2: cartSize = 2097152; break;
      case 3: cartSize = 4194304; break;
    }
  }

  void writeByte(uint32_t addr, uint8_t data)
  {
    dataOut();

    PORTF = addr & 0xFF;
    PORTK = (addr >> 8) & 0xFF;
    PORTL = (addr >> 16) & 0x1F;
    PORTC = data;

    // which chip to select
    // 0x000000 - 0x1FFFFF -> /CE0
    // 0x200000 - 0x3FFFFF -> /CE1
    data = (addr & 0x00200000 ? (1 << 0) : (1 << 3));

    PORTH &= ~data;
    PORTH &= ~(1 << 5);
    NOP;

    PORTH |= data;
    PORTH |= (1 << 5);
    NOP;
    NOP;
  }

  uint8_t readByte(uint32_t addr)
  {
    uint8_t data;

    dataIn();

    PORTF = addr & 0xFF;
    PORTK = (addr >> 8) & 0xFF;
    PORTL = (addr >> 16) & 0x1F;

    // which chip to select
    // 0x000000 - 0x1FFFFF -> /CE0
    // 0x200000 - 0x3FFFFF -> /CE1
    data = (addr & 0x00200000 ? (1 << 0) : (1 << 3));

    PORTH &= ~data;
    PORTH &= ~(1 << 6);

    NOP;
    NOP;
    NOP;

    data = PINC;

    PORTH |= data;
    PORTH |= (1 << 6);

    return data;
  }
} /* namespace OSCR::Cores::NeoGeoPocket */

#endif /* HAS_NGP */
