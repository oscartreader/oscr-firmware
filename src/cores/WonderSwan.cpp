//*********************************************************
// BANDAI WONDERSWAN & BENESSE POCKET CHALLENGE V2 MODULE
//*********************************************************
#include "config.h"

#if HAS_WS
# include "cores/include.h"
# include "cores/WonderSwan.h"

namespace OSCR::Cores::WonderSwan
{
  // Cartridge pinout
  // 48P 1.25mm pitch connector
  // C1, C48     : GND
  // C24, C25    : VDD (+3.3v)
  // C16-C23     : D7-D0
  // C34-C39     : D8-D13
  // C14-C15     : D15-D14
  // C26-C29     : A(-1)-A2
  // C10-C13     : A6-A3
  // C30-C33     : A18-A15
  // C2,C3,C4,C5 : A14,A9,A10,A8
  // C6,C7,C8,C9 : A7,A12,A13,A11
  // C40         : /RST
  // C41         : /IO? (only use when unlocking MMC)
  // C42         : /MMC (access port on cartridge with both /CART and /MMC = L)
  // C43         : /OE
  // C44         : /WE
  // C45         : /CART? (L when accessing cartridge (ROM/SRAM/PORT))
  // C46         : INT (for RTC alarm interrupt)
  // C47         : CLK (384KHz on WS)

  using OSCR::Databases::Basic::crdbRecord;
  using OSCR::Databases::Basic::crdb;
  using OSCR::Databases::Basic::romDetail;
  using OSCR::Databases::Basic::romRecord;

  using OSCR::Databases::Basic::mapperRecord;
  using OSCR::Databases::Basic::mapperDetail;

  constexpr char const PROGMEM menuOptionWriteWitchOS[] = "Write WitchOS";

  constexpr uint8_t const PROGMEM wwLaunchCode[] = {
    0xEA,
    0x00,
    0x00,
    0x00,
    0xE0,
    0x00,
    0xFF,
    0xFF,
  };

  uint8_t refreshCart(__FlashStringHelper const * menuOptions[], MenuOption * const menuOptionMap, bool & hasWitchOS)
  {
    uint8_t option = 0;

    hasWitchOS = getCartInfo();

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadROM);
    menuOptionMap[option] = MenuOption::ReadROM;
    option++;

    if (saveType == 1 || saveType == 2)
    {
      menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadSave);
      menuOptionMap[option] = MenuOption::ReadSave;
      option++;

