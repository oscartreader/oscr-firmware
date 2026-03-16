//***********************************************************
// SEGA MASTER SYSTEM / MARK III / GAME GEAR / SG-1000 MODULE
//***********************************************************
#include "config.h"

#if HAS_SMS
# include "cores/include.h"
# include "cores/SMS.h"

namespace OSCR::Cores::SMS
{
  //******************************************
  //   Menus
  //******************************************

  /**
   * Adapters Menu
   */
  // [Options] SMS
# if OPTION_SMS_ADAPTER == 0
  constexpr char const PROGMEM SMSAdapterRaphnet[]    = "SMS/MarkIII Raphnet";
  constexpr char const PROGMEM SMSAdapterRetrode[]    = "SMS Retrode";
  constexpr char const PROGMEM SMSAdapterRetron[]     = "SMS Retron3in1";
# endif /* OPTION_SMS_ADAPTER */

  // [Options] GameGear
# if OPTION_GG_ADAPTER == 0
  constexpr char const PROGMEM GGAdapterRetrode[]     = "Game Gear Retrode";
  constexpr char const PROGMEM GGAdapterRetron[]      = "Game Gear Retron3in1";
#endif /* OPTION_GG_ADAPTER */

  // [Options] SG-1000
# if OPTION_SG1000_ADAPTER == 0
  constexpr char const PROGMEM SG1000AdapterRaphnet[] = "SG-1000 Raphnet";
# endif /* OPTION_SG1000_ADAPTER */

  // [Menu] Text
  constexpr char const * const PROGMEM SMSAdapterMenu[] = {
    // SMS
# if OPTION_SMS_ADAPTER == 0
    SMSAdapterRaphnet,
    SMSAdapterRetrode,
    SMSAdapterRetron,
# else /* OPTION_SMS_ADAPTER > 0 */
    OSCR::Strings::Cores::SMS,
# endif /* OPTION_SMS_ADAPTER */

    // Game Gear
# if OPTION_GG_ADAPTER == 0
    GGAdapterRetrode,
    GGAdapterRetron,
# else /* OPTION_GG_ADAPTER > 0 */
    OSCR::Strings::Cores::GameGear,
# endif /* OPTION_GG_ADAPTER */

    // SG-1000
# if OPTION_SG1000_ADAPTER == 0
    SG1000AdapterRaphnet,
# else /* OPTION_SG1000_ADAPTER > 0 */
    OSCR::Strings::Cores::SG1000,
# endif /* OPTION_SG1000_ADAPTER */

    OSCR::Strings::MenuOptions::Back,
  };

  // [Menu] Checks
  #define HAS_SMS_RAPHNET     ((OPTION_SMS_ADAPTER == 0) || (OPTION_SMS_ADAPTER == 1))
  #define HAS_SMS_RETRODE     ((OPTION_SMS_ADAPTER == 0) || (OPTION_SMS_ADAPTER == 2))
  #define HAS_SMS_RETRON      ((OPTION_SMS_ADAPTER == 0) || (OPTION_SMS_ADAPTER == 3))
  #define HAS_GG_RETRODE      ((OPTION_GG_ADAPTER == 0) || (OPTION_GG_ADAPTER == 1))
  #define HAS_GG_RETRON       ((OPTION_GG_ADAPTER == 0) || (OPTION_GG_ADAPTER == 2))
  #define HAS_SG1000_RAPHNET  ((OPTION_SG1000_ADAPTER == 0) || (OPTION_SG1000_ADAPTER == 1))

  // [Menu] Enum for options
  enum class SMSMenuAdapters : uint8_t
  {
# if HAS_SMS_RAPHNET
    SMSRaphnet,
# endif
# if HAS_SMS_RETRODE
    SMSRetrode,
# endif
# if HAS_SMS_RETRON
    SMSRetron,
# endif
# if HAS_GG_RETRODE
    GGRetrode,
# endif
# if HAS_GG_RETRON
    GGRetron,
# endif
# if HAS_SG1000_RAPHNET
    SG1000Raphnet,
# endif
    Back,
  };

