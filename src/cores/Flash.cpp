//******************************************
// FLASHROM MODULE
// (also includes SNES repro functions)
//******************************************

#include "syslibinc.h"
#include "config.h"

#if defined(ENABLE_FLASH8) || defined(ENABLE_FLASH16) || defined(NEEDS_FLASH8) || defined(NEEDS_FLASH16)
# include "cores/include.h"
#endif

# include "cores/Flash.h"

namespace OSCR::Strings::Cores::Flash
{
  // Main Menu Header
  constexpr char const PROGMEM MenuHeader[]                   = "OSCR Flash Programmer";

  // Main Menu                                                                    // [   Requires   ]
  constexpr char const PROGMEM MenuOptionCFI[]                = "CFI";            //    8-bit Flash
  constexpr char const PROGMEM MenuOption8bit[]               = "8-bit Flash";    //    8-bit Flash
  constexpr char const PROGMEM MenuOption16bit[]              = "16-bit Flash";   //   16-bit Flash
  constexpr char const PROGMEM MenuOptionPLCCtoSNES[]         = "PLCC32 to SNES"; //    8-bit Flash
  constexpr char const PROGMEM MenuOptionEPROM[]              = "EPROM";          //   16-bit Flash

  // Submenu: Flash & EPROM
  constexpr char const PROGMEM MenuOptionSubmenuID[]          = "ID";
  constexpr char const PROGMEM MenuOptionSubmenuRead[]        = "Read";
  constexpr char const PROGMEM MenuOptionSubmenuWrite[]       = "Write";
  constexpr char const PROGMEM MenuOptionSubmenuErase[]       = "Erase";
  constexpr char const PROGMEM MenuOptionSubmenuPrint[]       = "Print";
  constexpr char const PROGMEM MenuOptionSubmenuVerify[]      = "Verify";
}

namespace OSCR::Cores::Flash
{
  /******************************************
     Variables
  *****************************************/
  uint16_t flashid;
  uint32_t flashSize;
  uint32_t blank;

#if defined(ENABLE_FLASH8) || defined(ENABLE_FLASH16) || defined(NEEDS_FLASH8) || defined(NEEDS_FLASH16)
  using OSCR::Databases::Basic::mapperDetail;
  using OSCR::Databases::Basic::mapperRecord;

  enum class MenuOptionMain : uint8_t
  {
    CFI,
    Flash8bit,
# if defined(ENABLE_FLASH16)
    Flash16bit,
# endif /* ENABLE_FLASH16 */
    PLCCtoSNES,
# if defined(ENABLE_FLASH16)
    EPROM,
# endif /* ENABLE_FLASH16 */
    Back,
  };

  constexpr char const * const PROGMEM menuOptionsMain[] = {
    OSCR::Strings::Cores::Flash::MenuOptionCFI,
    OSCR::Strings::Cores::Flash::MenuOption8bit,
# if defined(ENABLE_FLASH16)
    OSCR::Strings::Cores::Flash::MenuOption16bit,
# endif /* ENABLE_FLASH16 */
    OSCR::Strings::Cores::Flash::MenuOptionPLCCtoSNES,
# if defined(ENABLE_FLASH16)
    OSCR::Strings::Cores::Flash::MenuOptionEPROM,
# endif /* ENABLE_FLASH16 */
    OSCR::Strings::MenuOptions::Back,
  };

  enum class MenuOptionFlash : uint8_t
  {
    ID,
    Blankcheck,
    Read,
    Write,
    Erase,
    Print,
    Back,
  };