      menuOptions[option] = FS(OSCR::Strings::MenuOptions::WriteSave);
      menuOptionMap[option] = MenuOption::WriteSave;
      option++;
    }

    if (hasWitchOS)
    {
      menuOptions[option] = FS(menuOptionWriteWitchOS);
      menuOptionMap[option] = MenuOption::WriteWitchOS;
      option++;
    }

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::RefreshCart);
    menuOptionMap[option] = MenuOption::RefreshCart;
    option++;

    // Fill the rest of the array with the "Back" option
    //  (just in case something tries to read it)
    for (uint8_t i = option; i < kMenuOptionMax; i++)
    {
      menuOptions[i] = FS(OSCR::Strings::MenuOptions::Back);
      menuOptionMap[i] = MenuOption::Back;
    }

    // Increment once so the first "Back" option shows
    return ++option;
  }

  void menu()
  {
    bool hasWitchOS = false;
    __FlashStringHelper const * menuOptions[kMenuOptionMax] = {};
    MenuOption menuOptionMap[kMenuOptionMax] = {};
    uint8_t menuOptionCount = refreshCart(menuOptions, menuOptionMap, hasWitchOS);

    openCRDB();

    do
    {
      switch (menuOptionMap[OSCR::UI::menu(FS(OSCR::Strings::Cores::WonderSwan), menuOptions, menuOptionCount)])
      {
      case MenuOption::ReadROM:
        readROM(hasWitchOS);
        break;

      case MenuOption::ReadSave:
        readSave();
        break;

      case MenuOption::WriteSave:
        writeSave();
        break;

      case MenuOption::RefreshCart:
        menuOptionCount = refreshCart(menuOptions, menuOptionMap, hasWitchOS);
        break;

      case MenuOption::WriteWitchOS:
        writeWitchOS();
        break;

      case MenuOption::Back: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::WonderSwan));
  }

  void cartOn()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    // A-1 - A6
    DDRF = 0xFF;
    // A7 - A14
    DDRK = 0xFF;
    // A15 - A22
    DDRL = 0xFF;

    // D0 - D15
    DDRC = 0x00;
    DDRA = 0x00;

    // controls
    DDRH |= ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
    PORTH |= ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));

    // CLK outputs LOW
    DDRE |= (1 << WS_CLK_BIT);
    PORTE &= ~(1 << WS_CLK_BIT);

    // IO? as input with internal pull-up enabled
    DDRE &= ~(1 << 4);
    PORTE |= (1 << 4);

    // INT as input with internal pull-up enabled
    DDRG &= ~(1 << 5);
    PORTG |= (1 << 5);

    // unlock MMC
    //  if (!unlockMMC2003())
    //    OSCR::UI::fatalError(F("Can't initial MMC"));

    do
    {
      unlockMMC2003();
    }
    while (!headerCheck());
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
    dataDir = DataDirection::Unknown;
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::WonderSwan));
    OSCR::Databases::Basic::setupMapper(F("ws-mappers"));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  bool headerCheck()
  {
    //dataIn();

    for (uint32_t i = 0; i < 16; i += 2)
    {
      * ((uint16_t *)(OSCR::Storage::Shared::buffer + i)) = readWord(0xFFFF0 + i);
    }

    //uint8_t startByte = OSCR::Storage::Shared::buffer[0];

    // Start should be 0xEA
    if (OSCR::Storage::Shared::buffer[0] != 0xEA) return false;

    //uint8_t zeroByte = OSCR::Storage::Shared::buffer[5];

    // Zero Byte
    if (OSCR::Storage::Shared::buffer[5] != 0) return false;

    //uint8_t systemByte = OSCR::Storage::Shared::buffer[7];

    // System < 2
    if (OSCR::Storage::Shared::buffer[7] > 1) return false;

    //uint8_t revisionByte = OSCR::Storage::Shared::buffer[9];

    // Known Revisions: 0 to 6 and 0x80
    if ((OSCR::Storage::Shared::buffer[9] > 6) && (OSCR::Storage::Shared::buffer[9] != 0x80)) return false;

    //uint8_t sizeByte = OSCR::Storage::Shared::buffer[10];

    // Rom Size < 10
    return (OSCR::Storage::Shared::buffer[10] < 10);
  }

  bool getCartInfo()
  {
    bool hasWitchOS = false;

    cartOn();

    //dataIn();

    //  for (uint32_t i = 0; i < 16; i += 2)
    //    *((uint16_t*)(OSCR::Storage::Shared::buffer + i)) = readWord(0xFFFF0 + i);

    checksum = *(uint16_t *)(OSCR::Storage::Shared::buffer + 14);

    // some game has wrong info in header
    // patch here
    switch (checksum)
    {
    // games with wrong save type/size
    // 256kbits sram
    case 0xE600:  // BAN007
    case 0x8EED:  // BANC16
    case 0xEE90:  // WIZC01
      OSCR::Storage::Shared::buffer[11] = 0x02;
      break;

    // games missing 'COLOR' flag
    case 0x26DB:  // SQRC01
    case 0xBFDF:  // SUMC07
    case 0x50CA:  // BANC09
    case 0x9238:  // BANC0E
    case 0x04F1:  // BANC1A
      OSCR::Storage::Shared::buffer[7] |= 0x01;
      break;

    case 0x7F73:  // BAN030
      // missing developerId and cartId
      OSCR::Storage::Shared::buffer[6] = 0x01;
      OSCR::Storage::Shared::buffer[8] = 0x30;
      break;

    case 0xEAFD:  //BANC33
      // enable GPIO and set to LOW
      //dataOut();

      writeBytePort(0xCC, 0x03);
      writeBytePort(0xCD, 0x00);
      break;

    case 0x0000:
      // developerId/cartId/checksum are all filled with 0x00 in witch based games
      //dataIn();

      if (readWord(0xF0000) == 0x4C45 && readWord(0xF0002) == 0x5349 && readWord(0xF0004) == 0x0041)
      {
        // check witch BIOS
        if (readWord(0xFFF5E) == 0x006C && readWord(0xFFF60) == 0x5B1B)
        {
          // check flashchip
          // should be a MBM29DL400TC
          //dataOut();
          writeWord(0x80AAA, 0xAAAA);
          writeWord(0x80555, 0x5555);
          writeWord(0x80AAA, 0x9090);
          //dataIn();

          if (readWord(0x80000) == 0x0004 && readWord(0x80002) == 0x220C)
          {
            hasWitchOS = true;
          }

          //dataOut();
          writeWord(0x80000, 0xF0F0);
          //dataIn();

          // 7AC003
          OSCR::Storage::Shared::buffer[6] = 0x7A;
          OSCR::Storage::Shared::buffer[8] = 0x03;
        }
        else if (readWord(0xFFF22) == 0x006C && readWord(0xFFF24) == 0x5B1B) // check service menu
        {
          if (readWord(0x93246) == 0x4A2F && readWord(0x93248) == 0x5353 && readWord(0x9324A) == 0x2E32)
          {
            // jss2
            OSCR::Storage::Shared::buffer[6] = 0xFF;   // WWGP
            OSCR::Storage::Shared::buffer[8] = 0x1A;   // 2001A
            OSCR::Storage::Shared::buffer[7] = 0x01;   // color only
            OSCR::Storage::Shared::buffer[10] = 0x04;  // size based on ROM chip capacity

            if (readWord(0x93E9C) == 0x4648 && readWord(0x93E9E) == 0x0050)
            {
              // WWGP2001A3 -> HFP Version
              OSCR::Storage::Shared::buffer[9] = 0x03;
              checksum = 0x4870;
            }
            else
            {
              // TODO check other jss2 version
            }
          }
          else if (readWord(0xE4260) == 0x6B64 && readWord(0xE4262) == 0x696E)
          {
            // dknight
            OSCR::Storage::Shared::buffer[6] = 0xFF;  // WWGP
            OSCR::Storage::Shared::buffer[8] = 0x2B;  // 2002B
            OSCR::Storage::Shared::buffer[7] = 0x01;  // color only
            OSCR::Storage::Shared::buffer[9] = 0x00;
            OSCR::Storage::Shared::buffer[10] = 0x04;  // size based on ROM chip (MR27V1602) capacity

            checksum = 0x8B1C;
          }
        }
      }
      else if (OSCR::Storage::Shared::buffer[6] == 0x2A && OSCR::Storage::Shared::buffer[8] == 0x01 && OSCR::Storage::Shared::buffer[9] == 0x01)
      {
        // Mobile WonderGate v1.1, checksum is filled with 0x0000
        checksum = 0x1DA0;
      }

      break;
    }

    romType = (OSCR::Storage::Shared::buffer[7] & 0x01); // wsc only = 1
    romVersion = OSCR::Storage::Shared::buffer[9];
    romSize = OSCR::Storage::Shared::buffer[10];
    sramSize = OSCR::Storage::Shared::buffer[11];

    snprintf_P(cartID, 5, PSTR("%c%02X"), (romType ? 'C' : '0'), OSCR::Storage::Shared::buffer[8]);

    if (OSCR::Databases::Basic::matchMapper(OSCR::Storage::Shared::buffer[6]))
    {
      snprintf_P(fileName, 17, PSTR("%s%s"), mapperDetail->name, cartID);
    }
    else
    {
      snprintf_P(fileName, 17, PSTR("%02X%s"), OSCR::Storage::Shared::buffer[6], cartID);
    }

    switch (romSize)
    {
    case 0x01:  cartSize = 131072 * 2;    break;
    case 0x02:  cartSize = 131072 * 4;    break;
    case 0x03:  cartSize = 131072 * 8;    break;
    case 0x04:  cartSize = 131072 * 16;   break;
    // case 0x05: cartSize = 131072 * 24; break;
    case 0x06:  cartSize = 131072 * 32;   break;
    // case 0x07: cartSize = 131072 * 48; break;
    case 0x08:  cartSize = 131072 * 64;   break;
    case 0x09:  cartSize = 131072 * 128;  break;
    default:    cartSize = 0;             break;
    }

    switch (sramSize)
    {
    case 0x00:
      saveType = 0;
      sramSize = 0;
      break;

    case 0x01:
      saveType = 1;
      sramSize = 64;
      break;

    case 0x02:
      saveType = 1;
      sramSize = 256;
      break;

    case 0x03:
      saveType = 1;
      sramSize = 1024;
      break;

    case 0x04:
      saveType = 1;
      sramSize = 2048;
      break;

    case 0x05:
      saveType = 1;
      sramSize = 4096;
      break;

    case 0x10:
      saveType = 2;
      sramSize = 1;
      break;

    case 0x20:
      saveType = 2;
      sramSize = 16;
      break;

    case 0x50:
      saveType = 2;
      sramSize = 8;
      break;

    default:
      saveType = 0xFF;
      break;
    }

    if (saveType == 2)
    {
      unprotectEEPROM();
    }

    cartOff();

    printHeader();

    OSCR::UI::printLine(fileName);

    if (cartSize == 0)
    {
      OSCR::UI::printHexLine(romSize);
    }
    else
    {
      OSCR::UI::printSize(OSCR::Strings::Common::ROM, cartSize);
    }

    char const * saveTypePSTR = OSCR::Strings::Common::Unknown;

    switch (saveType)
    {
    case 0:
      saveTypePSTR = OSCR::Strings::Common::None;
      break;

    case 1:
      saveTypePSTR = OSCR::Strings::Common::SRAM;
      break;

    case 2:
      saveTypePSTR = OSCR::Strings::Common::EEPROM;
      break;

    default:
      break;
    }

    OSCR::UI::printType_P(OSCR::Strings::Common::Save, saveTypePSTR);

    if (saveType > 0)
    {
      OSCR::UI::printSize(OSCR::Strings::Common::Save, sramSize * 1024);
    }

    OSCR::UI::printLabel(OSCR::Strings::Common::Revision);
    OSCR::UI::printHexLine(romVersion);

    OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);
    OSCR::UI::printHexLine(checksum);

    return hasWitchOS;
  }

  void readROM(bool const hasWitchOS)
  {
    uint16_t calcChecksum = 0;

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::WonderSwan), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    OSCR::UI::ProgressBar::init(cartSize);

    cartOn();

    // start reading rom
    for (uint16_t bank = (256 - (cartSize >> 16)); bank <= 0xFF; bank++)
    {
      // switch bank on segment 0x2
      //dataOut();
      writeBytePort(0xC2, bank);

      // blink LED on cartridge (only for BANC33)
      if (checksum == 0xEAFD)
      {
        writeBytePort(0xCD, (bank & 0x03));
      }

      //dataIn();

      for (uint32_t addr = 0; addr < 0x10000; addr += 512)
      {
        for (uint32_t w = 0; w < 512; w += 2)
        {
          *((uint16_t *)(OSCR::Storage::Shared::buffer + w)) = readWord(0x20000 + addr + w);

          // only calculate last two banks of checksum (os and bios region)
          if (hasWitchOS && bank < 0xFE)
            continue;

          // skip last two bytes of rom (checksum value)
          if (w == 510 && OSCR::UI::ProgressBar::current == cartSize - 512)
            continue;

          calcChecksum += OSCR::Storage::Shared::buffer[w];
          calcChecksum += OSCR::Storage::Shared::buffer[w + 1];
        }

        OSCR::Storage::Shared::writeBuffer();

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::ProgressBar::finish();

    // turn off LEDs for BANC33
    if (checksum == 0xEAFD)
    {
      //dataOut();
      writeBytePort(0xCD, 0x00);
    }

    OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);
    OSCR::UI::printHex(checksum);

    if (calcChecksum == checksum)
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::Arrow));
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

      OSCR::Databases::Basic::matchCRC();
    }
    else
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
      OSCR::UI::printHexLine(calcChecksum);

      OSCR::UI::error(FS(OSCR::Strings::Errors::IncorrectChecksum));
    }
  }

  void readSave()
  {
    switch (saveType)
    {
    case 0:
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::NoSave));
      return;

    case 1:
      readSRAM();
      return;

    case 2:
      readEEPROM();
      return;

    default:
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::UnknownType));
      return;
    }
  }

  void readSRAM()
  {
    uint32_t bank_size = (sramSize << 7);
    uint16_t end_bank = (bank_size >> 16);  // 64KB per bank
    uint16_t bank = 0;

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::WonderSwan), FS(OSCR::Strings::Directory::Save), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::Save));

    cartOn();

    if (bank_size > 0x10000)
    {
      bank_size = 0x10000;
    }

    do
    {
      //dataOut();
      writeBytePort(0xC1, bank);

      //dataIn();

      for (uint32_t addr = 0; addr < bank_size; addr += 512)
      {
        // SRAM data on D0-D7, with A-1 to select high/low byte
        for (uint32_t w = 0; w < 512; w++)
        {
          OSCR::Storage::Shared::buffer[w] = readByte(0x10000 + addr + w);
        }

        OSCR::Storage::Shared::writeBuffer();
      }
    }
    while (++bank < end_bank);

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void writeSave()
  {
    switch (saveType)
    {
    case 0:
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::NoSave));
      return;

    case 1:
      writeSRAM();
      return;

    case 2:
      writeEEPROM();
      return;

    default:
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::UnknownType));
      return;
    }
  }

  void writeSRAM()
  {
    uint32_t bank_size = (sramSize << 7);
    uint16_t end_bank = (bank_size >> 16);  // 64KB per bank
    uint16_t bank = 0;

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    if (bank_size > 0x10000)
      bank_size = 0x10000;

    //dataOut();

    do
    {
      writeBytePort(0xC1, bank);

      for (uint32_t addr = 0; addr < bank_size && OSCR::Storage::Shared::available(); addr += 512)
      {
        OSCR::Storage::Shared::fill();

        // SRAM data on D0-D7, with A-1 to select high/low byte
        for (uint32_t w = 0; w < 512; w++)
        {
          writeByte(0x10000 + addr + w, OSCR::Storage::Shared::buffer[w]);
        }
      }
    }
    while (++bank < end_bank);

    OSCR::Storage::Shared::rewind();

    printHeader();
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    bank = 0;
    writeErrors = 0;

    do
    {
      //dataOut();
      writeBytePort(0xC1, bank);
      //dataIn();

      for (uint32_t addr = 0; addr < bank_size && OSCR::Storage::Shared::available(); addr += 512)
      {
        OSCR::Storage::Shared::fill();

        // SRAM data on D0-D7, with A-1 to select high/low byte
        for (uint32_t w = 0; w < 512; w++)
        {
          if (readByte(0x10000 + addr + w) != OSCR::Storage::Shared::buffer[w])
          {
            writeErrors++;
          }
        }
      }
    }
    while (++bank < end_bank);

    OSCR::Storage::Shared::close();

    if (writeErrors == 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }
  }

  void readEEPROM()
  {
    uint8_t wsEepromShiftReg[2];
    uint32_t eepromSize = (sramSize << 7);
    uint32_t bufSize = (eepromSize < 512 ? eepromSize : 512);

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::WonderSwan), FS(OSCR::Strings::Directory::Save), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::SaveEEPROM));

    cartOn();

    for (uint32_t i = 0; i < eepromSize; i += bufSize)
    {
      for (uint32_t j = 0; j < bufSize; j += 2)
      {
        generateEepromInstruction(wsEepromShiftReg, 0x2, ((i + j) >> 1));

        //dataOut();

        writeBytePort(0xC6, wsEepromShiftReg[0]);
        writeBytePort(0xC7, wsEepromShiftReg[1]);
        writeBytePort(0xC8, 0x10);

        // MMC will shift out from port 0xC7 to 0xC6
        // and shift in 16bits into port 0xC5 to 0xC4
        pulseCLK(1 + 32 + 3);

        //dataIn();

        OSCR::Storage::Shared::buffer[j] = readBytePort(0xC4);
        OSCR::Storage::Shared::buffer[j + 1] = readBytePort(0xC5);
      }

      OSCR::Storage::Shared::writeBuffer(bufSize);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void writeEEPROM()
  {
    uint8_t wsEepromShiftReg[2];
    uint32_t const eepromSize = (sramSize << 7);
    uint32_t const bufSize = (eepromSize < 512 ? eepromSize : 512);

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    for (uint32_t i = 0; i < eepromSize; i += bufSize)
    {
      OSCR::Storage::Shared::readBuffer(bufSize);

      for (uint32_t j = 0; j < bufSize; j += 2)
      {
        generateEepromInstruction(wsEepromShiftReg, 0x1, ((i + j) >> 1));

        //dataOut();
        writeBytePort(0xC6, wsEepromShiftReg[0]);
        writeBytePort(0xC7, wsEepromShiftReg[1]);
        writeBytePort(0xC4, OSCR::Storage::Shared::buffer[j]);
        writeBytePort(0xC5, OSCR::Storage::Shared::buffer[j + 1]);
        writeBytePort(0xC8, 0x20);

        // MMC will shift out from port 0xC7 to 0xC4
        pulseCLK(1 + 32 + 3);

        //dataIn();

        do
        {
          pulseCLK(128);
        }
        while ((readBytePort(0xC8) & 0x02) == 0x00);
      }
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    OSCR::Storage::Shared::rewind();

    writeErrors = 0;

    for (uint32_t i = 0; i < eepromSize; i += bufSize)
    {
      OSCR::Storage::Shared::readBuffer(bufSize);

      for (uint32_t j = 0; j < bufSize; j += 2)
      {
        generateEepromInstruction(wsEepromShiftReg, 0x2, ((i + j) >> 1));

        //dataOut();
        writeBytePort(0xC6, wsEepromShiftReg[0]);
        writeBytePort(0xC7, wsEepromShiftReg[1]);
        writeBytePort(0xC8, 0x10);

        // MMC will shift out from port 0xC7 to 0xC6
        // and shift in 16bits into port 0xC5 to 0xC4
        pulseCLK(1 + 32 + 3);

        //dataIn();
        if (readBytePort(0xC4) != OSCR::Storage::Shared::buffer[j])
          writeErrors++;

        if (readBytePort(0xC5) != OSCR::Storage::Shared::buffer[j + 1])
          writeErrors++;
      }
    }

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

  // make sure that OS sectors not protected
  bool canWriteWitchOS()
  {
    bool writeProtected = false;

    cartOn();

    //dataOut();

    writeWord(0x80AAA, 0xAAAA);
    writeWord(0x80555, 0x5555);
    writeWord(0xE0AAA, 0x9090);

    //dataIn();

    writeProtected = (readWord(0xE0004) || readWord(0xE4004) || readWord(0xEC004) || readWord(0xEE004));

    cartOff();

    return !writeProtected;
  }

  void writeWitchOS()
  {
    uint32_t fbin_length;

    if (!canWriteWitchOS())
    {
      printHeader();
      OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
      return;
    }

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    fbin_length = OSCR::Storage::Shared::getSize();

    printHeader();
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    cartOn();

    eraseWitchFlashSector(0xE0000);
    eraseWitchFlashSector(0xE4000);
    eraseWitchFlashSector(0xEC000);
    eraseWitchFlashSector(0xEE000);

    printHeader();
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    // OS size seems limit to 64KBytes
    // last 16 bytes contains jmpf code and block count (written by BIOS)
    if (fbin_length > 65520)
    {
      fbin_length = 65520;
    }

    // enter fast program mode
    //dataOut();
    writeWord(0x80AAA, 0xAAAA);
    writeWord(0x80555, 0x5555);
    writeWord(0x80AAA, 0x2020);

    // 128bytes per block
    for (uint32_t i = 0; i < fbin_length; i += 128)
    {
      uint8_t key = 0xFF;
      uint16_t bytesRead = OSCR::Storage::Shared::readBuffer(128);

      for (uint16_t j = 0; j < bytesRead; j += 2)
      {
        // for each decoded[n] = encoded[n] ^ key
        // where key = encoded[n - 1]
        // key = 0xFF when n = 0, 0 <= n < 128
        uint16_t pd = ((OSCR::Storage::Shared::buffer[j] ^ key) | ((OSCR::Storage::Shared::buffer[j + 1] ^ OSCR::Storage::Shared::buffer[j]) << 8));

        key = OSCR::Storage::Shared::buffer[j + 1];

        fastProgramWitchFlash(0xE0000 + i + j, pd);
      }
    }

    // write jmpf instruction and block counts at 0xE0000
    memcpy_P(OSCR::Storage::Shared::buffer, wwLaunchCode, 8);
    *((uint16_t *)(OSCR::Storage::Shared::buffer + 6)) = ((fbin_length >> 7) & 0xFFFF);

    for (uint8_t i = 0; i < 8; i += 2)
    {
      fastProgramWitchFlash(0xEFFF0 + i, *((uint16_t *)(OSCR::Storage::Shared::buffer + i)));
    }

    // leave fast program mode
    //dataOut();

    writeWord(0xE0000, 0x9090);
    writeWord(0xE0000, 0xF0F0);
    writeWord(0x80000, 0xF0F0);

    cartOff();

    OSCR::Storage::Shared::close();
  }

  void fastProgramWitchFlash(uint32_t addr, uint16_t data)
  {
    //dataOut();

    writeWord(addr, 0xA0A0);
    writeWord(addr, data);

    //dataIn();
    while (readWord(addr) != data);
  }

  void eraseWitchFlashSector(uint32_t sector_addr)
  {
    //dataOut();

    writeWord(0x80AAA, 0xAAAA);
    writeWord(0x80555, 0x5555);
    writeWord(0x80AAA, 0x8080);
    writeWord(0x80AAA, 0xAAAA);
    writeWord(0x80555, 0x5555);
    writeWord(sector_addr, 0x3030);

    //dataIn();

    while ((readWord(sector_addr) & 0x0080) == 0x0000);
  }

  void writeBytePort(uint8_t port, uint8_t data)
  {
    dataOut();

    PORTF = (port & 0x0F);
    PORTL = (port >> 4);

    // switch CART(PH3), MMC(PH4) to LOW
    PORTH &= ~((1 << 3) | (1 << 4));

    // set data
    PORTC = data;

    // switch WE(PH5) to LOW
    PORTH &= ~(1 << 5);
    NOP;

    // switch WE(PH5) to HIGH
    PORTH |= (1 << 5);
    NOP;
    NOP;

    // switch CART(PH3), MMC(PH4) to HIGH
    PORTH |= ((1 << 3) | (1 << 4));
  }

  uint8_t readBytePort(uint8_t port)
  {
    dataIn();

    PORTF = (port & 0x0F);
    PORTL = (port >> 4);

    // switch CART(PH3), MMC(PH4) to LOW
    PORTH &= ~((1 << 3) | (1 << 4));

    // switch OE(PH6) to LOW
    PORTH &= ~(1 << 6);
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

    // switch OE(PH6) to HIGH
    PORTH |= (1 << 6);

    // switch CART(PH3), MMC(PH4) to HIGH
    PORTH |= ((1 << 3) | (1 << 4));

    return ret;
  }

  void writeWord(uint32_t addr, uint16_t data)
  {
    dataOut();

    PORTF = addr & 0xFF;
    PORTK = (addr >> 8) & 0xFF;
    PORTL = (addr >> 16) & 0x0F;

    PORTC = data & 0xFF;
    PORTA = (data >> 8);

    // switch CART(PH3) and WE(PH5) to LOW
    PORTH &= ~((1 << 3) | (1 << 5));
    NOP;

    // switch CART(PH3) and WE(PH5) to HIGH
    PORTH |= (1 << 3) | (1 << 5);
    NOP;
    NOP;
  }

  uint16_t readWord(uint32_t addr)
  {
    dataIn();

    PORTF = addr & 0xFF;
    PORTK = (addr >> 8) & 0xFF;
    PORTL = (addr >> 16) & 0x0F;

    // switch CART(PH3) and OE(PH6) to LOW
    PORTH &= ~((1 << 3) | (1 << 6));
    NOP;
    NOP;
    NOP;

    uint16_t ret = ((PINA << 8) | PINC);

    // switch CART(PH3) and OE(PH6) to HIGH
    PORTH |= (1 << 3) | (1 << 6);

    return ret;
  }

  void writeByte(uint32_t addr, uint8_t data)
  {
    dataOut();

    PORTF = addr & 0xFF;
    PORTK = (addr >> 8) & 0xFF;
    PORTL = (addr >> 16) & 0x0F;

    PORTC = data;

    // switch CART(PH3) and WE(PH5) to LOW
    PORTH &= ~((1 << 3) | (1 << 5));
    NOP;

    // switch CART(PH3) and WE(PH5) to HIGH
    PORTH |= (1 << 3) | (1 << 5);
    NOP;
    NOP;
  }

  uint8_t readByte(uint32_t addr)
  {
    dataIn();

    PORTF = addr & 0xFF;
    PORTK = (addr >> 8) & 0xFF;
    PORTL = (addr >> 16) & 0x0F;

    // switch CART(PH3) and OE(PH6) to LOW
    PORTH &= ~((1 << 3) | (1 << 6));
    NOP;
    NOP;
    NOP;

    uint8_t ret = PINC;

    // switch CART(PH3) and OE(PH6) to HIGH
    PORTH |= (1 << 3) | (1 << 6);

    return ret;
  }

  void unprotectEEPROM()
  {
    uint8_t wsEepromShiftReg[2];

    generateEepromInstruction(wsEepromShiftReg, 0x0, 0x3);

    //dataOut();

    writeBytePort(0xC6, wsEepromShiftReg[0]);
    writeBytePort(0xC7, wsEepromShiftReg[1]);
    writeBytePort(0xC8, 0x40);

    // MMC will shift out port 0xC7 to 0xC6 to EEPROM
    pulseCLK(1 + 16 + 3);
  }

  // generate data for port 0xC6 to 0xC7
  // number of CLK pulses needed for each instruction is 1 + (16 or 32) + 3
  void generateEepromInstruction(uint8_t *instruction, uint8_t opcode, uint16_t addr)
  {
    uint8_t addr_bits = (sramSize > 1 ? 10 : 6);
    uint16_t *ptr = (uint16_t *)instruction;
    *ptr = 0x0001;  // initial with a start bit

    if (opcode == 0)
    {
      // 2bits opcode = 0x00
      *ptr <<= 2;
      // 2bits ext cmd (from addr)
      *ptr <<= 2;
      *ptr |= (addr & 0x0003);
      *ptr <<= (addr_bits - 2);
    }
    else
    {
      // 2bits opcode
      *ptr <<= 2;
      *ptr |= (opcode & 0x03);
      // address bits
      *ptr <<= addr_bits;
      *ptr |= (addr & ((1 << addr_bits) - 1));
    }
  }

  // 2003 MMC need to be unlock,
  // or it will reject all reading and bank switching
  // All signals' timing are analyzed by using LogicAnalyzer
  bool unlockMMC2003()
  {
    // initialize all control pin state
    // RST(PH0) and CLK(PE3or5) to LOW
    // CART(PH3) MMC(PH4) WE(PH5) OE(PH6) to HIGH
    PORTH &= ~(1 << 0);
    PORTE &= ~(1 << WS_CLK_BIT);
    PORTH |= ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));

    // switch RST(PH0) to HIGH
    PORTH |= (1 << 0);

    PORTF = 0x0A;
    PORTL = 0x05;
    pulseCLK(3);

    PORTF = 0x05;
    PORTL = 0x0A;
    pulseCLK(4);

    // MMC is outputing something on IO? pin synchronized with CLK
    // so still need to pulse CLK until MMC is ok to work
    pulseCLK(18);

    // unlock procedure finished
    // see if we can set bank number to MMC
    //dataOut();
    writeBytePort(0xC2, 0xAA);
    writeBytePort(0xC3, 0x55);

    //dataIn();

    if (readBytePort(0xC2) == 0xAA && readBytePort(0xC3) == 0x55)
    {
      // now set initial bank number to MMC
      //dataOut();

      writeBytePort(0xC0, 0x2F);
      writeBytePort(0xC1, 0x3F);
      writeBytePort(0xC2, 0xFF);
      writeBytePort(0xC3, 0xFF);

      return true;
    }

    return false;
  }

  // doing a L->H on CLK pin
  void pulseCLK(uint8_t count)
  {
    uint8_t tic;

    // about 384KHz, 50% duty cycle
    asm volatile("L0_%=:\n\t"
                "cpi %[count], 0\n\t"
                "breq L3_%=\n\t"
                "dec %[count]\n\t"
                "cbi %[porte], %[ws_clk_bit]\n\t"
                "ldi %[tic], 6\n\t"
                "L1_%=:\n\t"
                "dec %[tic]\n\t"
                "brne L1_%=\n\t"
                "sbi %[porte], %[ws_clk_bit]\n\t"
                "ldi %[tic], 5\n\t"
                "L2_%=:\n\t"
                "dec %[tic]\n\t"
                "brne L2_%=\n\t"
                "rjmp L0_%=\n\t"
                "L3_%=:\n\t"
                : [tic] "=a"(tic)
                : [count] "a"(count), [porte] "I"(_SFR_IO_ADDR(PORTE)), [ws_clk_bit] "I"(WS_CLK_BIT));
  }

  void dataIn()
  {
    DDRC = 0x00;
    DDRA = 0x00;

    // some game's ROM chip needs internal-pullup be disabled to work properly
    // ex: Mobile Suit Gundam Vol.2 - JABURO (MX23L6410MC-12 Mask ROM)
    PORTC = 0x00;
    PORTA = 0x00;

    dataDir = DataDirection::In;
  }

  void dataOut()
  {
    DDRC = 0xFF;
    DDRA = 0xFF;

    dataDir = DataDirection::Out;
  }
} /* namespace OSCR::Cores::WonderSwan */

#endif /* HAS_WS */