  // Operations menu
  constexpr char const * const SMSOperationMenu[] PROGMEM = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    OSCR::Strings::MenuOptions::SetSize,
    OSCR::Strings::MenuOptions::Back,
  };

  // SG Operations menu
  constexpr char const * const SGOperationsMenu[] PROGMEM = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::Back,
  };

  //
  // ROM Size Menus

  // [ROM Size Menu] SG-1000
  constexpr char const * const SG1RomSizeMenu[] PROGMEM = {
    OSCR::Strings::Units::Size8KB,
    OSCR::Strings::Units::Size16KB,
    OSCR::Strings::Units::Size24KB,
    OSCR::Strings::Units::Size32KB,
  };

  // [ROM Size Menu] SMS/GG
  constexpr char const * const SMSRomSizeMenu[] PROGMEM = {
    OSCR::Strings::Units::Size32KB,
    OSCR::Strings::Units::Size64KB,
    OSCR::Strings::Units::Size128KB,
    OSCR::Strings::Units::Size256KB,
    OSCR::Strings::Units::Size512KB,
    OSCR::Strings::Units::Size1MB,
    OSCR::Strings::MenuOptions::Back,
  };

  enum class SMSMode : uint8_t
  {
    SMSRetrode,
    GGRetrode,
    SMSRetron,
    GGRetron,
    SMSRaphnet,
    SG1000Raphnet,
  };

  SMSMode adapterMode;

  // Manual ROM Size Selection Flag
  bool manRomSizeSelected = false;

  constexpr char const PROGMEM TMRSEGA[] = "TMR SEGA";

  //*********************************************************
  //  Main menu with systems/adapters setups to choose from
  //*********************************************************
  void menu()
  {
    do
    {
      SMSMenuAdapters menuSelection = static_cast<SMSMenuAdapters>(OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetCartType), SMSAdapterMenu, sizeofarray(SMSAdapterMenu)));

      switch (menuSelection)
      {
      // SMS or MarkIII with raphnet adapter
# if HAS_SMS_RAPHNET
      case SMSMenuAdapters::SMSRaphnet:
        adapterMode = SMSMode::SMSRaphnet;
        break;
# endif /* HAS_SMS_RAPHNET */

      // SMS with Retrode adapter
# if HAS_SMS_RETRODE
      case SMSMenuAdapters::SMSRetrode:
        adapterMode = SMSMode::SMSRetrode;
        break;
# endif /* HAS_SMS_RETRODE */

      // SMS with Retron 3in1 adapter
# if HAS_SMS_RETRON
      case SMSMenuAdapters::SMSRetron:
        adapterMode = SMSMode::SMSRetron;
        break;
# endif /* HAS_SMS_RETRON */

      // GameGear with Retrode adapter
# if HAS_GG_RETRODE
      case SMSMenuAdapters::GGRetrode:
        adapterMode = SMSMode::GGRetrode;
        break;
# endif /* HAS_GG_RETRODE */

      // GameGear with Retron 3in1 adapter
# if HAS_GG_RETRON
      case SMSMenuAdapters::GGRetron:
        adapterMode = SMSMode::GGRetron;
        break;
# endif /* HAS_GG_RETRON */

      // SG-1000 with raphnet adapter
# if HAS_SG1000_RAPHNET
      case SMSMenuAdapters::SG1000Raphnet:
        adapterMode = SMSMode::SG1000Raphnet;
        break;
# endif /* HAS_SG1000_RAPHNET */

      case SMSMenuAdapters::Back:
        return;
      }

      operationsMenu();
    }
    while (true);
  }

  //****************************************************
  //  Create custom menu depending on selected setup
  //****************************************************
  void operationsMenu()
  {
    do
    {
      if (adapterMode == SMSMode::SG1000Raphnet)
      {
        switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::SG1000), SGOperationsMenu, sizeofarray(SGOperationsMenu)))
        {
        case 0: // Read ROM
          readROM();
          if (cartSize == 0) continue;
          break;

        default: // Back
          return;
        }
      }
      else
      {
        __FlashStringHelper const * menuHeader;

        switch (adapterMode)
        {
        case SMSMode::SMSRaphnet:
        case SMSMode::SMSRetrode:
        case SMSMode::SMSRetron:
          menuHeader = FS(OSCR::Strings::Cores::SMS);
          break;

        case SMSMode::GGRetrode:
        case SMSMode::GGRetron:
          menuHeader = FS(OSCR::Strings::Cores::GameGear);
          break;

        default:
          OSCR::resetArduino();
        }

        switch (OSCR::UI::menu(menuHeader, SMSOperationMenu, sizeofarray(SMSOperationMenu)))
        {
        case 0: // Read ROM
          readROM();
          if (cartSize == 0) continue;
          break;

        case 1: // Read SRAM
          readSRAM();
          break;

        case 2: // Write SRAM
          writeSRAM();
          break;

        case 3: // Select ROM Size
          manual_selectRomSize();
          break;

        default: // Back
          return;
        }
      }

      OSCR::UI::waitButton();
    }
    while(true);
  }

  void printHeader()
  {
    __FlashStringHelper const * menuHeader;

    switch (adapterMode)
    {
    case SMSMode::SG1000Raphnet:
      menuHeader = FS(OSCR::Strings::Cores::SG1000);
      break;

    case SMSMode::SMSRaphnet:
    case SMSMode::SMSRetrode:
    case SMSMode::SMSRetron:
      menuHeader = FS(OSCR::Strings::Cores::SMS);
      break;

    case SMSMode::GGRetrode:
    case SMSMode::GGRetron:
      menuHeader = FS(OSCR::Strings::Cores::GameGear);
      break;

    default:
      OSCR::Util::unreachable();
    }

    OSCR::UI::printHeader(menuHeader);
  }

  //********************************
  //   Setup I/O
  //********************************
  void cartOn()
  {
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A14
    DDRK = 0xFF;
    //A15
    DDRH |= (1 << 3);

    // For Retrode adapter
    if (adapterMode == SMSMode::SMSRetrode || adapterMode == SMSMode::GGRetrode)
    {
      PORTH &= ~((1 << 0) | (1 << 3) | (1 << 5));
      PORTL &= ~(1 << 1);
      DDRH &= ~((1 << 0) | (1 << 5));
      DDRL &= ~((1 << 1));
      // Set Control Pins to Output OE(PH6)
      DDRH |= (1 << 6);
      // WR(PL5) and RD(PL6)
      DDRL |= (1 << 5) | (1 << 6);
      // Setting OE(PH6) HIGH
      PORTH |= (1 << 6);
      // Setting WR(PL5) and RD(PL6) HIGH
      PORTL |= (1 << 5) | (1 << 6);
    }
    else // For Raphnet and Retron adapters
    {
      // Set Control Pins to Output RST(PH0) WR(PH5) OE(PH6)
      DDRH |= (1 << 0) | (1 << 5) | (1 << 6);
      // CE(PL1)
      DDRL |= (1 << 1);
      // Setting RST(PH0) WR(PH5) OE(PH6) HIGH
      PORTH |= (1 << 0) | (1 << 5) | (1 << 6);
      // CE(PL1)
      PORTL |= (1 << 1);
    }

    if (adapterMode != SMSMode::SG1000Raphnet)
    {
      // SMS/GG ROM has 16KB banks which can be mapped to one of three slots via register writes
      // Register Slot Address space
      // $fffd    0    $0000-$3fff
      // $fffe    1    $4000-$7fff
      // $ffff    2    $8000-$bfff

      // Disable sram
      writeByte(0xFFFC, 0);

      // Map first 3 banks so we can read-out the header info
      writeByte(0xFFFD, 0);
      writeByte(0xFFFE, 1);
      writeByte(0xFFFF, 2);
    }

    delay(400);

    // Read and print cart info only if ROM size not manually selected
    if (manRomSizeSelected == false)
    {
      getCartInfo();
    }
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  //*****************************************
  //  Low level functions
  //*****************************************
  void writeByte(uint16_t myAddress, uint8_t myData)
  {
    if (adapterMode == SMSMode::GGRetrode)
    {
      // Set Data Pins (D8-D15) to Output
      DDRA = 0xFF;
    }
    else
    {
      // Set Data Pins (D0-D7) to Output
      DDRC = 0xFF;
    }

    // Set address
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;

    if (!(adapterMode == SMSMode::SMSRetrode || adapterMode == SMSMode::GGRetrode))
    {
      // A15/M0-7(PH3) and OE(PH6) are connected
      PORTH = (PORTH & 0b11110111) | ((myAddress >> 12) & 0b00001000);
    }

    // Output data
    if (adapterMode == SMSMode::GGRetrode)
    {
      PORTA = myData;
    }
    else
    {
      PORTC = myData;
    }

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    if (adapterMode == SMSMode::SMSRetrode || adapterMode == SMSMode::GGRetrode)
    {
      // Switch WR(PL5) and OE/CE(PH6) to LOW
      PORTL &= ~(1 << 5);
      PORTH &= ~(1 << 6);
    }
    else
    {
      // Switch CE(PL1) and WR(PH5) to LOW
      PORTL &= ~(1 << 1);
      PORTH &= ~(1 << 5);
    }

    // Leave WR low for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    if (adapterMode == SMSMode::SMSRetrode || adapterMode == SMSMode::GGRetrode)
    {
      // Switch WR(PL5) and OE/CE(PH6) to HIGH
      PORTH |= (1 << 6);
      PORTL |= (1 << 5);
    }
    else
    {
      // Switch CE(PL1) and WR(PH5) to HIGH
      PORTH |= (1 << 5);
      PORTL |= (1 << 1);
    }

    // Leave WR high for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    if (adapterMode == SMSMode::GGRetrode)
    {
      // Set Data Pins (D8-D15) to Input
      DDRA = 0x00;
    }
    else
    {
      // Set Data Pins (D0-D7) to Input
      DDRC = 0x00;
    }
  }

  uint8_t readByte(uint16_t myAddress)
  {
    if (adapterMode == SMSMode::GGRetrode)
    {
      // Set Data Pins (D8-D15) to Input
      DDRA = 0x00;
    }
    else
    {
      // Set Data Pins (D0-D7) to Input
      DDRC = 0x00;
    }

    // Set Address
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    if (!(adapterMode == SMSMode::SMSRetrode || adapterMode == SMSMode::GGRetrode))
    {
      // A15/M0-7(PH3) and OE(PH6) are connected
      PORTH = (PORTH & 0b11110111) | ((myAddress >> 12) & 0b00001000);
    }

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    if (adapterMode == SMSMode::SMSRetrode || adapterMode == SMSMode::GGRetrode)
    {
      // Switch RD(PL6) and OE(PH6) to LOW
      PORTL &= ~(1 << 6);
      PORTH &= ~(1 << 6);
    }
    else
    {
      // Switch CE(PL1) and OE(PH6) to LOW
      PORTL &= ~(1 << 1);
      PORTH &= ~(1 << 6);
    }

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Read
    uint8_t tempByte = (adapterMode == SMSMode::GGRetrode) ? PINA : PINC;

    if (adapterMode == SMSMode::SMSRetrode || adapterMode == SMSMode::GGRetrode)
    {
      // Switch RD(PL6) and OE(PH6) to HIGH
      PORTH |= (1 << 6);
      PORTL |= (1 << 6);
    }
    else
    {
      // Switch CE(PL1) and OE(PH6) to HIGH
      PORTH |= (1 << 6);
      PORTL |= (1 << 1);
    }

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    return tempByte;
  }

  uint8_t readNibble(uint8_t data, uint8_t number)
  {
    return ((data >> (number * 4)) & 0xF);
  }

  //*****************************************
  //  Cartridges functions
  //*****************************************
  void getCartInfo()
  {
    printHeader();

    uint8_t cartNib = readNibble(readByte(0x7FFF), 0);

    // Get rom size from header
    //    Note: Common for this value to be smaller than the actual value.
    //          Normally used for BIOS checksum calculations on export hardware (non JP).
    switch (cartNib)
    {
    case 0xa:
      cartSize = 8192; // 8 KiB
      break;

    case 0xb:
      cartSize = 16384; // 16 KiB
      break;

    case 0xc:
      cartSize = 32768; // 32 KiB
      break;

    case 0xd:
      cartSize = 49152; // 48 KiB
      break;

    case 0xe:
      cartSize = 65536; // 64 KiB
      break;

    case 0xf:
      cartSize = 131072; // 128 KiB
      break;

    case 0x0:
      cartSize = 262144; // 256 KiB
      break;

    case 0x1:
    case 0x2:
      cartSize = 524288; // 512 KiB
      break;

    case 0x3:
      // 0x3 is (only?) used in The Pro Yakyuu '91 (Game Gear)
      cartSize = 131072; // 128 KiB
      break;

    default:
      cartSize = 49152; // 48 KiB
      break;
    }

    // Get rom name (expecting "TMR SEGA" string)
    for (uint8_t i = 0; i < 8; i++)
    {
      fileName[i] = char(readByte(0x7FF0 + i));
    }
    fileName[8] = '\0';

    // Attempt to detect cart size by checking if "TMR SEGA" is mirrored
    // Based on https://www.raphnet.net/electronique/sms_cartridge_programmer/index_en.php#6
    // Note: Logic does not work on US CloudMaster (256K) and Penquin Land (128K) SMS carts.
    //       Both detect as 512 KB based on the logic below.
    if (strcmp_P(fileName, TMRSEGA) == 0)
    {
      uint8_t bank = 1;
      char romNameBuf[9];

      while (bank < 64) // Total number of possible banks: 1 MiB / 16 (KiB/Bank)
      {
        bank++; // Increment the bank

        writeByte(0xFFFE, bank); // Load bank into slot 1

        for (uint8_t i = 0; i < 8; i++)
        {
          romNameBuf[i] = char(readByte(0x7FF0 + i));
        }
        romNameBuf[8] = '\0';

        // Debug info:
        // OSCR::UI::print(FS(OSCR::Strings::Labels::BANK));
        // OSCR::UI::printLine(bank);
        // OSCR::UI::print(F("Parsed ROM name: "));
        // OSCR::UI::printLine(romNameBuf);
        // OSCR::UI::printLine();

        if (strcmp(romNameBuf, fileName) == 0)
        {
          break;
        }
      }

      if (bank > 2) // 32 KiB is the smallest SMS/GG ROM size
      {
        cartSize = (bank - 1) * 16384UL;
      }

      // Debug info:
      // OSCR::UI::print(F("Calculated ROM Size: "));
      // OSCR::Lang::printBytesLine(cartSize);

      // Reset Bank Slot 1
      writeByte(0xFFFE, 1);
    }
    else // fileName != "TMR SEGA"
    {
      // Fix for "Fantasy Zone (J) (V1.0)" that has not the normal header, but "COPYRIGHT SEGAPRG. BY T.ASAI".
      char headerFZ[29];

      if (strcmp_P(fileName, PSTR("G. BY T.A")) != 0)
      {
        for (uint8_t i = 0; i < 28; i++)
        {
          headerFZ[i] = char(readByte(0x7FE0 + i));
        }

        headerFZ[28] = '\0';

        if (strcmp_P(headerFZ, PSTR("COPYRIGHT SEGAPRG. BY T.ASAI")) == 0)
        {
          strcpy_P(fileName, TMRSEGA);
          cartSize = 131072; // 128 KiB
        }
      }

      manual_selectRomSize();

      if (cartSize == 0) return;

      // Display cart info
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::HeaderNotFound));
    }

    OSCR::UI::printLine();

    OSCR::UI::print(FS(OSCR::Strings::Labels::NAME));
    OSCR::UI::printLine(fileName);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    OSCR::Lang::printBytesLine(cartSize);

    OSCR::UI::waitButton();
  }

  void manual_selectRomSize()
  {
    // Set rom size manually
    if (adapterMode == SMSMode::SG1000Raphnet)
    {
      // Rom sizes for SG-1000
      uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetROMSize), SG1RomSizeMenu, sizeofarray(SG1RomSizeMenu)); //sizeMenu.select()

      switch (menuSelection)
      {
      case 0:
        cartSize = 8192;   // 8 KiB
        break;

      case 1:
        cartSize = 16384;  // 16 KiB
        break;

      case 2:
        cartSize = 24576;  // 24 KiB
        break;

      case 3:
        cartSize = 32768;  // 32 KiB
        break;

      //case 4:
      //  cartSize = 40960; // 40KB
      //  break;

      //case 5:
      //  cartSize = 49152; // 48KB
      //  break;
      }
    }
    else
    {
      // Rom sizes for SMS and GG
      uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetROMSize), SMSRomSizeMenu, sizeofarray(SMSRomSizeMenu)); //sizeMenu.select()

      switch (menuSelection)
      {
      case 0:
        cartSize = 32768;    // 32 KiB
        break;

      case 1:
        cartSize = 65536;    // 64 KiB
        break;

      case 2:
        cartSize = 131072;   // 128 KiB
        break;

      case 3:
        cartSize = 262144;   // 256 KiB
        break;

      case 4:
        cartSize = 524288;   // 512 KiB
        break;

      case 5:
        cartSize = 1048576;  // 1 MiB
        break;

      case 6:
        cartSize = 0;
        return;
      }
    }

    strcpy_P(fileName, OSCR::Strings::Common::Unknown); // Use default UNKNOWN rom name upon manual selection
    manRomSizeSelected = true; // This ensures manually selected value is used upon read from the menu

    // Display info
    OSCR::UI::print(FS(OSCR::Strings::Labels::SIZE));
    OSCR::Lang::printBytesLine(cartSize);
  }

  //******************************************
  //  Read ROM and save it to the SD card
  //******************************************
  void readROM()
  {
    char const * target;
    char const * ext;

    // Compare dump checksum with database values
    switch (adapterMode)
    {
    case SMSMode::SMSRaphnet:
    case SMSMode::SMSRetrode:
    case SMSMode::SMSRetron:
      target = OSCR::Strings::FileType::SMS;
      ext = OSCR::Strings::FileType::SMS;
      break;

    case SMSMode::GGRetrode:
    case SMSMode::GGRetron:
      target = OSCR::Strings::FileType::GameGear;
      ext = OSCR::Strings::FileType::GameGear;
      break;

    case SMSMode::SG1000Raphnet:
      target = OSCR::Strings::Cores::SG1000;
      ext = OSCR::Strings::FileType::SG1000;
      break;

    default:
      OSCR::Util::unreachable();
    }

    cartOn();

    if (cartSize == 0)
    {
      cartOff();
      return;
    }

    printHeader();

    OSCR::Storage::Shared::createFile(FS(target), FS(OSCR::Strings::Directory::ROM), fileName, FS(ext));

    // Set default bank size to 16 KiB
    uint16_t bankSize = 16384;

    // For carts not using mappers (SG1000 or SMS/GG 32 KiB)
    if ((adapterMode == SMSMode::SG1000Raphnet) || (cartSize == 32768))
    {
      bankSize = cartSize;
    }

    OSCR::UI::ProgressBar::init(cartSize / bankSize); // Initialize progress bar

    for (uint8_t currBank = 0x0; currBank < (cartSize / bankSize); currBank++)
    {
      // Write current 16KB bank to slot 2 register 0xFFFF
      if (adapterMode != SMSMode::SG1000Raphnet)
      {
        writeByte(0xFFFF, currBank);
      }

      // Read 16KB from slot 2 which starts at 0x8000
      for (uint16_t currBuffer = 0; currBuffer < bankSize; currBuffer += 512)
      {
        // Fill SD buffer
        for (int currByte = 0; currByte < 512; currByte++)
        {
          OSCR::Storage::Shared::buffer[currByte] = readByte(((adapterMode == SMSMode::SG1000Raphnet) || (cartSize == 32768) ? 0 : 0x8000) + currBuffer + currByte);
        }
        OSCR::Storage::Shared::writeBuffer();
      }

      OSCR::UI::ProgressBar::advance(); // Advance progress bar
    }

    OSCR::UI::ProgressBar::finish(); // Update progress bar

    OSCR::Storage::Shared::close(); // Close file

    cartOff(); // Disable power

    crc32_t crcSearch = crc32_t(OSCR::Storage::Shared::getCRC32());

    if (OSCR::Databases::Basic::compareCRC(FS(target), crcSearch))
    {
      OSCR::Storage::Shared::rename_P(OSCR::Databases::Basic::crdb->record()->data()->name, FS(ext));
    }
  }

  //******************************************
  //  Read SRAM and save to the SD card
  ///*****************************************
  void readSRAM()
  {
    cartOn();

    printHeader();

    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(
      FS((adapterMode == SMSMode::GGRetrode || adapterMode == SMSMode::GGRetron) ? OSCR::Strings::FileType::GameGear : OSCR::Strings::FileType::SMS),
      FS(OSCR::Strings::Directory::Save),
      fileName,
      FS(OSCR::Strings::FileType::Save)
    );

    // Write the whole 32 KiB
    // When there is only 8 KiB of SRAM, the contents should be duplicated
    uint16_t bankSize = 16384;

    for (uint8_t currBank = 0x0; currBank < 2; currBank++)
    {
      writeByte(0xFFFC, 0x08 | (currBank << 2));

      // Read 16 KiB from slot 2 which starts at 0x8000
      for (uint16_t currBuffer = 0; currBuffer < bankSize; currBuffer += 512)
      {
        // Fill SD buffer
        for (int currByte = 0; currByte < 512; currByte++)
        {
          OSCR::Storage::Shared::buffer[currByte] = readByte(0x8000 + currBuffer + currByte);
        }

        OSCR::Storage::Shared::writeBuffer();
      }
    }

    cartOff();

    // Close file
    OSCR::Storage::Shared::close();
  }

  //**********************************************
  //  Read file from SD card and write it to SRAM
  //**********************************************
  void writeSRAM()
  {
    if (false)
    {
      OSCR::UI::error(F("DISABLED"));
      return;
    }

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    cartOn();

    // Get SRAM size from file, with a maximum of 32KB
    uint32_t const sramSize = OSCR::Util::minOf<uint32_t>(OSCR::Storage::Shared::getSize(), 32768U);

    OSCR::UI::print(FS(OSCR::Strings::Labels::SAVE_SIZE));
    OSCR::Lang::printBytesLine(sramSize);

    OSCR::UI::update();

    uint16_t bankSize = 16384;

    for (uint16_t address = 0x0; address < sramSize; address += 512)
    {
      uint8_t currBank = address >= bankSize ? 1 : 0;
      uint16_t page_address = address - (currBank * bankSize);

      writeByte(0xFFFC, 0x08 | (currBank << 2));

      OSCR::Storage::Shared::readBuffer();

      for (int x = 0; x < 512; x++)
      {
        writeByte(0x8000 + page_address + x, OSCR::Storage::Shared::buffer[x]);
      }
    }

    // Close file
    OSCR::Storage::Shared::close();

    cartOff();
  }
} /* namespace OSCR::Cores::SMS */

#endif /* HAS_SMS */