  constexpr char const * const PROGMEM menuOptionsFlash[] = {
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuID,
    OSCR::Strings::Headings::BlankCheck,
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuRead,
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuWrite,
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuErase,
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuPrint,
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr uint8_t const kMenuOptionFlashMax = sizeofarray(menuOptionsFlash);

# if defined(ENABLE_FLASH16)
  enum class MenuOptionEPROM : uint8_t
  {
    Blankcheck,
    Read,
    Write,
    Verify,
    Print,
    Back,
  };

  constexpr char const * const PROGMEM menuOptionsEPROM[] = {
    OSCR::Strings::Headings::BlankCheck,
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuRead,
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuWrite,
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuWrite,
    OSCR::Strings::Cores::Flash::MenuOptionSubmenuVerify,
    OSCR::Strings::MenuOptions::Back,
  };
# endif /* ENABLE_FLASH16 */

#endif /* ENABLE_FLASH8 || ENABLE_FLASH16 */

#if defined(ENABLE_FLASH8) || defined(NEEDS_FLASH8)
  uint32_t flashBanks;
  bool flashX16Mode;
  bool flashSwitchLastBits;

  // Flashrom
  uint8_t flashromType;
  uint32_t sectorSize;
  uint16_t bufferSize;
  uint8_t mapping = 0;
  bool byteCtrl = 0;

  /******************************************
     Menu
  *****************************************/

  // Misc flash strings
  constexpr char const PROGMEM ATTENTION_3_3V[] = "ATTENTION 3.3V";

  // Main Menu
  void menu()
  {
    do
    {
      switch (static_cast<MenuOptionMain>(OSCR::UI::menu(FS(OSCR::Strings::Cores::Flash::MenuHeader), menuOptionsMain, sizeofarray(menuOptionsMain))))
      {
      case MenuOptionMain::CFI:
        flashSize = 8388608;
        writeCFI(1, 1, 0);
        break;

      case MenuOptionMain::Flash8bit:
        setup8();
        id8();
        menuFlash8();
        break;

      case MenuOptionMain::PLCCtoSNES:
        mapping = 3;
        setup8();
        id8();
        menuFlash8();
        break;

# if defined(ENABLE_FLASH16)

      case MenuOptionMain::EPROM:
        setup_Eprom();
        menuEPROM();
        break;

      case MenuOptionMain::Flash16bit:
        setup16();
        id16();
        menuFlash16();
        break;

# endif /* ENABLE_FLASH16 */

      case MenuOptionMain::Back:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void menuFlash8()
  {
    do
    {
      MenuOptionFlash menuSelection = static_cast<MenuOptionFlash>(OSCR::UI::menu(FS(OSCR::Strings::Cores::Flash::MenuHeader), menuOptionsFlash, sizeofarray(menuOptionsFlash)));

      switch (menuSelection)
      {
      case MenuOptionFlash::Blankcheck:
        blankcheck();
        break;

      case MenuOptionFlash::Erase:
        if (flashromType == 0)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        if (!OSCR::Prompts::confirmErase()) continue;

        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        switch (flashromType)
        {
        case 1:
          eraseFlash29F032();
          break;

        case 2:
          eraseFlash29F1610();
          break;

        case 3:
          eraseFlash28FXXX();
          break;
        }

        resetFlash8();
        break;

      case MenuOptionFlash::Read:
        resetFlash8();
        readFlash();
        break;

      case MenuOptionFlash::Write:
        if (flashromType == 0)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

        switch (flashromType)
        {
        case 1:
          writeFlash29F032();
          break;

        case 2:
          if (flashid == 0xC2F3)
            writeFlash29F1601();
          else if ((flashid == 0xC2F1) || (flashid == 0xC2F9))
            writeFlash29F1610();
          else if ((flashid == 0xC2C4) || (flashid == 0xC249) || (flashid == 0xC2A7) || (flashid == 0xC2A8) || (flashid == 0xC2C9) || (flashid == 0xC2CB) || (flashid == 0x0149) || (flashid == 0x01C4) || (flashid == 0x01F9) || (flashid == 0x01F6) || (flashid == 0x01D7))
            writeFlash29LV640();
          else if (flashid == 0x017E)
            writeFlash29GL(sectorSize, bufferSize);
          else if ((flashid == 0x0458) || (flashid == 0x0158) || (flashid == 0x01AB) || (flashid == 0x0422) || (flashid == 0x0423))
            writeFlash29F800();
          else if (flashid == 0x0)  // Manual flash config, pick most common type
            writeFlash29LV640();
          break;

        case 3:
          writeFlash28FXXX();
          break;
        }

        delay(100);

        // Reset twice just to be sure
        resetFlash8();
        resetFlash8();

        verifyFlash();

        break;

      case MenuOptionFlash::ID:
        resetFlash8();

        printHeader();

        switch (flashromType)
        {
        case 1:
          idFlash29F032();
          break;

        case 2:
          idFlash29F1610();
          break;

        case 3:
          idFlash28FXXX();
          break;

        default:
          break;
        }

        OSCR::UI::printLine();

        printFlash(40);

        OSCR::UI::printLineSync();

        resetFlash8();

        break;

      case MenuOptionFlash::Print:
        printHeader();
        resetFlash8();
        printFlash(70);
        break;

      case MenuOptionFlash::Back:
        resetFlash8();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

# if defined(ENABLE_FLASH16)
  void menuFlash16()
  {
    do
    {
      MenuOptionFlash menuSelection = static_cast<MenuOptionFlash>(OSCR::UI::menu(FS(OSCR::Strings::Cores::Flash::MenuHeader), menuOptionsFlash, sizeofarray(menuOptionsFlash)));

      switch (menuSelection)
      {
      case MenuOptionFlash::Blankcheck:
        OSCR::UI::printLineSync(FS(OSCR::Strings::Headings::BlankCheck));

        resetFlash16();
        blankcheck16();
        break;

      case MenuOptionFlash::Erase:
        eraseFlash16();
        break;

      case MenuOptionFlash::Read:
        readFlash16();
        break;

      case MenuOptionFlash::Write:
        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

        if (flashid == 0xC2F3)
        {
          writeFlash16_29F1601();
        }
        else if ((flashid == 0xC2C4) || (flashid == 0xC249) || (flashid == 0xC2A7) || (flashid == 0xC2A8) || (flashid == 0xC2C9) || (flashid == 0xC2CB) || (flashid == 0x0149) || (flashid == 0x01C4) || (flashid == 0x01F9) || (flashid == 0x01F6) || (flashid == 0x01D7) || (flashid == 0xC2FC))
        {
          writeFlash16_29LV640();
        }
        else
        {
          writeFlash16();
        }

        delay(100);

        resetFlash16();

        delay(100);

        verifyFlash16();

        break;

      case MenuOptionFlash::ID:
        printHeader();

        idFlash16();
        printFlash16(40);

        OSCR::UI::printLine();

        resetFlash16();
        break;

      case MenuOptionFlash::Print:
        printHeader();

        resetFlash16();
        printFlash16(70);
        break;

      case MenuOptionFlash::Back:
        resetFlash16();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void menuEPROM()
  {
    do
    {
      MenuOptionEPROM menuSelection = static_cast<MenuOptionEPROM>(OSCR::UI::menu(FS(OSCR::Strings::Cores::Flash::MenuHeader), menuOptionsEPROM, sizeofarray(menuOptionsEPROM)));

      switch (menuSelection)
      {
      case MenuOptionEPROM::Blankcheck:
        OSCR::UI::printLineSync(FS(OSCR::Strings::Headings::BlankCheck));

        blankcheck_Eprom();

        break;

      case MenuOptionEPROM::Read:
        read_Eprom();
        break;

      case MenuOptionEPROM::Write:
        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

        write_Eprom();

        delay(1000);

        verify_Eprom();

        break;

      case MenuOptionEPROM::Verify:
        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

        verify_Eprom();

        break;

      case MenuOptionEPROM::Print:
        print_Eprom(80);
        break;

      case MenuOptionEPROM::Back:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }
# endif

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Flash::MenuHeader));
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
    dataDir = DataDirection::Unknown;
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setupMapper(FS(OSCR::Strings::FileType::Flash));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  void setupCFI()
  {
    setup8();
    identifyCFI();
  }

  /******************************************
     Flash ID
  *****************************************/\
  uint8_t selectFlashtype(bool option)
  {
    uint8_t selectionByte;

    if (option) selectionByte = OSCR::UI::rangeSelect(FS(OSCR::Strings::Headings::SelectMapper), 0, 3);
    else selectionByte = OSCR::UI::rangeSelect(FS(OSCR::Strings::Headings::SelectCartSize), 1, 8);

    if (option)
    {
      OSCR::UI::printType(OSCR::Strings::Common::Flash, selectionByte);
    }
    else
    {
      OSCR::UI::printSize(OSCR::Strings::Common::Flash, selectionByte * 1024 * 1024);
    }

    delay(200);

    return selectionByte;
  }

  bool getFlashDetail()
  {
    if (!OSCR::Databases::Basic::matchMapper(flashid)) return false;

    flashSize = mapperDetail->sizeLow;
    flashromType = mapperDetail->meta1;

    return true;
  }

  void id8()
  {
    uint16_t lastId = 0;

    printHeader();
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Searching));

    for (uint8_t retries = 0; retries < 4; retries++)
    {
      resetFlash8();

      switch (retries)
      {
      case 0: idFlash28FXXX(); break;
      case 1: idFlash29F032(); break;
      case 2: idFlash29F1610(); break;
      case 3: idFlash39SF040(); break;
      }

      OSCR::UI::setLineRel(-1);
      OSCR::UI::clearLine();

      if (lastId != flashid)
      {
        // ID: 12345... X
        OSCR::UI::printLabel(OSCR::Strings::Common::ID);
        OSCR::UI::printHex(lastId);
        OSCR::UI::print(FS(OSCR::Strings::Symbol::Ellipsis));
        OSCR::UI::printLine(FS(OSCR::Strings::Symbol::PaddedX));
      }

      OSCR::UI::printLabel(OSCR::Strings::Common::ID);
      OSCR::UI::printHex(flashid);
      OSCR::UI::printLineSync(FS(OSCR::Strings::Symbol::Ellipsis));

      if (getFlashDetail())
      {
        resetFlash8();

        printHeader();
        OSCR::UI::printValue(OSCR::Strings::Common::Name, mapperDetail->name);
        OSCR::UI::waitButton();

        return;
      }

      lastId = flashid;
    }

    // ID not found
    OSCR::UI::setLineRel(-1);
    OSCR::UI::clearLine();

    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    OSCR::UI::printHexLine(flashid);

    OSCR::UI::waitButton();

    // Select flashrom config manually
    flashSize = selectFlashtype(0) * 1024UL * 1024UL;
    flashromType = selectFlashtype(1);
    flashid = 0;

    // print first 40 bytes of flash
    printFlash(40);

    resetFlash8();

    OSCR::UI::waitButton();
  }

# if defined(ENABLE_FLASH16) || defined(NEEDS_FLASH16)
  void id16()
  {
    printHeader();
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Searching));

    idFlash16();
    resetFlash16();

    if (!getFlashDetail())
    {
      OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::CRDB));
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotFoundDB));
    }

    printHeader();
    OSCR::UI::printValue(OSCR::Strings::Common::Name, mapperDetail->name);
    OSCR::UI::waitButton();
  }
# endif

  /******************************************
     Setup
  *****************************************/
# if defined(ENABLE_VSELECT) || defined(ENABLE_3V3FIX)
  constexpr char const * const PROGMEM flashvoltOptions[] = {
    OSCR::Strings::Power::Voltage5,
    OSCR::Strings::Power::Voltage3V3,
    OSCR::Strings::MenuOptions::Back,
  };

  void setupVoltage()
  {
    switch (OSCR::UI::menu(FS(OSCR::Strings::Headings::ChangeVoltage), flashvoltOptions, sizeofarray(flashvoltOptions)))
    {
    case 0:
      // Request 3.3V
      OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
      break;

    case 1:
      // Request 5V
      OSCR::Power::setVoltage(OSCR::Voltage::k5V);
      break;

    case 2:
      return;
    }
  }
# else
  // The compiler will optimize this out when this condition is met.
  void setupVoltage() {}
# endif

  void setup8()
  {
    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output RST(PH0) OE(PH1) OE_SNS(PH3) WE(PH4) WE_SNS(PH5) CE(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    // Setting RST(PH0) OE(PH1) OE_SNS(PH3) WE(PH4) WE_SNS(PH5) HIGH
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);
    // Setting CE(PH6) LOW
    PORTH &= ~(1 << 6);

    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
    // Disable Internal Pullups
    PORTC = 0x00;
  }

# if defined(ENABLE_FLASH16) || defined(NEEDS_FLASH16)
  void setup16()
  {
    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output RST(PH0) OE(PH1) BYTE(PH3) WE(PH4) WP(PH5) CE(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set Data Pins (D0-D15) to Input
    DDRC = 0x00;
    DDRA = 0x00;
    // Disable Internal Pullups
    PORTC = 0x00;
    PORTA = 0x00;

    // Setting RST(PH0) OE(PH1) BYTE(PH3) WE(PH4) WP(PH5) HIGH
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);
    // Setting CE(PH6) LOW
    PORTH &= ~(1 << 6);

    delay(100);
  }

  void setup_Eprom()
  {
    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Data Pins (D0-D15) to Input
    DDRC = 0x00;
    DDRA = 0x00;
    // Disable Internal Pullups
    PORTC = 0x00;
    PORTA = 0x00;

    // Set Control Pins to Output VPP/OE(PH5) CE(PH6)
    DDRH |= (1 << 5) | (1 << 6);

    // Setting CE(PH6) HIGH
    PORTH |= (1 << 6);
    // Setting VPP/OE(PH5) LOW
    PORTH &= ~(1 << 5);

    // 27C322 is a 4MB eprom
    flashSize = 4194304;
  }
# endif

  /******************************************
     I/O Functions
  *****************************************/

  // Switch data pins to read
  void dataIn()
  {
    // Set to Input
    DDRA = 0x00;
    DDRC = 0x00;

    // Pullups
    PORTA = 0x00;
    PORTC = 0x00;
  }

  // Switch data pins to write
  void dataOut()
  {
    DDRA = 0xFF;
    DDRC = 0xFF;
  }

  /******************************************
     Low level functions
  *****************************************/
  void writeByte(uint32_t myAddress, uint8_t myData)
  {
    dataOut();

    // A0-A7
    PORTF = myAddress & 0xFF;

    // flash adapter (without SRAM save chip)
    if (mapping == 0)
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A23
      PORTL = (myAddress >> 16) & 0xFF;
    }
    else if (mapping == 3)
    {
      /* SNES maskrom flash adapter combined with PLCC32 adapter
        SNES A19 -> 32PLCC A17
        SNES A18 -> 32PLCC A16
        SNES A17 -> 32PLCC A18
        SNES A16 -> 32PLCC OE
        SNES CS/Flash OE -> 32PLCC WE
      */

      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16
      if (!(((myAddress >> 16) & 0xFF) & 0x1))
      {
        PORTL &= ~(1 << 2);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x1)
      {
        PORTL |= (1 << 2);
      }

      // A17
      if (!(((myAddress >> 16) & 0xFF) & 0x2))
      {
        PORTL &= ~(1 << 3);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x2)
      {
        PORTL |= (1 << 3);
      }

      // A18
      if (!(((myAddress >> 16) & 0xFF) & 0x4))
      {
        PORTL &= ~(1 << 1);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x4)
      {
        PORTL |= (1 << 1);
      }

      // Switch PLCC32 OE to HIGH to disable output
      PORTL |= (1 << 0);
    }
    else if (mapping == 1) // SNES LoRom
    {
      // A8-A14
      PORTK = (myAddress >> 8) & 0x7F;

      // Set SNES A15(PK7) HIGH to disable SRAM
      PORTK |= (1 << 7);

      // A15-A22
      PORTL = (myAddress >> 15) & 0xFF;
    }
    else if (mapping == 2) // SNES HiRom
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A23
      PORTL = (myAddress >> 16) & 0xFF;

      // Set PL7 to value of PL6
      if (!(((myAddress >> 16) & 0xFF) & 0x40))
      {
        // if PL6 is 0 set PL7 to 0
        PORTL &= ~(1 << 7);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x40)
      {
        // if PL6 is 1 set PL7 to 1
        PORTL |= (1 << 7);
      }

      // Switch SNES BA6(PL6) to HIGH to disable SRAM
      PORTL |= (1 << 6);
    }
    else if (mapping == 122) // for SNES LoRom repro with 2x 2MB
    {
      // A8-A14
      PORTK = (myAddress >> 8) & 0x7F;

      // Set SNES A15(PK7) HIGH to disable SRAM
      PORTK |= (1 << 7);

      // A15-A22
      PORTL = (myAddress >> 15) & 0xFF;

      // Flip BA6(PL6) to address second rom chip
      PORTL ^= (1 << 6);
    }
    else if (mapping == 222) // for SNES HiRom repro with 2x 2MB
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A23
      PORTL = (myAddress >> 16) & 0xFF;

      // Flip BA5(PL5) to address second rom chip
      PORTL ^= (1 << 5);

      // Switch SNES BA6(PL6) to HIGH to disable SRAM
      PORTL |= (1 << 6);
    }
    else if (mapping == 124) // for SNES ExLoRom repro with 2x 4MB
    {
      // A8-A14
      PORTK = (myAddress >> 8) & 0x7F;

      // Set SNES A15(PK7) HIGH to disable SRAM
      PORTK |= (1 << 7);

      // A15-A22
      PORTL = (myAddress >> 15) & 0xFF;

      // Flip A22(PL7) to reverse P0 and P1 roms
      PORTL ^= (1 << 7);
    }
    // for SNES ExHiRom repro with 2x 4MB
    else if (mapping == 224)
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A22
      PORTL = (myAddress >> 16) & 0xFF;

