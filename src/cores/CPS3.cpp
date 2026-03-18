//******************************************
// CP System III Module
// Cartridge
// 32/64/128 Mbits SIMM
// Tested with  HW5
// https://github.com/herzmx/CPS3-OSCR-Adapter
//******************************************
#include "config.h"

#if HAS_CPS3
# include "cores/include.h"
# include "cores/CPS3.h"
# include "cores/Flash.h"

namespace OSCR::Strings::CPS3
{
  constexpr char const SIMMWriter[] PROGMEM       = "CPS3 SIMM Writer";
  constexpr char const FujitsuDetected[] PROGMEM  = " Fujitsu SIMM detected";
}

namespace OSCR::Cores::CPS3
{
  using OSCR::Cores::Flash::menuOptionsFlash;
  using OSCR::Cores::Flash::kMenuOptionFlashMax;
  using OSCR::Cores::Flash::flashid;
  using OSCR::Cores::Flash::flashromType;
  using OSCR::Cores::Flash::flashSize;
  using OSCR::Cores::Flash::byteCtrl;
  using OSCR::Cores::Flash::blank;

  using OSCR::Cores::Flash::writeByteCommandShift;
  using OSCR::Cores::Flash::writeByte;
  using OSCR::Cores::Flash::busyCheck29F032;
  using OSCR::Cores::Flash::readWord;
  using OSCR::Cores::Flash::readByte;
  using OSCR::Cores::Flash::idFlash29F032;
  using OSCR::Cores::Flash::idFlash29F1610;
  using OSCR::Cores::Flash::idFlash28FXXX;
  using OSCR::Cores::Flash::printFlash;
  using OSCR::Cores::Flash::printFlash16;
  using OSCR::Cores::Flash::eraseFlash29F032;
  using OSCR::Cores::Flash::eraseFlash29F1610;
  using OSCR::Cores::Flash::eraseFlash28FXXX;
  using OSCR::Cores::Flash::writeWord;
  using OSCR::Cores::Flash::writeFlash29LV640;
  using OSCR::Cores::Flash::resetFlash8;
  using OSCR::Cores::Flash::blankcheck;
  using OSCR::Cores::Flash::id8;

  using OSCR::Databases::Basic::mapperDetail;
  using OSCR::Databases::Basic::mapperRecord;

  enum class ModeCPS3 : uint8_t
  {
    Cart,
    SIMM64,
    SIMM128,
    SIMM512,
    SIMM01,
  } modeCPS3;

  /******************************************
     Variables
  *****************************************/

  uint16_t flashids[8];

  /******************************************
     Menu
  *****************************************/
  // CPS3 start menu options
  constexpr char const OPTION_00[] PROGMEM = "CPS3 Cartridge";
  constexpr char const OPTION_01[] PROGMEM = "32/128 Mbits SIMM";
  constexpr char const OPTION_02[] PROGMEM = "64 Mbits SIMM";
  constexpr char const OPTION_03[] PROGMEM = "32/128 MBytes SIMM";
  constexpr char const OPTION_04[] PROGMEM = "64 MBytes SIMM";

  constexpr char const * const menuOptions[] PROGMEM = {
    OPTION_00,
    OPTION_01,
    OPTION_02,
    OPTION_03,
    OPTION_04,
    OSCR::Strings::MenuOptions::Back,
  };