      // Set PL7 to inverse of PL6 to reverse P0 and P1 roms
      if (!(((myAddress >> 16) & 0xFF) & 0x40))
      {
        // if PL6 is 0 set PL7 to 1
        PORTL |= (1 << 7);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x40)
      {
        // if PL6 is 1 set PL7 to 0
        PORTL &= ~(1 << 7);
      }

      // Switch SNES BA6(PL6) to HIGH to disable SRAM
      PORTL |= (1 << 6);
    }
    else if (mapping == 142) // for SNES ExLoRom repro with 4x 2MB
    {
      // A8-A14
      PORTK = (myAddress >> 8) & 0x7F;

      // Set SNES A15(PK7) HIGH to disable SRAM
      PORTK |= (1 << 7);

      // A15-A22
      PORTL = (myAddress >> 15) & 0xFF;

      // Flip BA6(PL6) to address second rom chip
      PORTL ^= (1 << 6);

      // Flip A22(PL7) to reverse P0 and P1 roms
      PORTL ^= (1 << 7);
    }
    else if (mapping == 242) // for SNES ExHiRom repro with 4x 2MB
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A22
      PORTL = (myAddress >> 16) & 0xFF;

      // Flip BA5(PL5) to address second rom chip
      PORTL ^= (1 << 5);

      // Set PL7 to inverse of PL6 to reverse P0 and P1 roms
      if (!(((myAddress >> 16) & 0xFF) & 0x40))
      {
        // if PL6 is 0 set PL7 to 1
        PORTL |= (1 << 7);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x40)
      {
        // if PL6 is 1 set PL7 to 0
        PORTL &= ~(1 << 7);
      }

      // Switch SNES BA6(PL6) to HIGH to disable SRAM
      PORTL |= (1 << 6);
    }

    // Data
    PORTC = myData;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    if (mapping == 3) // Switch PLCC32 WE to LOW
    {
      PORTH &= ~(1 << 3);
    }
    else // Switch WE(PH4) WE_SNS(PH5) to LOW
    {
      PORTH &= ~((1 << 4) | (1 << 5));
    }

    // Leave WE low for at least 60ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    if (mapping == 3) // Switch PLCC32 WE to HIGH
    {
      PORTH |= (1 << 3);
    }
    else // Switch WE(PH4) WE_SNS(PH5) to HIGH
    {
      PORTH |= (1 << 4) | (1 << 5);
    }

    // Leave WE high for at least 50ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );
  }

  uint8_t readByte(uint32_t myAddress)
  {
    dataIn();

    // A0-A7
    PORTF = myAddress & 0xFF;

    // flash adapter (without SRAM save chip)
    if (mapping == 0)
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A23
      PORTL = (myAddress >> 16) & 0xFF;
    }
    else if (mapping == 3) // SNES maskrom flash adapter combined with PLCC32 adapter
    {
      /*
        SNES A19 -> 32PLCC A17
        SNES A18 -> 32PLCC A16
        SNES A17 -> 32PLCC A18
        SNES A16 -> 32PLCC OE
        SNES CS/Flash OE -> 32PLCC WE
      */

      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16
      if (!(((myAddress >> 16) & 0xFF) & 0x1))
      {
        PORTL &= ~(1 << 2);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x1)
      {
        PORTL |= (1 << 2);
      }

      // A17
      if (!(((myAddress >> 16) & 0xFF) & 0x2))
      {
        PORTL &= ~(1 << 3);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x2)
      {
        PORTL |= (1 << 3);
      }

      // A18
      if (!(((myAddress >> 16) & 0xFF) & 0x4))
      {
        PORTL &= ~(1 << 1);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x4)
      {
        PORTL |= (1 << 1);
      }

      // Switch PLCC32 WE to HIGH to disable writing
      PORTH |= (1 << 3);
    }
    else if (mapping == 1) // SNES LoRom
    {
      // A8-A14
      PORTK = (myAddress >> 8) & 0x7F;
      // Set SNES A15(PK7) HIGH to disable SRAM
      PORTK |= (1 << 7);
      // A15-A22
      PORTL = (myAddress >> 15) & 0xFF;
    }
    else if (mapping == 2) // SNES HiRom
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A23
      PORTL = (myAddress >> 16) & 0xFF;

      // Set PL7 to value of PL6
      if (!(((myAddress >> 16) & 0xFF) & 0x40)) // if PL6 is 0 set PL7 to 0
      {
        PORTL &= ~(1 << 7);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x40) // if PL6 is 1 set PL7 to 1
      {
        PORTL |= (1 << 7);
      }

      // Switch SNES BA6(PL6) to HIGH to disable SRAM
      PORTL |= (1 << 6);
    }
    else if (mapping == 122) // for SNES LoRom repro with 2x 2MB
    {
      // A8-A14
      PORTK = (myAddress >> 8) & 0x7F;

      // Set SNES A15(PK7) HIGH to disable SRAM
      PORTK |= (1 << 7);

      // A15-A22
      PORTL = (myAddress >> 15) & 0xFF;

      // Flip BA6(PL6) to address second rom chip
      PORTL ^= (1 << 6);
    }
    else if (mapping == 222) // for SNES HiRom repro with 2x 2MB
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A23
      PORTL = (myAddress >> 16) & 0xFF;

      // Flip BA5(PL5) to address second rom chip
      PORTL ^= (1 << 5);

      // Switch SNES BA6(PL6) to HIGH to disable SRAM
      PORTL |= (1 << 6);
    }
    else if (mapping == 124) // for SNES ExLoRom repro with 2x 4MB
    {
      // A8-A14
      PORTK = (myAddress >> 8) & 0x7F;

      // Set SNES A15(PK7) HIGH to disable SRAM
      PORTK |= (1 << 7);

      // A15-A22
      PORTL = (myAddress >> 15) & 0xFF;

      // Flip A22(PL7) to reverse P0 and P1 roms
      PORTL ^= (1 << 7);
    }
    else if (mapping == 224) // for SNES ExHiRom repro with 2x 4MB
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A22
      PORTL = (myAddress >> 16) & 0xFF;

      // Set PL7 to inverse of PL6 to reverse P0 and P1 roms
      if (!(((myAddress >> 16) & 0xFF) & 0x40)) // if PL6 is 0 set PL7 to 1
      {
        PORTL |= (1 << 7);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x40) // if PL6 is 1 set PL7 to 0
      {
        PORTL &= ~(1 << 7);
      }

      // Switch SNES BA6(PL6) to HIGH to disable SRAM
      PORTL |= (1 << 6);
    }
    else if (mapping == 142) // for SNES ExLoRom repro with 4x 2MB
    {
      // A8-A14
      PORTK = (myAddress >> 8) & 0x7F;

      // Set SNES A15(PK7) HIGH to disable SRAM
      PORTK |= (1 << 7);

      // A15-A22
      PORTL = (myAddress >> 15) & 0xFF;

      // Flip BA6(PL6) to address second rom chip
      PORTL ^= (1 << 6);

      // Flip A22(PL7) to reverse P0 and P1 roms
      PORTL ^= (1 << 7);
    }
    else if (mapping == 242) // for SNES ExHiRom repro with 4x 2MB
    {
      // A8-A15
      PORTK = (myAddress >> 8) & 0xFF;

      // A16-A22
      PORTL = (myAddress >> 16) & 0xFF;

      // Flip BA5(PL5) to address second rom chip
      PORTL ^= (1 << 5);

      // Set PL7 to inverse of PL6 to reverse P0 and P1 roms
      if (!(((myAddress >> 16) & 0xFF) & 0x40)) // if PL6 is 0 set PL7 to 1
      {
        PORTL |= (1 << 7);
      }
      else if (((myAddress >> 16) & 0xFF) & 0x40) // if PL6 is 1 set PL7 to 0
      {
        PORTL &= ~(1 << 7);
      }

      // Switch SNES BA6(PL6) to HIGH to disable SRAM
      PORTL |= (1 << 6);
    }

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    if (byteCtrl)
    {
      // Setting OE(PH1) LOW
      PORTH &= ~(1 << 1);
    }
    else if (mapping == 3)
    {
      // Switch PLCC32 OE to LOW
      PORTL &= ~(1 << 0);
    }
    else
    {
      // Setting OE(PH1) OE_SNS(PH3) LOW
      PORTH &= ~((1 << 1) | (1 << 3));
    }

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read
    uint8_t tempByte = PINC;

    if (byteCtrl) // Setting OE(PH1) HIGH
    {
      PORTH |= (1 << 1);
    }
    else if (mapping == 3) // Switch PLCC32 OE to HIGH
    {
      PORTL |= (1 << 0);
    }
    else // Setting OE(PH1) OE_SNS(PH3) HIGH
    {
      PORTH |= (1 << 1) | (1 << 3);
    }

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    return tempByte;
  }

  void writeWord(uint32_t myAddress, uint16_t myData)
  {
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
    PORTC = myData;
    PORTA = (myData >> 8) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    AVR_ASM("nop\n\t");

    // Switch WE(PH4) to LOW
    PORTH &= ~(1 << 4);

    // Leave WE low for at least 60ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Switch WE(PH4) to HIGH
    PORTH |= (1 << 4);

    // Leave WE high for at least 50ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );
  }

  uint16_t readWord(uint32_t myAddress)
  {
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(AVR_INS("nop"));

    // Setting OE(PH1) LOW
    PORTH &= ~(1 << 1);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read
    uint16_t tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

    AVR_ASM(AVR_INS("nop"));

    // Setting OE(PH1) HIGH
    PORTH |= (1 << 1);
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    return tempWord;
  }

  /******************************************
    Command functions
  *****************************************/
  void writeByteCommand(uint8_t command)
  {
    if ((flashid == 0xBFB7) || (flashid == 0xBFB6) || (flashid == 0xBFB5))
    {
      //39F040
      writeByte(0x5555, 0xAA);
      writeByte(0x2AAA, 0x55);
      writeByte(0x5555, command);
    }
    else
    {
      writeByte(0x555, 0xAA);
      writeByte(0x2AA, 0x55);
      writeByte(0x555, command);
    }
  }

  void writeByteCommandShift(uint8_t command)
  {
    writeByte(0x5555 << 1, 0xAA);
    writeByte(0x2AAA << 1, 0x55);
    writeByte(0x5555 << 1, command);
  }

  void writeWordCommand(uint8_t command)
  {
    writeWord(0x5555, 0xAA);
    writeWord(0x2AAA, 0x55);
    writeWord(0x5555, command);
  }

  /******************************************
    29F032/39SF040 flashrom functions
  *****************************************/
  void resetFlash29F032()
  {
    writeByte(0x555, 0xF0); // Reset command sequence
    delay(500);
  }

  void idFlash39SF040()
  {
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0x90);

    // Read the two id bytes into a string
    flashid = readByte(0) << 8;
    flashid |= readByte(1);
  }

  void idFlash29F032()
  {
    // ID command sequence
    writeByteCommand(0x90);

    // Read the two id bytes into a string
    flashid = readByte(0) << 8;
    flashid |= readByte(1);
  }

  void busyWait(uint32_t address, uint8_t data)
  {
    while ((readByte(address) & 0x80) != (data & 0x80))
    {
      delay(100);
    }
  }

  void busyWait(uint32_t address)
  {
    busyWait(address, 0x80);
  }

  void busyWait()
  {
    busyWait(0, 0x80);
  }

  void eraseFlash29F032()
  {
    // Erase command sequence
    writeByteCommand(0x80);
    writeByteCommand(0x10);

    // After a completed erase, D7 will output 1
    busyWait();
  }

  void writeFlash29F032()
  {
    // Retry writing, for when /RESET is not connected (floating)
    int dq5failcnt = 0;
    int noread = 0;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    // Fill buffer
    for (uint32_t currByte = 0; currByte < fileSize; currByte += 512)
    {
      // if (currByte >= 0) {
      //   OSCR::UI::print(currByte);
      //   OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));
      //   OSCR::UI::print(dq5failcnt);
      //   OSCR::UI::printLine();
      // }
      if (!noread)
      {
        OSCR::Storage::Shared::fill();
      }

      noInterrupts();

      int blockfailcnt = 0;

      for (int c = 0; c < 512; c++)
      {
        uint8_t datum = OSCR::Storage::Shared::buffer[c];
        uint8_t d = readByte(currByte + c);

        if (d == datum || datum == 0xFF)
        {
          continue;
        }

        // Write command sequence
        writeByteCommand(0xA0);

        // Write current byte
        writeByte(currByte + c, datum);

        if (busyCheck29F032(currByte + c, datum))
        {
          dq5failcnt++;
          blockfailcnt++;
        }
      }

      interrupts();

      if (blockfailcnt > 0)
      {
        OSCR::UI::print(F("Failures at "));
        OSCR::UI::print(currByte);
        OSCR::UI::print(FS(OSCR::Strings::Symbol::LabelEnd));
        OSCR::UI::print(blockfailcnt);
        OSCR::UI::printLine();
        dq5failcnt -= blockfailcnt;
        currByte -= 512;
        delay(100);
        noread = 1;
      }
      else
      {
        noread = 0;
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();
  }

  bool busyCheck29F032(uint32_t addr, uint8_t c)
  {
    bool ret = false;

    if (byteCtrl)
    {
      // Setting OE(PH1) CE(PH6)LOW
      PORTH &= ~((1 << 1) | (1 << 6));
    }
    else
    {
      // Setting OE(PH1) OE_SNS(PH3) CE(PH6)LOW
      PORTH &= ~((1 << 1) | (1 << 3) | (1 << 6));
    }

    // Setting WE(PH4) WE_SNES(PH5) HIGH
    PORTH |= (1 << 4) | (1 << 5);

    //When the Embedded Program algorithm is complete, the device outputs the datum programmed to D7
    for (;;)
    {
      uint8_t d = readByte(addr);

      if ((d & 0x80) == (c & 0x80))
      {
        break;
      }

      if ((d & 0x20) == 0x20)
      {
        // From the datasheet:
        // DQ 5 will indicate if the program or erase time has exceeded the specified limits (internal pulse count).
        // Under these conditions DQ 5 will produce a “1”.
        // This is a failure condition which indicates that the program or erase cycle was not successfully completed.
        // Note : DQ 7 is rechecked even if DQ 5 = “1” because DQ 7 may change simultaneously with DQ 5 .
        d = readByte(addr);

        if ((d & 0x80) == (c & 0x80))
        {
          break;
        }
        else
        {
          ret = true;
          break;
        }
      }
    }

    if (byteCtrl)
    {
      // Setting OE(PH1) HIGH
      PORTH |= (1 << 1);
    }
    else
    {
      // Setting OE(PH1) OE_SNS(PH3) HIGH
      PORTH |= (1 << 1) | (1 << 3);
    }

    return ret;
  }

  /******************************************
    29F1610 flashrom functions
   *****************************************/

  void resetFlash29F1610()
  {
    // Reset command sequence
    writeByteCommandShift(0xF0);

    delay(500);
  }

  void writeFlash29F1610()
  {
    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currByte = 0; currByte < fileSize; currByte += 128)
    {
      // Fill SD buffer with 1 page at a time then write it repeat until all bytes are written
      OSCR::Storage::Shared::readBuffer(128);

      // Check if write is complete
      delayMicroseconds(100);
      busyWait();

      // Write command sequence
      writeByteCommandShift(0xA0);

      // Write one full page at a time
      for (uint8_t c = 0; c < 128; c++)
      {
        writeByte(currByte + c, OSCR::Storage::Shared::buffer[c]);
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(128);
    }

    // Check if write is complete
    busyWait();

    OSCR::UI::ProgressBar::finish();
  }

  void writeFlash29F1601()
  {
    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currByte = 0; currByte < fileSize; currByte += 128)
    {
      // Fill SD buffer with 1 page at a time then write it repeat until all bytes are written
      OSCR::Storage::Shared::readBuffer(128);

      // Check if write is complete
      delayMicroseconds(100);
      busyWait();

      // Write command sequence
      writeByteCommandShift(0xA0);

      // Write one full page at a time
      for (uint8_t c = 0; c < 128; c++)
      {
        writeByte(currByte + c, OSCR::Storage::Shared::buffer[c]);
      }

      // Write the last byte again or it won't write at all
      writeByte(currByte + 127, OSCR::Storage::Shared::buffer[127]);

      // update progress bar
      OSCR::UI::ProgressBar::advance(128);
    }

    // Check if write is complete
    busyWait();

    OSCR::UI::ProgressBar::finish();
  }

  void idFlash29F1610()
  {
    // ID command sequence
    writeByteCommandShift(0x90);

    // Read the two id bytes into a string
    flashid = readByte(0) << 8;
    flashid |= readByte(2);
  }

  uint8_t readStatusReg()
  {
    // Status reg command sequence
    writeByteCommandShift(0x70);

    // Read the status register
    return readByte(0);
  }

  void eraseFlash29F1610()
  {
    // Erase command sequence
    writeByteCommandShift(0x80);
    writeByteCommandShift(0x10);

    // Read the status register
    busyWait();
  }

  /******************************************
    MX29LV flashrom functions
  *****************************************/
  void writeFlash29LV640()
  {
    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currByte = 0; currByte < fileSize; currByte += 512)
    {
      // Fill buffer
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++)
      {
        // Write command sequence
        writeByte(0x555 << 1, 0xAA);
        writeByte(0x2AA << 1, 0x55);
        writeByte(0x555 << 1, 0xA0);

        // Write current byte
        writeByte(currByte + c, OSCR::Storage::Shared::buffer[c]);

        // Check if write is complete
        busyWait(currByte + c, OSCR::Storage::Shared::buffer[c]);
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();
  }

  /******************************************
    S29GL flashrom functions
  *****************************************/
  void writeFlash29GL(uint32_t sectorSize, uint8_t bufferSize)
  {
    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currSector = 0; currSector < fileSize; currSector += sectorSize)
    {
      // Write to flashrom
      for (uint32_t currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::fill();

        // Write bufferSize bytes at a time
        for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += bufferSize)
        {
          // 2 unlock commands
          writeByte(0x555 << 1, 0xAA);
          writeByte(0x2AA << 1, 0x55);

          // Write buffer load command at sector address
          writeByte(currSector + currSdBuffer + currWriteBuffer, 0x25);

          // Write byte count (minus 1) at sector address
          writeByte(currSector + currSdBuffer + currWriteBuffer, bufferSize - 1);

          // Load bytes into buffer
          for (uint8_t currByte = 0; currByte < bufferSize; currByte++)
          {
            writeByte(currSector + currSdBuffer + currWriteBuffer + currByte, OSCR::Storage::Shared::buffer[currWriteBuffer + currByte]);
          }

          // Write Buffer to Flash
          writeByte(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1, 0x29);

          // Read the status register at last written address
          busyWait(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1, OSCR::Storage::Shared::buffer[currWriteBuffer + bufferSize - 1]);
        }
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(sectorSize);
    }

    OSCR::UI::ProgressBar::finish();
  }

  /******************************************
    29F800 functions
  *****************************************/
  void writeFlash29F800()
  {
    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    // Fill buffer
    for (uint32_t currByte = 0; currByte < fileSize; currByte += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++)
      {
        // Write command sequence
        writeByteCommandShift(0xA0);

        // Write current byte
        writeByte(currByte + c, OSCR::Storage::Shared::buffer[c]);
        busyCheck29F032(currByte + c, OSCR::Storage::Shared::buffer[c]);
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();
  }

  /******************************************
    28FXXX series flashrom functions
  *****************************************/
  void idFlash28FXXX()
  {
    writeByte(0x0, 0x90);

    // Read the two id bytes into a string
    flashid = readByte(0) << 8;
    flashid |= readByte(1);
  }

  void resetFlash28FXXX()
  {
    writeByte(0x0, 0xFF);

    delay(500);
  }

  uint8_t statusFlash28FXXX()
  {
    writeByte(0x0, 0x70);
    return readByte(0x0);
  }

  void eraseFlash28FXXX()
  {
    // only can erase block by block
    for (uint32_t ba = 0; ba < flashSize; ba += sectorSize)
    {
      writeByte(ba, 0x20);
      writeByte(ba, 0xD0);

      busyWait(ba);
    }
  }

  void writeFlash28FXXX()
  {
    if ((flashid == 0xB088))
      writeFlashLH28F0XX();
    else if ((flashid == 0x8916) || (flashid == 0x8917) || (flashid == 0x8918))
      writeFlashE28FXXXJ3A();
  }

  void writeFlashE28FXXXJ3A()
  {
    uint32_t block_addr;
    uint32_t block_addr_mask = ~(sectorSize - 1);

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    // Fill buffer
    for (uint32_t currByte = 0; currByte < fileSize; currByte += 512)
    {
      OSCR::Storage::Shared::fill();

      block_addr = currByte & block_addr_mask;

      for (uint32_t c = 0; c < 512; c += bufferSize)
      {
        // write to buffer start
        writeByte(block_addr, 0xE8);

        // waiting for buffer available
        busyWait(block_addr);

        // set write byte count
        writeByte(block_addr, bufferSize - 1);

        // filling buffer
        for (uint32_t d = 0; d < bufferSize; d++)
          writeByte(currByte + c + d, OSCR::Storage::Shared::buffer[c + d]);

        // start flashing page
        writeByte(block_addr, 0xD0);

        // waiting for finishing
        busyWait(block_addr);
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();
  }

  void writeFlashLH28F0XX()
  {

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    // Fill buffer
    for (uint32_t currByte = 0; currByte < fileSize; currByte += 512)
    {
      OSCR::Storage::Shared::fill();

      for (uint32_t c = 0; c < 512; c += bufferSize)
      {
        // sequence load to page
        writeByte(0x0, 0xE0);
        writeByte(0x0, bufferSize - 1);  // BCL
        writeByte(0x0, 0x00);            // BCH should be 0x00

        for (uint32_t d = 0; d < bufferSize; d++)
        {
          writeByte(d, OSCR::Storage::Shared::buffer[c + d]);
        }

        // start flashing page
        writeByte(0x0, 0x0C);
        writeByte(0x0, bufferSize - 1);  // BCL
        writeByte(currByte + c, 0x00);   // BCH should be 0x00

        // waiting for finishing
        busyWait(currByte + c);
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();
  }

  /******************************************
    Common flashrom functions
  *****************************************/
  void blankcheck()
  {
    printHeader();

    OSCR::UI::ProgressBar::init((uint32_t)(flashSize), 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    blank = true;

    resetFlash8();

    for (uint32_t currBuffer = 0; (currBuffer < flashSize) && (blank); currBuffer += 512)
    {
      // Fill buffer
      for (int c = 0; c < 512; c++)
      {
        // Read byte
        OSCR::Storage::Shared::buffer[c] = readByte(currBuffer + c);
      }

      // Check if all bytes are 0xFF
      for (uint32_t currByte = 0; currByte < 512; currByte++)
      {
        if (OSCR::Storage::Shared::buffer[currByte] != 0xFF)
        {
          currByte = 512;
          currBuffer = flashSize;
          blank = false;
          break;
        }
      }

      // Update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::UI::printLine(FS((blank) ? OSCR::Strings::Common::OK : OSCR::Strings::Common::FAIL));
  }

  void verifyFlash(bool closeFile)
  {
    verifyFlash(1, 1, 0, closeFile);
  }

  void verifyFlash(uint8_t currChip, uint8_t totalChips, bool reversed, bool closeFile)
  {
    blank = 0;

    // Adjust filesize to fit flashchip
    adjustFileSizeOffset(currChip, totalChips, reversed);

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currByte = 0; currByte < fileSize; currByte += 512)
    {
      if ((reversed) && (currChip == 1) && (totalChips == 1) && (fileSize == 8388608) && (currByte == 4194304))
      {
        OSCR::Storage::Shared::rewind();
      }

      if ((reversed) && (currChip == 1) && (totalChips == 1) && (fileSize == 6291456) && (currByte == 2097152))
      {
        OSCR::Storage::Shared::rewind();

        currByte = 4194304;
        fileSize = 8388608;
      }

      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++)
      {
        if (readByte(currByte + c) != OSCR::Storage::Shared::buffer[c])
        {
          blank++;
        }
      }

      // Update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    if (closeFile) OSCR::Storage::Shared::close();

    if (blank != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(blank);
    }
  }

  void readFlash()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Flash), NULL, "FL", FS(OSCR::Strings::FileType::Raw));

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(flashSize));

    for (uint32_t currByte = 0; currByte < flashSize; currByte += 512)
    {
      for (int c = 0; c < 512; c++)
      {
        OSCR::Storage::Shared::buffer[c] = readByte(currByte + c);
      }

      OSCR::Storage::Shared::writeBuffer();

      // Update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();
  }

  void printFlash(int numBytes)
  {
    for (int currByte = 0; currByte < numBytes; currByte += 10)
    {
      for (int c = 0; c < 10; c++)
      {
        // Now print the significant bits
        OSCR::UI::printHex<false>(readByte(currByte + c));
      }

      OSCR::UI::printLine();
    }

    OSCR::UI::update();
  }

  void resetFlash8()
  {
    switch (flashromType)
    {
      case 1: resetFlash29F032(); break;
      case 2: resetFlash29F1610(); break;
      case 3: resetFlash28FXXX(); break;
    }
  }

# if defined(ENABLE_FLASH16) || defined(NEEDS_FLASH16)
  /******************************************
    29L3211 16bit flashrom functions
  *****************************************/
  void resetFlash16()
  {
    // Reset command sequence
    writeWordCommand(0xF0);

    delay(500);
  }

  void writeFlash16()
  {
    // Fill buffer with 1 page at a time then write it repeat until all bytes are written
    for (uint32_t currByte = 0; currByte < fileSize / 2; currByte += 64)
    {
      OSCR::Storage::Shared::readBuffer(128);

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck16();

      // Write command sequence
      writeWordCommand(0xA0);

      // Write one full page at a time
      for (uint8_t c = 0, d = 0; c < 64; c++, d+=2)
      {
        uint16_t currWord = ((OSCR::Storage::Shared::buffer[d + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[d] & 0xFF);
        writeWord(currByte + c, currWord);
      }
    }

    // Check if write is complete
    busyCheck16();
  }

  void writeFlash16_29F1601()
  {
    // Fill buffer with 1 page at a time then write it repeat until all bytes are written
    for (uint32_t currByte = 0; currByte < fileSize / 2; currByte += 64)
    {
      OSCR::Storage::Shared::readBuffer(128);

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck16();

      // Write command sequence
      writeWordCommand(0xA0);

      // Write one full page at a time
      for (uint8_t c = 0, d = 0; c < 64; c++, d+=2)
      {
        uint16_t currWord = ((OSCR::Storage::Shared::buffer[d + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[d] & 0xFF);
        writeWord(currByte + c, currWord);

        if (c == 63)
        {
          // Write the last byte twice or else it won't write at all
          writeWord(currByte + c, currWord);
        }
      }
    }

    // Check if write is complete
    busyCheck16();
  }

  void idFlash16()
  {
    // ID command sequence
    writeWordCommand(0x90);

    // Read the two id bytes into a string
    flashid = (readWord(0) & 0xFF) << 8;
    flashid |= readWord(1) & 0xFF;
  }

  uint8_t readStatusReg16()
  {
    // Status reg command sequence
    writeWordCommand(0x70);

    // Read the status register
    return readWord(0);
  }

  void eraseFlash16()
  {
    printHeader();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    resetFlash16();

    // Erase command sequence
    writeWordCommand(0x80);
    writeWordCommand(0x10);

    busyCheck16();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void blankcheck16()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

    blank = 1;
    for (uint32_t currByte = 0; currByte < flashSize / 2; currByte++)
    {
      if (readWord(currByte) != 0xFFFF)
      {
        currByte = flashSize / 2;
        blank = 0;
      }
    }

    OSCR::UI::printLine(FS(blank ? OSCR::Strings::Common::Blank : OSCR::Strings::Common::NotBlank));
  }

  void verifyFlash16(bool closeFile)
  {
    blank = 0;

    for (uint32_t currByte = 0; currByte < fileSize / 2; currByte += 256)
    {
      OSCR::Storage::Shared::fill();

      for (uint16_t c = 0, d = 0; c < 256; c++, d+=2)
      {
        uint16_t currWord = ((OSCR::Storage::Shared::buffer[d + 1] << 8) | OSCR::Storage::Shared::buffer[d]);

        if (readWord(currByte + c) != currWord)
        {
          blank++;
        }
      }
    }

    if (blank != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(blank);
    }

    if (closeFile) OSCR::Storage::Shared::close();
  }

  void readFlash16()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Flash), NULL, "FL", FS(OSCR::Strings::FileType::Raw));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    resetFlash16();

    for (uint32_t currByte = 0; currByte < flashSize / 2; currByte += 256)
    {
      for (uint16_t c = 0, d = 0; c < 256; c++, d+=2)
      {
        uint16_t currWord = readWord(currByte + c);

        // Split word into two bytes
        OSCR::Storage::Shared::buffer[d + 1] = ((currWord >> 8) & 0xFF); // Right
        OSCR::Storage::Shared::buffer[d] = (currWord & 0xFF); // Left
      }

      OSCR::Storage::Shared::writeBuffer(512);
    }

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void printFlash16(int numBytes)
  {
    for (int currByte = 0; currByte < numBytes / 2; currByte += 5)
    {
      // 5 words per line
      for (int c = 0; c < 5; c++)
      {
        uint16_t const currWord = readWord(currByte + c);

        // Split word into two bytes
        uint8_t const left_byte = currWord & 0xFF;
        uint8_t const right_byte = (currWord >> 8) & 0xFF;

        OSCR::UI::printHex(left_byte);
        OSCR::UI::printHex<false>(right_byte);
      }

      OSCR::UI::printLine();
    }

    OSCR::UI::update();
  }

  // Delay between write operations based on status register
  void busyCheck16()
  {
    // Read the status register
    while ((readWord(0) | 0xFF7F) != 0xFFFF);
  }

  /******************************************
    MX29LV flashrom functions 16bit
  *****************************************/
  // Delay between write operations based on status register
  void busyCheck16_29LV640(uint32_t myAddress, uint16_t myData)
  {
    // Read the status register
    while ((readWord(myAddress) & 0x80) != (myData & 0x80));
  }

  void writeFlash16_29LV640()
  {
    for (uint32_t currWord = 0; currWord < fileSize / 2; currWord += 256)
    {
      OSCR::Storage::Shared::fill();

      for (uint16_t c = 0, d = 0; c < 256; c++, d+=2)
      {
        // Write command sequence
        writeWordCommand(0xA0);

        // Write current word
        uint16_t myWord = ((OSCR::Storage::Shared::buffer[d + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[d] & 0xFF);

        writeWord(currWord + c, myWord);

        // Check if write is complete
        busyCheck16_29LV640(currWord + c, myWord);
      }
    }
  }

  /******************************************
    Eprom functions
  *****************************************/
  uint16_t writeWord_Eprom(uint32_t myAddress, uint16_t myData)
  {
    // Data out
    DDRC = 0xFF;
    DDRA = 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(AVR_INS("nop"));

    // Set address
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;
    // Set data
    PORTC = myData;
    PORTA = (myData >> 8) & 0xFF;

    AVR_ASM(AVR_INS("nop"));

    // Switch VPP/OE(PH5) to HIGH
    PORTH |= (1 << 5);
    // Wait 1us for VPP High to Chip Enable Low
    delayMicroseconds(1);
    // Setting CE(PH6) LOW
    PORTH &= ~(1 << 6);

    // Leave VPP HIGH for 50us Chip Enable Program Pulse Width
    delayMicroseconds(55);

    // Setting CE(PH6) HIGH
    PORTH |= (1 << 6);
    // Wait 2us for Chip Enable High to VPP Transition
    delayMicroseconds(2);
    // Switch VPP/OE(PH5) to LOW
    PORTH &= ~(1 << 5);

    // Leave CE High for 1us for VPP Low to Chip Enable Low
    delayMicroseconds(1);

    // Data in
    DDRC = 0x00;
    DDRA = 0x00;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Setting CE(PH6) LOW
    PORTH &= ~(1 << 6);

    // Wait 1us for Chip Enable Low to Output Valid while program verify
    delayMicroseconds(3);

    // Read
    uint16_t tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Setting CE(PH6) HIGH
    PORTH |= (1 << 6);

    // Delay 130ns for Chip Enable High to Output Hi-Z
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    return tempWord;
  }

  uint16_t readWord_Eprom(uint32_t myAddress)
  {
    // Data in
    DDRC = 0x00;
    DDRA = 0x00;
    // Set address
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(AVR_INS("nop"));

    // Setting CE(PH6) LOW
    PORTH &= ~(1 << 6);

    // Delay for 100ns for Address Valid/Chip Enable Low to Output Valid
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read
    uint16_t tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

    // Setting CE(PH6) HIGH
    PORTH |= (1 << 6);

    return tempWord;
  }

  void blankcheck_Eprom()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

    blank = 1;

    for (uint32_t currWord = 0; currWord < flashSize / 2; currWord++)
    {
      if (readWord_Eprom(currWord) != 0xFFFF)
      {
        currWord = flashSize / 2;
        blank = 0;
      }
    }

    OSCR::UI::printLine(FS(blank ? OSCR::Strings::Common::Blank : OSCR::Strings::Common::NotBlank));
  }

  void read_Eprom()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::Flash), NULL, "FL", FS(OSCR::Strings::FileType::Raw));

    for (uint32_t currWord = 0; currWord < flashSize / 2; currWord += 256)
    {
      for (uint16_t c = 0, d = 0; c < 256; c++, d+=2)
      {
        uint16_t myWord = readWord_Eprom(currWord + c);

        // Split word into two bytes
        OSCR::Storage::Shared::buffer[d + 1] = ((myWord >> 8) & 0xFF); // Right
        OSCR::Storage::Shared::buffer[d] = (myWord & 0xFF); // Left
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    OSCR::Storage::Shared::close();
  }

  void write_Eprom()
  {
    // Switch VPP/OE(PH5) to HIGH
    PORTH |= (1 << 5);
    delay(1000);

    for (uint32_t currWord = 0; currWord < fileSize / 2; currWord += 256)
    {
      OSCR::Storage::Shared::fill();

      // Work through SD buffer
      for (uint16_t c = 0, d = 0; c < 256; c++, d+=2)
      {
        uint16_t checkWord;
        uint16_t myWord = ((OSCR::Storage::Shared::buffer[d + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[d] & 0xFF);

        // Error counter
        uint8_t n = 0;

        // Presto III allows up to 25 rewrites per word
        do
        {
          // Write word
          checkWord = writeWord_Eprom(currWord + c, myWord);

          // Check for fail
          if (n == 25)
          {
            OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::FatalError));

            OSCR::UI::printHex(currWord + c);

            OSCR::UI::print(FS(OSCR::Strings::Symbol::LabelEnd));

            OSCR::UI::printHex(readWord_Eprom(currWord + c));

            OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));

            OSCR::UI::printHex(myWord);

            OSCR::UI::fatalError(FS(OSCR::Strings::Symbol::Empty));
          }

          n++;
        }
        while (checkWord != myWord);
      }
    }
  }

  void verify_Eprom()
  {
    blank = 0;

    for (uint32_t currWord = 0; currWord < (fileSize / 2); currWord += 256)
    {
      OSCR::Storage::Shared::fill();

      for (uint16_t c = 0, d = 0; c < 256; c++, d+=2)
      {
        uint16_t myWord = (((OSCR::Storage::Shared::buffer[d + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[d] & 0xFF));

        if (readWord_Eprom(currWord + c) != myWord)
        {
          blank++;
        }
      }
    }

    if (blank != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(blank*2);
    }
  }

  void print_Eprom(int numBytes)
  {
    for (int currByte = 0; currByte < numBytes / 2; currByte += 5)
    {
      // 5 words per line
      for (int c = 0; c < 5; c++)
      {
        uint16_t const currWord = readWord_Eprom(currByte + c);

        // Split word into two bytes
        uint8_t const left_byte = currWord & 0xFF;
        uint8_t const right_byte = (currWord >> 8) & 0xFF;

        OSCR::UI::printHex(left_byte);
        OSCR::UI::printHex<false>(right_byte);
      }

      OSCR::UI::printLine();
    }

    OSCR::UI::update();
  }

# endif

  /******************************************
    CFI flashrom functions (modified from GB.ino)
  *****************************************/
  void sendCFICommand(uint8_t cmd)
  {
    writeByteCompensated(0xAAA, 0xAA);
    writeByteCompensated(0x555, 0x55);
    writeByteCompensated(0xAAA, cmd);
  }

  uint8_t readByteCompensated(int address)
  {
    uint8_t data = readByte(address >> (flashX16Mode ? 1 : 0));

    if (flashSwitchLastBits)
    {
      return (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
    }

    return data;
  }

  void writeByteCompensated(int address, uint8_t data)
  {
    if (flashSwitchLastBits)
    {
      data = (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
    }

    writeByte(address >> (flashX16Mode ? 1 : 0), data);
  }

  void startCFIMode(bool x16Mode)
  {
    if (x16Mode)
    {
      writeByte(0x555, 0xF0);  //x16 mode reset command
      delay(500);
      writeByte(0x555, 0xF0);  //Double reset to get out of possible Autoselect + CFI mode
      delay(500);
      writeByte(0x55, 0x98);  //x16 CFI Query command
    }
    else
    {
      writeByte(0xAAA, 0xF0);  //x8  mode reset command
      delay(100);
      writeByte(0xAAA, 0xF0);  //Double reset to get out of possible Autoselect + CFI mode
      delay(100);
      writeByte(0xAA, 0x98);  //x8 CFI Query command
    }
  }

  bool identifyCFIbyIds(bool x16Mode)
  {
    startCFIMode(x16Mode);

    if (!x16Mode)
    {
      uint8_t query8[3] = {
        readByte(0x20),
        readByte(0x22),
        readByte(0x24),
      };

      if ((query8[0] == 0x51) && (query8[1] == 0x52) && (query8[2] == 0x59))
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Names::NormalCFI8));

        flashX16Mode = false;
        flashSwitchLastBits = false;

        return true;
      }

      if ((query8[0] == 0x52) && (query8[1] == 0x51) && (query8[2] == 0x5A))
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Names::SwitchedCFI8));

        flashX16Mode = false;
        flashSwitchLastBits = true;

        return true;
      }
    }

    uint8_t query16[3] = {
      readByte(0x10),
      readByte(0x11),
      readByte(0x12),
    };

    if ((query16[0] == 0x51) && (query16[1] == 0x52) && (query16[2] == 0x59))
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Names::NormalCFI16));

      flashX16Mode = true;
      flashSwitchLastBits = false;

      return true;
    }

    if ((query16[0] == 0x52) && (query16[1] == 0x51) && (query16[2] == 0x5A))
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Names::SwitchedCFI16));

      flashX16Mode = true;
      flashSwitchLastBits = true;

      return true;
    }

    return false;
  }

  void identifyCFI()
  {
    writeByteCompensated(0xAAA, 0xF0);

    delay(100);

    // Trying x8 mode first
    if (!identifyCFIbyIds(false))
    {
      // Try x16 mode next
      if (!identifyCFIbyIds(true))
      {
        OSCR::UI::printLine(F("CFI Query failed!"));
        OSCR::UI::waitButton();
      }
    }

    flashBanks = 1 << (readByteCompensated(0x4E) - 14);  // - flashX16Mode);

    // Reset flash
    writeByteCompensated(0xAAA, 0xF0);

    delay(100);
  }

  // Adjust file size to fit flash chip and goto needed file offset
  void adjustFileSizeOffset(uint8_t currChip, uint8_t totalChips, bool reversed)
  {
    if ((currChip == 1) && (totalChips == 1)) // 1*2MB, 1*4MB or 1*8MB
    {
      if (reversed)
      {
        OSCR::Storage::Shared::sharedFile.seekSet(4194304);
      }
    }
    else if ((currChip == 1) && (totalChips == 2)) // 2*2MB or 2*4MB
    {
      if (reversed)
      {
        if (fileSize > 4194304)
        {
          fileSize = fileSize - flashSize / 2;
          OSCR::Storage::Shared::sharedFile.seekSet(4194304);
        }
        else
        {
          fileSize = 0;
        }
      }
      else if (fileSize > flashSize / 2)
      {
        fileSize = flashSize / 2;
      }
    }
    else if ((currChip == 2) && (totalChips == 2))
    {
      if (reversed)
      {
        fileSize = flashSize / 2;
        OSCR::Storage::Shared::rewind();
      }
      else if (fileSize > flashSize / 2)
      {
        fileSize = fileSize - flashSize / 2;
        OSCR::Storage::Shared::sharedFile.seekSet(flashSize / 2);
      }
      else
      {
        fileSize = 0;
      }
    }
    else if ((currChip == 1) && (totalChips == 4)) // 4*2MB
    {
      if (reversed)
      {
        if (fileSize > 4194304)
        {
          OSCR::Storage::Shared::sharedFile.seekSet(4194304);
          fileSize = 2097152;
        }
        else
        {
          fileSize = 0;
        }
      }
      else if (fileSize > 2097152)
      {
        fileSize = 2097152;
      }
    }
    else if ((currChip == 2) && (totalChips == 4))
    {
      if (reversed)
      {
        if (fileSize > 6291456)
        {
          OSCR::Storage::Shared::sharedFile.seekSet(6291456);
          fileSize = 2097152;
        }
        else
        {
          fileSize = 0;
        }
      }
      else
      {
        if (fileSize > 2097152)
        {
          OSCR::Storage::Shared::sharedFile.seekSet(2097152);
          fileSize = 2097152;
        }
        else
        {
          fileSize = 0;
        }
      }
    }
    else if ((currChip == 3) && (totalChips == 4))
    {
      if (reversed)
      {
        OSCR::Storage::Shared::sharedFile.seekSet(0);
        fileSize = 2097152;
      }
      else
      {
        if (fileSize > 4194304)
        {
          OSCR::Storage::Shared::sharedFile.seekSet(4194304);
          fileSize = 2097152;
        }
        else
        {
          fileSize = 0;
        }
      }

    }
    else if ((currChip == 4) && (totalChips == 4))
    {
      if (reversed)
      {
        if (fileSize > 2097152)
        {
          OSCR::Storage::Shared::sharedFile.seekSet(2097152);
          fileSize = 2097152;
        }
        else
          fileSize = 0;
      }
      else
      {
        if (fileSize > 6291456)
        {
          OSCR::Storage::Shared::sharedFile.seekSet(6291456);
          fileSize = 2097152;
        }
        else
        {
          fileSize = 0;
        }
      }
    }
    else // skip write
    {
      fileSize = 0;
    }
  }

  // Write flashrom
  void writeCFI(uint8_t currChip, uint8_t totalChips, bool reversed)
  {
    if (currChip == 1)
    {
      setup8();
      identifyCFI();
    }

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    // Print filepath
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    // Check size
    if ((flashSize == 8388608) && (fileSize < 6291456) && reversed)
    {
      OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::HardwareProblem));
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotSupportedByCart));
    }

    // Reset flash
    writeByteCompensated(0xAAA, 0xF0);

    delay(100);

    // Reset flash
    writeByte(0x555, 0xF0);

    delay(100);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    // Erase flash
    sendCFICommand(0x80);
    sendCFICommand(0x10);

    // After a completed erase D7 will output 1
    busyWait();

    // Adjust filesize to fit flashchip
    adjustFileSizeOffset(currChip, totalChips, reversed);

    // File offset indicator for SNES repros with multiple chips
    if ((totalChips > 1) || reversed)
    {
      OSCR::UI::print(currChip);
      OSCR::UI::print(FS(OSCR::Strings::Symbol::Slash));
      OSCR::UI::printLine(totalChips);
      OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));
    }

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currAddr = 0; currAddr < fileSize; currAddr += 512)
    {
      if ((reversed) && (currChip == 1) && (totalChips == 1) && (fileSize == 8388608) && (currAddr == 4194304))
      {
        OSCR::Storage::Shared::rewind();
      }

      if ((reversed) && (currChip == 1) && (totalChips == 1) && (fileSize == 6291456) && (currAddr == 2097152))
      {
        OSCR::Storage::Shared::rewind();

        currAddr = 4194304;
        fileSize = 8388608;
      }

      OSCR::Storage::Shared::fill();

      for (int currByte = 0; currByte < 512; currByte++)
      {
        // Write command sequence
        sendCFICommand(0xA0);

        // Write current byte
        writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);

        // Read the status register
        busyWait(currAddr + currByte);
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    verifyFlash();

    writeByteCompensated(0xAAA, 0xF0);
    delay(100);
  }

#endif /* ENABLE_FLASH8 */

} /* namespace OSCR::Cores::Flash */