  // CPS3 start menu
  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetCartType), menuOptions, sizeofarray(menuOptions)))
      {
      case 0:
        modeCPS3 = ModeCPS3::Cart;
        flashromCPS_Cartridge();
        break;

      case 1:
        modeCPS3 = ModeCPS3::SIMM128;
        flashromCPS_SIMM2x8();
        break;

      case 2:
        modeCPS3 = ModeCPS3::SIMM64;
        flashromCPS_SIMM4x8();
        break;

      case 3:
        modeCPS3 = ModeCPS3::SIMM01;
        flashromCPS_Cartridge();
        break;

      case 4:
        modeCPS3 = ModeCPS3::SIMM512;
        flashromCPS_Cartridge();
        break;

      case 5:
        closeCRDB();
        return;
      }
    }
    while (true);
  }

  // CPS3 Cartridge menu
  void flashromCPS_Cartridge()
  {
    CartCD cartCD;
    crc32_t crc32;

    if (modeCPS3 == ModeCPS3::Cart)
    {
      id8();
    }

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::CPS3), menuOptionsFlash, kMenuOptionFlashMax))
      {
      case 0:
        OSCR::UI::printLineSync(FS(OSCR::Strings::Headings::BlankCheck));
        resetFlash8();
        blankcheck();
        break;

      case 1:
        if (flashromType == 0)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        if (!OSCR::Prompts::confirmErase()) continue;

        printHeader();
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

        switch (flashromType)
        {
        case 1: eraseFlash29F032(); break;
        case 2: eraseFlash29F1610(); break;
        case 3: eraseFlash28FXXX(); break;
        }

        resetFlash8();
        break;

      case 2:
        resetFlash8();
        readCartridge();
        break;

      case 3:
        if (flashromType == 0)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

        // Calculate CRC32 of BIOS
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

        crc32 = OSCR::Storage::Shared::getCRC32();

        cartCD = setCartridgePatchData(crc32);

        switch (flashromType)
        {
        case 1:
          break;

        case 2:
          if ((flashid == 0x0458) || (flashid == 0x0158) || (flashid == 0x01AB) || (flashid == 0x0422) || (flashid == 0x0423))
          {
            writeCartridge(cartCD);
          }
          else if (flashid == 0x0) // Manual flash config, pick most common type
          {
            writeFlash29LV640();
            verifyCartridge(cartCD);
          }
          break;

        case 3:
          break;
        }

        delay(100);
        break;

      case 4:
        resetFlash8();

        printHeader();
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

        switch (flashromType)
        {
        case 0: break;
        case 1: idFlash29F032(); break;
        case 2: idFlash29F1610(); break;
        case 3: idFlash28FXXX(); break;
        }

        OSCR::UI::printLine();

        printFlash(40);

        OSCR::UI::printLineSync();

        resetFlash8();

        break;

      case 5:
        resetFlash8();
        printFlash(70);
        break;

      case 6:
        resetFlash8();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  // CPS3 32/128 SIMM menu
  void flashromCPS_SIMM2x8()
  {
    id_SIMM2x8();

    do
    {
      // 2x8bit
      uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::CPS3::SIMMWriter), menuOptionsFlash, kMenuOptionFlashMax);

      switch (menuSelection)
      {
      case 0:
        OSCR::UI::printLineSync(FS(OSCR::Strings::Headings::BlankCheck));

        resetSIMM2x8();
        blankcheckSIMM2x8();

        break;

      case 1:
        if (flashromType == 0)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        if (!OSCR::Prompts::confirmErase()) continue;

        printHeader();
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        switch (flashromType)
        {
        case 1:
          eraseSIMM2x8();
          break;

        default:
          break;
        }

        resetSIMM2x8();

        break;

      case 2:
        resetSIMM2x8();
        readSIMM2x8B();
        break;

      case 3:
        if (flashromType == 0)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        switch (flashromType)
        {
        case 1:
          if (flashid == 0x04AD)
            writeSIMM2x8();
          break;

        default:
          break;
        }
        break;

      case 4:
        resetFlash8();

        printHeader();

        switch (flashromType)
        {
        case 1:
          idFlash2x8(0x0);
          break;

        default:
          break;
        }

        OSCR::UI::printLine();

        printFlash16(40);

        OSCR::UI::printLineSync();

        resetSIMM2x8();

        break;

      case 5:
        resetSIMM2x8();
        printFlash16(70);
        break;

      case 6:
        resetSIMM2x8();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  // CPS3 32/128 SIMM menu
  void flashromCPS_SIMM4x8()
  {
    id_SIMM4x8();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::CPS3::SIMMWriter), menuOptionsFlash, kMenuOptionFlashMax))
      {
      case 0:
        OSCR::UI::printLineSync(FS(OSCR::Strings::Headings::BlankCheck));

        resetSIMM4x8();
        blankcheckSIMM4x8();

        break;

      case 1:
        if (flashromType == 0)
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        if (!OSCR::Prompts::confirmErase()) continue;

        printHeader();
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        switch (flashromType)
        {
        case 1:
          eraseSIMM4x8();
          break;

        default:
          break;
        }

        resetSIMM4x8();
        break;

      case 2:
        resetSIMM4x8();
        readSIMM4x8();
        break;

      case 3:
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
          if (flashid == 0x04AD)
            writeSIMM4x8();
          break;

        default:
          break;
        }

        break;

      case 4:
        resetFlash8();

        printHeader();
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

        switch (flashromType)
        {
        case 1:
          enable64MSB();
          idFlash2x8(0x0);
          enable64LSB();
          idFlash2x8(0x1);
          break;

        default:
          break;
        }

        OSCR::UI::printLine();

        printSIMM4x8(40);

        OSCR::UI::printLineSync();

        resetSIMM4x8();

        break;

      case 5:
        resetSIMM4x8();
        enable64MSB();
        printSIMM4x8(70);
        break;

      case 6:
        resetSIMM4x8();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::CPS3));
  }

  /******************************************
     Setup
  *****************************************/
  void cartOn()
  {
    //
    // Set Address Pins to Output

    //A0-A7
    DDRF = 0xFF;

    //A8-A15
    DDRK = 0xFF;

    //A16-A23
    DDRL = 0xFF;

    //A24(PJ0), A25(PJ1)
    DDRJ |= (1 << 0) | (1 << 1);

    //
    // Set Control Pins to Output

    // RST(PH0) OE(PH1) BYTE(PH3) WE(PH4) CKIO_CPU(PH5) CE/CER_CST(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // CE64LSB(PE3) CE128(PE4)
    DDRE |= (1 << 3) | (1 << 4);

    // RST_CPUC(PG1) CE64MSB(PG5)
    DDRG |= (1 << 1) | (1 << 5);

    // Set Data Pins (D0-D15) to Input
    DDRC = 0x00;
    DDRA = 0x00;

    // Disable Internal Pullups
    PORTC = 0x00;
    PORTA = 0x00;

    // Setting RST(PH0) OE(PH1) WE(PH4) CKIO_CPU(PH5) HIGH
    PORTH |= (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5);

    // Setting CE64LSB(PE3) CE128(PE4) HIGH
    PORTE |= (1 << 3) | (1 << 4);

    // Setting CE64MSB(PG5) HIGH
    PORTG |= (1 << 5);

    // Setting CE_CART/CER_CST(PH6) HIGH
    PORTH |= (1 << 6);

    // Setting BYTE(PH3) LOW
    PORTH &= ~(1 << 3);

    // Setting RST_CPUC(PG1) LOW
    PORTG &= ~(1 << 1);

    if (modeCPS3 == ModeCPS3::SIMM128 || modeCPS3 == ModeCPS3::SIMM01)
    {
      // Setting CE128(PE4) LOW
      PORTE &= ~(1 << 4);
    }
    else if (modeCPS3 == ModeCPS3::Cart)
    {
      // Setting CE_CART/CER_CST(PH6) LOW
      PORTH &= ~(1 << 6);
    }

    byteCtrl = 1;
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::CPS3));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  /******************************************
     Low level functions
  *****************************************/
  void enable64MSB()
  {
    // Setting CE64LSB(PE3) HIGH
    PORTE |= (1 << 3);
    // Setting CE64MSB(PG5) LOW
    PORTG &= ~(1 << 5);
    // Wait till output is stable
    NOP;
    NOP;
  }

  void enable64LSB()
  {
    // Setting CE64MSB(PG5) HIGH
    PORTG |= (1 << 5);
    // Setting CE64LSB(PE3) LOW
    PORTE &= ~(1 << 3);
    // Wait till output is stable
    NOP;
    NOP;
  }

  /******************************************
    Command functions
  *****************************************/
  void writeByteCommand_Flash2x8(uint32_t bank, uint8_t command)
  {
    writeWord((bank << 21) | 0x555, 0xAAAA);
    writeWord((bank << 21) | 0x2AA, 0x5555);
    writeWord((bank << 21) | 0x555, command << 8 | command);
  }

  /******************************************
    Cartridge functions
  *****************************************/
  void readCartridge()
  {
    setOutName_P(PSTR("29f400"));

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::CPS3), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::U2));

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

    OSCR::Databases::Basic::matchCRC();
  }

  void writeCartridge(CartCD cartCD)
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    fileSize = OSCR::Storage::Shared::getSize();

    // Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(fileSize));

    // Fill buffer
    for (uint32_t currByte = 0; currByte < fileSize; currByte += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++)
      {
        // Write command sequence
        writeByteCommandShift(0xA0);

        // Write patch to avoid region menu in multi
        if ((currByte + c) == 0x405F && cartCD.region > 0 && cartCD.multiCart == 1)
        {
          writeByte(currByte + c, cartCD.multiCartPatch1 & 0xFF);
          busyCheck29F032(currByte + c, cartCD.multiCartPatch1 & 0xFF);
          continue;
        }

        if ((currByte + c) == 0x20746 && cartCD.region > 0 && cartCD.multiCart == 1)
        {
          writeByte(currByte + c, cartCD.multiCartPatch2 & 0xFF);
          busyCheck29F032(currByte + c, cartCD.multiCartPatch2 & 0xFF);
          continue;
        }

        // Write cartridge region
        if ((currByte + c) == cartCD.regionOffset && cartCD.region > 0)
        {
          writeByte(currByte + c, cartCD.region & 0xFF);
          busyCheck29F032(currByte + c, cartCD.region & 0xFF);
          continue;
        }

        // Write cartridge cd
        if ((currByte + c) == cartCD.offsetNoCD && cartCD.disc > 0)
        {
          writeByte(currByte + c, cartCD.patch & 0xFF);
          busyCheck29F032(currByte + c, cartCD.patch & 0xFF);
          continue;
        }

        // Write current byte
        writeByte(currByte + c, OSCR::Storage::Shared::buffer[c]);
        busyCheck29F032(currByte + c, OSCR::Storage::Shared::buffer[c]);
      }

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    verifyCartridge(cartCD);

    OSCR::Storage::Shared::close();
  }

  void verifyCartridge(CartCD cartCD)
  {
    resetFlash8();
    resetFlash8();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    OSCR::Storage::Shared::rewind();

    writeErrors = 0;

    OSCR::UI::ProgressBar::init((uint32_t)(fileSize));

    for (uint32_t currByte = 0; currByte < fileSize; currByte += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++)
      {
        // Verify patch to avoid region menu
        if ((currByte + c) == 0x405F && cartCD.region > 0 && cartCD.multiCart == 1 && readByte(currByte + c) == (cartCD.multiCartPatch1 & 0xFF))
        {
          continue;
        }
        else if ((currByte + c) == 0x20746 && cartCD.region > 0 && cartCD.multiCart == 1 && readByte(currByte + c) == (cartCD.multiCartPatch2 & 0xFF))
        {
          continue;
          // Verify cartridge region
        }
        else if ((currByte + c) == cartCD.regionOffset && cartCD.region > 0 && readByte(currByte + c) == (cartCD.region & 0xFF))
        {
          continue;
        }
        else if ((currByte + c) == cartCD.offsetNoCD && cartCD.disc > 0 && readByte(currByte + c) == (cartCD.patch & 0xFF)) // Verify cartridge cd
        {
          continue;
        }
        else if (readByte(currByte + c) != OSCR::Storage::Shared::buffer[c])
        {
          writeErrors++;
        }
      }

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();

    if (writeErrors == 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }
  }

  CartCD setCartridgePatchData(crc32_t & crc32)
  {
    CartCD cartCD;

    if (!OSCR::Databases::Extended::searchDatabase(&crc32))
    {
      OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CRDB));
      OSCR::UI::print(FS(OSCR::Strings::Labels::CRCSum));
      OSCR::UI::printLine(crc32);
      OSCR::UI::error(FS(OSCR::Strings::Errors::NotFoundDB));
      return cartCD;
    }

    return cartCD;
  }

  /******************************************
    2x8bit SIMM functions
  *****************************************/
  void id_SIMM2x8()
  {
    uint8_t ngFlash = 0;
    uint8_t okFlash = 0;

    idFlash2x8(0x0);
    idFlash2x8(0x1);
    idFlash2x8(0x2);
    idFlash2x8(0x3);

    resetSIMM2x8();

    if (flashids[6] == flashids[7])
    {
      flashid = flashids[7];
      okFlash = 2;

      for (uint8_t i = 0; i < 6; i++)
      {
        if (flashid == flashids[i])
        {
          okFlash += 1;
        }
        else
        {
          ngFlash += 1;
        }
      }
    }

    // Print start screen
    OSCR::UI::printHeader(FS(OSCR::Strings::CPS3::SIMMWriter));
    OSCR::UI::print(FS(OSCR::Strings::Labels::ID));
    OSCR::UI::printHexLine(flashid);

    if (flashid == 0x04AD)
    {
      if (okFlash == 2 && ngFlash == 6)
      {
        flashromType = 1;
      }
      else if (okFlash == 8)
      {
        flashromType = 1;
      }
      else if (okFlash > 2)
      {
        flashromType = 0;
      }

      flashSize = 0x200000 * okFlash;

      OSCR::Lang::printBits(flashSize * 8);
      OSCR::UI::printLine(FS(OSCR::Strings::CPS3::FujitsuDetected));
    }
    else
    {
      // ID not found
      flashSize = 0x1000000;
      flashromType = 0;

      OSCR::UI::printHeader(FS(OSCR::Strings::CPS3::SIMMWriter));

      OSCR::UI::print(FS(OSCR::Strings::Labels::ID));
      OSCR::UI::print(vendorID);

      OSCR::UI::print(FS(OSCR::Strings::Symbol::MenuSpaces));
      OSCR::UI::printHexLine(flashid);

      OSCR::UI::printLine(FS(OSCR::Strings::Common::Unknown));

      OSCR::UI::waitButton();

      // print first 40 bytes of flash
      printFlash16(40);
    }

    resetSIMM2x8();

    OSCR::UI::waitButton();
  }

  void resetSIMM2x8()
  {
    resetFlash2x8(0x3);
    resetFlash2x8(0x2);
    resetFlash2x8(0x1);
    resetFlash2x8(0x0);
  }

  void blankcheckSIMM2x8()
  {
    OSCR::UI::ProgressBar::init((uint32_t)(flashSize), 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    blank = 1;

    for (uint32_t currBuffer = 0; currBuffer < flashSize / 2; currBuffer += 256)
    {
      // Fill buffer
      for (uint16_t c = 0, d = 0, currWord = 0; c < 256; c++, d+=2)
      {
        currWord = readWord(currBuffer + c);
        OSCR::Storage::Shared::buffer[d + 1] = ((currWord >> 8) & 0xFF); // Read byte right
        OSCR::Storage::Shared::buffer[d] = (currWord & 0xFF); // Read byte left
      }

      // Check if all bytes are 0xFF
      for (uint16_t currByte = 0, d = 0; currByte < 256; currByte++, d+=2)
      {
        if (OSCR::Storage::Shared::buffer[d] != 0xFF || OSCR::Storage::Shared::buffer[d + 1] != 0xFF)
        {
          currByte = 256;
          currBuffer = flashSize / 2;
          blank = 0;
        }
      }

      // Update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::printLine(FS((blank) ? OSCR::Strings::Common::Blank : OSCR::Strings::Common::NotBlank));

    OSCR::UI::ProgressBar::finish();
  }

  // From readFlash
  void readSIMM2x8()
  {
    setOutName_P(OSCR::Strings::Units::Size128MB);

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::CPS3), FS(OSCR::Strings::Directory::SIMM), fileName, FS(OSCR::Strings::FileType::Raw));

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(flashSize));

    for (uint32_t currByte = 0; currByte < flashSize / 2; currByte += 256)
    {
      for (uint16_t c = 0, d = 0, currWord = 0; c < 256; c++, d+=2)
      {
        currWord = readWord(currByte + c);
        OSCR::Storage::Shared::buffer[d + 1] = ((currWord >> 8) & 0xFF); // Right
        OSCR::Storage::Shared::buffer[d] = (currWord & 0xFF); // Left
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();
  }

  // From readFlash16
  void readSIMM2x8B()
  {
    setOutName_P(OSCR::Strings::Units::Size128MB);

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::CPS3), FS(OSCR::Strings::Directory::SIMM), fileName, FS(OSCR::Strings::FileType::Raw));

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(flashSize));

    for (uint32_t currByte = 0; currByte < flashSize / 2; currByte += 256)
    {
      for (uint16_t c = 0, d = 0, currWord = 0; c < 256; c++, d+=2)
      {
        currWord = readWord(currByte + c);
        OSCR::Storage::Shared::buffer[d + 1] = ((currWord >> 8) & 0xFF); // Right
        OSCR::Storage::Shared::buffer[d] = (currWord & 0xFF); // Left
      }

      OSCR::Storage::Shared::writeBuffer();

      // Update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();
  }

  void eraseSIMM2x8()
  {
    eraseFlash2x8(0x3);
    eraseFlash2x8(0x2);
    eraseFlash2x8(0x1);
    eraseFlash2x8(0x0);
  }

  // From writeFlash29F032
  void writeSIMM2x8()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    cartOn();

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(fileSize));

    // Fill buffer LSB
    for (uint32_t currByte = 0; currByte < fileSize / 2; currByte += 256)
    {
      OSCR::Storage::Shared::fill();

      noInterrupts();

      for (int c = 0; c < 256; c++)
      {
        uint16_t myWord = ((OSCR::Storage::Shared::buffer[(c * 2) + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[c * 2] & 0xFF);
        uint32_t simmAddress = currByte + c;
        uint16_t wordFlash = readWord(simmAddress);

        // Skip if data exist in flash
        if (wordFlash == myWord || myWord == 0xFFFF)
        {
          continue;
        }

        // Write command sequence
        writeByteCommand_Flash2x8(simmAddress >> 21, 0xA0);

        // Write current word
        writeWord(simmAddress, myWord);
        busyCheck2x8(simmAddress, myWord);
      }

      interrupts();

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    delay(100);

    // Reset twice just to be sure
    resetSIMM2x8();
    resetSIMM2x8();

    OSCR::Storage::Shared::rewind();

    blank = 0;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(fileSize));

    for (uint32_t currByte = 0; currByte < fileSize / 2; currByte += 256)
    {
      //fill buffer
      OSCR::Storage::Shared::fill();

      for (uint16_t c = 0, d = 0; c < 256; c++, d+=2)
      {
        if (readWord(currByte + c) != ((uint16_t)(OSCR::Storage::Shared::buffer[d + 1] << 8) | (uint16_t)OSCR::Storage::Shared::buffer[d]))
        {
          blank++;
        }
      }

      // Update progress bar
      OSCR::UI::ProgressBar::advance(256);
    }

    OSCR::UI::ProgressBar::finish();

    if (blank != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(blank);
    }

    OSCR::Storage::Shared::close();
  }

  /******************************************
    4x8bit SIMM functions
  *****************************************/
  void id_SIMM4x8()
  {
    uint8_t ngFlash = 0;
    uint8_t okFlash = 0;

    printHeader();
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    enable64MSB();
    idFlash2x8(0x0);
    enable64LSB();
    idFlash2x8(0x1);
    resetSIMM4x8();

    flashid = flashids[7];

    for (uint8_t i = 4; i < 8; i++)
    {
      if (flashid == flashids[i])
      {
        okFlash += 1;
      }
      else
      {
        ngFlash += 1;
      }
    }

    resetSIMM4x8();

    OSCR::UI::clearLine();
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Searching));

    if (!OSCR::Databases::Basic::matchMapper(flashid))
    {
      // ID not found
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));

      OSCR::UI::print(FS(OSCR::Strings::Labels::ID));
      OSCR::UI::printLine(vendorID);

      OSCR::UI::print(FS(OSCR::Strings::Labels::ID));
      OSCR::UI::printLine(flashid);

      OSCR::UI::waitButton();

      // print first 40 bytes of flash
      printFlash16(40);

      resetSIMM4x8();
      return;
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

    flashSize = mapperDetail->sizeLow * 4;
    flashromType = mapperDetail->meta1;

    if (okFlash != 4)
    {
      flashromType = 0;
      OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
    }

    OSCR::UI::print(FS(OSCR::Strings::Labels::NAME));
    OSCR::UI::printLineSync(mapperDetail->name);

    if (okFlash != 4)
    {
      OSCR::UI::printLine();
      OSCR::UI::error(FS(OSCR::Strings::Headings::HardwareProblem));
    }
  }

  void resetSIMM4x8()
  {
    enable64MSB();
    resetFlash2x8(0x0);
    enable64LSB();
    resetFlash2x8(0x0);
  }

  void blankcheckSIMM4x8()
  {
    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(flashSize));

    blank = 1;

    for (uint32_t currBuffer = 0; currBuffer < flashSize / 4; currBuffer += 128)
    {
      // Fill buffer
      for (uint16_t c = 0, d = 0, currWord = 0; c < 128; c++, d+=4)
      {
        enable64MSB();
        currWord = readWord(currBuffer + c);
        OSCR::Storage::Shared::buffer[d + 1] = ((currWord >> 8) & 0xFF); // Read byte right
        OSCR::Storage::Shared::buffer[d] = (currWord & 0xFF); // Read byte left

        enable64LSB();
        currWord = readWord(currBuffer + c);
        OSCR::Storage::Shared::buffer[d + 3] = ((currWord >> 8) & 0xFF); // Read byte right
        OSCR::Storage::Shared::buffer[d + 2] = (currWord & 0xFF); // Read byte left
      }

      // Check if all bytes are 0xFF
      for (uint32_t currByte = 0, d = 0; currByte < 128; currByte++, d+=4)
      {
        if (OSCR::Storage::Shared::buffer[d] != 0xFF || OSCR::Storage::Shared::buffer[d + 1] != 0xFF || OSCR::Storage::Shared::buffer[d + 2] != 0xFF || OSCR::Storage::Shared::buffer[d + 3] != 0xFF)
        {
          currByte = 128;
          currBuffer = flashSize / 4;
          blank = 0;
        }
      }

      // Update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::UI::printLine(FS((blank) ? OSCR::Strings::Common::Blank : OSCR::Strings::Common::NotBlank));
  }

  void eraseSIMM4x8()
  {
    enable64MSB();
    eraseFlash2x8(0x0);
    enable64LSB();
    eraseFlash2x8(0x0);
  }

  // From readFlash16
  void readSIMM4x8()
  {
    strncpy_P(fileName, OSCR::Strings::Units::Size64MB, kFileNameMax);
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::CPS3), FS(OSCR::Strings::Directory::SIMM), fileName, FS(OSCR::Strings::FileType::Raw));

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(flashSize));

    for (uint32_t currByte = 0; currByte < flashSize / 4; currByte += 128)
    {
      for (uint16_t c = 0, d = 0, currWord = 0; c < 128; c++, d+=4)
      {
        enable64MSB();
        currWord = readWord(currByte + c);
        OSCR::Storage::Shared::buffer[d + 1] = ((currWord >> 8) & 0xFF); // Right
        OSCR::Storage::Shared::buffer[d] = (currWord & 0xFF); // Left

        enable64LSB();
        currWord = readWord(currByte + c);
        OSCR::Storage::Shared::buffer[d + 3] = ((currWord >> 8) & 0xFF); // Right
        OSCR::Storage::Shared::buffer[d + 2] = (currWord & 0xFF); // Left
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();
  }

  void writeSIMM4x8()
  {
    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(fileSize));

    // Fill buffer
    for (uint32_t currByte = 0; currByte < fileSize / 4; currByte += 128)
    {
      OSCR::Storage::Shared::fill();

      noInterrupts();

      for (int c = 0; c < 128; c++)
      {
        // 0600 0EA0
        enable64MSB();
        // 0006
        uint16_t myWord = ((uint16_t)(OSCR::Storage::Shared::buffer[(c * 4) + 1] & 0xFF) << 8) | (uint16_t)(OSCR::Storage::Shared::buffer[(c * 4)] & 0xFF);

        // Write command sequence
        writeByteCommand_Flash2x8(0x0, 0xA0);

        // Write current word
        writeWord(currByte + c, myWord);
        busyCheck2x8(currByte + c, myWord);

        enable64LSB();

        // A00E
        myWord = ((uint16_t)(OSCR::Storage::Shared::buffer[(c * 4) + 3] & 0xFF) << 8) | (uint16_t)(OSCR::Storage::Shared::buffer[(c * 4) + 2] & 0xFF);

        // Write command sequence
        writeByteCommand_Flash2x8(0x0, 0xA0);

        // Write current word
        writeWord(currByte + c, myWord);
        busyCheck2x8(currByte + c, myWord);
      }

      interrupts();

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();

    delay(100);

    // Reset twice just to be sure
    resetSIMM4x8();
    resetSIMM4x8();

    blank = 0;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(fileSize));

    for (uint32_t currByte = 0; currByte < fileSize / 4; currByte += 128)
    {
      OSCR::Storage::Shared::fill();

      for (uint16_t c = 0, d = 0; c < 128; c++, d+=4)
      {
        enable64MSB();
        if (readWord(currByte + c) != ((uint16_t)(OSCR::Storage::Shared::buffer[d + 1] << 8) | (uint16_t)OSCR::Storage::Shared::buffer[d]))
        {
          blank++;
        }

        enable64LSB();
        if (readWord(currByte + c) != ((uint16_t)(OSCR::Storage::Shared::buffer[d + 3] << 8) | (uint16_t)OSCR::Storage::Shared::buffer[d + 2]))
        {
          blank++;
        }
      }

      // Update progress bar
      OSCR::UI::ProgressBar::advance(128);
    }

    OSCR::UI::ProgressBar::finish();

    if (blank == 0)
    {
      OSCR::Lang::printErrorVerifyBytes(blank);
    }

    OSCR::Storage::Shared::close();
  }

  void printSIMM4x8(int numBytes)
  {
    for (int currByte = 0; currByte < numBytes / 4; currByte += 4)
    {
      // 2 dwords per line
      for (uint8_t c = 0; c < 2; c++)
      {
        enable64MSB();
        OSCR::UI::printHex(readWord(currByte + c));

        OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));

        enable64LSB();
        OSCR::UI::printHex(readWord(currByte + c));
      }

      OSCR::UI::printLine();
    }

    OSCR::UI::update();
  }

  /******************************************
    SIMM 2x8bit flashrom functions
  *****************************************/

  void resetFlash2x8(uint32_t bank)
  {
    // Reset command sequence
    writeByteCommand_Flash2x8(bank, 0xF0);
    delay(500);
  }

  void idFlash2x8(uint32_t bank)
  {
    // ID command sequence
    writeByteCommand_Flash2x8(bank, 0x90);

    // Read the two id bytes into a string
    flashids[(bank * 2)] = (readWord((bank << 21) | 0) >> 8) << 8;
    flashids[(bank * 2)] |= readWord((bank << 21) | 1) >> 8;

    // Read the two id bytes into a string
    flashids[(bank * 2) + 1] = (readWord((bank << 21) | 0) & 0xFF) << 8;
    flashids[(bank * 2) + 1] |= readWord((bank << 21) | 1) & 0xFF;
  }

  // From eraseFlash29F032
  void eraseFlash2x8(uint32_t bank)
  {
    // Erase command sequence
    writeByteCommand_Flash2x8(bank, 0x80);
    writeByteCommand_Flash2x8(bank, 0x10);

    // Read the status register
    uint16_t statusReg = readWord((bank << 21) | 0);

    // After a completed erase D7 and D15 will output 1
    while ((statusReg & 0x8080) != 0x8080)
    {
      delay(100);

      // Update Status
      statusReg = readWord((bank << 21) | 0);
    }
  }

  // From busyCheck29F032
  int busyCheck2x8(uint32_t addr, uint16_t c)
  {
    int ret = 0;

    // Setting OE(PH1) LOW
    PORTH &= ~(1 << 1);
    // Setting WE(PH4) HIGH
    PORTH |= (1 << 4);
    NOP;
    NOP;

    //When the Embedded Program algorithm is complete, the device outputs the datum programmed to D7 and D15
    for (;;)
    {
      uint16_t d = readWord(addr);
      if ((d & 0x8080) == (c & 0x8080))
      {
        break;
      }

      if ((d & 0x2020) == 0x2020)
      {
        // From the datasheet:
        // DQ 5 will indicate if the program or erase time has exceeded the specified limits (internal pulse count).
        // Under these conditions DQ 5 will produce a “1”.
        // This is a failure condition which indicates that the program or erase cycle was not successfully completed.
        // Note : DQ 7 is rechecked even if DQ 5 = “1” because DQ 7 may change simultaneously with DQ 5 .
        d = readWord(addr);

        if ((d & 0x8080) != (c & 0x8080))
        {
          ret = 1;
        }

        break;
      }
    }

    // Setting OE(PH1) HIGH
    PORTH |= (1 << 1);
    NOP;
    NOP;

    return ret;
  }
} /* namespace OSCR::Cores::CPS3 */

#endif /* HAS_CPS3 */
