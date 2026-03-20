//******************************************
// GAME BOY ADVANCE MODULE
//******************************************
#include "config.h"

#if HAS_GBX
# include "cores/include.h"
# include "cores/GameBoy.h"
# include "cores/GameBoyAdvance.h"
# include "cores/Flash.h"

namespace OSCR::Cores::GameBoyAdvance
{
  using OSCR::Cores::Flash::flashid;

  /******************************************
     Menu
  *****************************************/
  // GBA menu items
  enum class MenuOption : uint8_t
  {
    ReadROM,
    ReadSave,
    WriteSave,
    SetSaveType,
    RefreshCart,
    Back,
  };

  constexpr uint32_t const saveRtcData    = 0xC4;
  constexpr uint32_t const saveRtcRW      = 0xC6;
  constexpr uint32_t const saveRtcEnable  = 0xC8;

# if HAS_FLASH
  // 369-in-1 menu items
  constexpr char const PROGMEM Menu369Item1[] = "Read 256MB";
  constexpr char const PROGMEM Menu369Item2[] = "Write 256MB";
  constexpr char const PROGMEM Menu369Item3[] = "Read Offset";
  constexpr char const PROGMEM Menu369Item4[] = "Write Offset";
  constexpr char const * const PROGMEM Options369GBA[] = {
    Menu369Item1,
    Menu369Item2,
    Menu369Item3,
    Menu369Item4,
    OSCR::Strings::MenuOptions::Back,
  };
# endif

  // Save menu
  constexpr char const PROGMEM GBASaveItem1[] = "4K EEPROM";
  constexpr char const PROGMEM GBASaveItem2[] = "64K EEPROM";
  constexpr char const PROGMEM GBASaveItem3[] = "256K SRAM/FRAM";
  constexpr char const PROGMEM GBASaveItem4[] = "512K SRAM/FRAM";
  constexpr char const PROGMEM GBASaveItem5[] = "512K FLASH";
  constexpr char const PROGMEM GBASaveItem6[] = "1M FLASH";

  constexpr char const * const PROGMEM saveOptionsGBA[] = {
    OSCR::Strings::Common::None,
    GBASaveItem1,
    GBASaveItem2,
    GBASaveItem3,
    GBASaveItem4,
    GBASaveItem5,
    GBASaveItem6,
  };

  crdbGBARecord * romDetail;
  OSCR::Databases::GBARecord * romRecord = nullptr;

  uint8_t refreshCart(__FlashStringHelper const * menuOptions[], MenuOption * const menuOptionMap)
  {
    uint8_t option = 0;

    if (!getCartInfo())
    {
      // Set the menu options to only be "Back" so we return to the main menu.
      menuOptions[option] = FS(OSCR::Strings::MenuOptions::Back);
      menuOptionMap[option] = MenuOption::Back;
      return 1;
    }

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadROM);
    menuOptionMap[option] = MenuOption::ReadROM;
    option++;

    if (saveType)
    {
      menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadSave);
      menuOptionMap[option] = MenuOption::ReadSave;
      option++;

      menuOptions[option] = FS(OSCR::Strings::MenuOptions::WriteSave);
      menuOptionMap[option] = MenuOption::WriteSave;
      option++;
    }

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::SetSaveType);
    menuOptionMap[option] = MenuOption::SetSaveType;
    option++;

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
    __FlashStringHelper const * menuOptions[kMenuOptionMax] = {};
    MenuOption menuOptionMap[kMenuOptionMax] = {};
    uint8_t menuOptionCount;

    openCRDB();

    menuOptionCount = refreshCart(menuOptions, menuOptionMap);

    do
    {
      // If there is only one option, automatically select it. Otherwise, create a menu as normal.
      uint8_t selected = (menuOptionCount == 1) ? 0 : OSCR::UI::menu(FS(OSCR::Strings::Cores::GameBoyAdvance), menuOptions, menuOptionCount);

      switch (menuOptionMap[selected])
      {
      case MenuOption::ReadROM:
        readROM();
        break;

      case MenuOption::ReadSave:
        readSave();
        break;

      case MenuOption::WriteSave:
        writeSave();
        break;

      case MenuOption::SetSaveType:
        configureSave();
        break;

      case MenuOption::RefreshCart:
        menuOptionCount = refreshCart(menuOptions, menuOptionMap);
        continue;

      case MenuOption::Back:
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void rtcTest()
  {
    uint32_t rtcData = 0;

    cartOn();

    writeByte(0x0000, 0x0B);
    writeByte(0xA000, 0x60);
    writeByte(0x0000, 0x0D);
    writeByte(0xA000, 0xFE);

    _delay_us(1);

    writeByte(0x0000, 0x0C);
    writeByte(0x0000, 0x00);
    writeByte(0x0000, 0x0B);
    writeByte(0xA000, 0x40);
    writeByte(0x0000, 0x0D);
    writeByte(0xA000, 0xFE);

    _delay_us(1);

    delay(20);

    writeByte(0x0000, 0x0C);
    writeByte(0x0000, 0x00);

    for (uint8_t i = 0; i < 7; i++)
    {
      writeByte(0x0000, 0x0B);
      writeByte(0xA000, 0x10);
      writeByte(0x0000, 0x0D);
      writeByte(0xA000, 0xFE);

      _delay_us(1);

      writeByte(0x0000, 0x0C);

      rtcData |= (readByte(0xA000) & 0x0F) << (i * 4);
      writeByte(0x0000, 0x00);
    }

    cartOff();
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::GameBoyAdvance));
  }

# if HAS_FLASH
  // Flash GBA Repro
  void reproMenu()
  {
    cartOn();
    flashRepro(0);
  }

  // Read/Write GBA 369-in-1 Repro
  void repro369in1Menu()
  {
    if (!OSCR::Prompts::confirmErase()) return;

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Cores::GameBoyX::GBFlashItem6), Options369GBA, sizeofarray(Options369GBA)))
      {
      case 0:
        read369in1(0, 0);
        break;

      case 1:
        flashRepro(0);
        break;

      case 2:
        read369in1(selectBlockNumber(1), selectBlockNumber(0));
        break;

      case 3:
        flashRepro(1);
        break;

      case 4:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }
# endif /* HAS_FLASH */

  /******************************************
     Setup
  *****************************************/
  void cartOn()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    setROM();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    cartCRDB = new OSCR::Databases::GBA(FS(OSCR::Strings::FileType::GameBoyAdvance));
  }

  void closeCRDB()
  {
    if (cartCRDB == nullptr) return;
    delete gbaCRDB;
    resetGlobals();
  }

  /******************************************
     Low level functions
  *****************************************/
  uint8_t getSaveType()
  {
    if (saveType == 0)
    {
      constexpr uint8_t const saveOptionsGBAType[] = { 0, 1, 2, 3, 6, 4, 5 };
      uint8_t selection = OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetCartType), saveOptionsGBA, sizeofarray(saveOptionsGBA));
      saveType = saveOptionsGBAType[selection];
    }
    return saveType;
  }

  void printFlashTypeAndWait(char const * caption)
  {
    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    OSCR::UI::printHexLine(flashid);

    OSCR::UI::printType_P(OSCR::Strings::Common::Flash, caption);

    OSCR::UI::waitButton();
  }

  void setROM()
  {
    // CS_SRAM(PH0)
    DDRH |= (1 << 0);
    PORTH |= (1 << 0);
    // CS_ROM(PH3)
    DDRH |= (1 << 3);
    PORTH |= (1 << 3);
    // WR(PH5)
    DDRH |= (1 << 5);
    PORTH |= (1 << 5);
    // RD(PH6)
    DDRH |= (1 << 6);
    PORTH |= (1 << 6);
    // AD0-AD7
    DDRF = 0xFF;
    // AD8-AD15
    DDRK = 0xFF;
    // AD16-AD23
    DDRC = 0xFF;
    // Wait
    delay(500);
  }

  uint16_t readWord(uint32_t myAddress)
  {
    // Set address/data ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;
    DDRC = 0xFF;

    // Divide address by two to get word addressing
    myAddress = myAddress >> 1;

    // Output address to address pins,
    PORTF = myAddress;
    PORTK = myAddress >> 8;
    PORTC = myAddress >> 16;

    // Pull CS(PH3) to LOW
    PORTH &= ~(1 << 3);

    // Set address/data ports to input
    PORTF = 0x0;
    PORTK = 0x0;
    DDRF = 0x0;
    DDRK = 0x0;

    // Pull RD(PH6) to LOW
    PORTH &= ~(1 << 6);

    // Delay here or read error with repro
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    uint16_t myWord = (PINK << 8) | PINF;

    // Switch RD(PH6) to HIGH
    PORTH |= (1 << 6);

    // Switch CS_ROM(PH3) to HIGH
    PORTH |= (1 << 3);

    return myWord;
  }

  void writeWord(uint32_t myAddress, uint16_t myWord)
  {
    // Set address/data ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;
    DDRC = 0xFF;

    // Divide address by two to get word addressing
    myAddress = myAddress >> 1;

    // Output address to address pins,
    PORTF = myAddress;
    PORTK = myAddress >> 8;
    PORTC = myAddress >> 16;

    // Pull CS(PH3) to LOW
    PORTH &= ~(1 << 3);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Output data
    PORTF = myWord & 0xFF;
    PORTK = myWord >> 8;

    // Pull WR(PH5) to LOW
    PORTH &= ~(1 << 5);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Switch WR(PH5) to HIGH
    PORTH |= (1 << 5);

    // Switch CS_ROM(PH3) to HIGH
    PORTH |= (1 << 3);
  }

  // This function swaps bit at positions p1 and p2 in an integer n
  uint16_t swapBits(uint16_t n, uint16_t p1, uint16_t p2)
  {
    // Move p1'th to rightmost side
    uint16_t bit1 = (n >> p1) & 1;

    // Move p2'th to rightmost side
    uint16_t bit2 = (n >> p2) & 1;

    // XOR the two bits */
    uint16_t x = (bit1 ^ bit2);

    // Put the xor bit back to their original positions
    x = (x << p1) | (x << p2);

    // XOR 'x' with the original number so that the two sets are swapped
    uint16_t result = n ^ x;

    return result;
  }

  // Some repros have D0 and D1 switched
  uint16_t readWord_GAB(uint32_t myAddress)
  {
    uint16_t tempWord = swapBits(readWord(myAddress), 0, 1);
    return tempWord;
  }

  void writeWord_GAB(uint32_t myAddress, uint16_t myWord)
  {
    writeWord(myAddress, swapBits(myWord, 0, 1));
  }

  uint8_t readByte(uint16_t myAddress)
  {
    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;
    // Set data port to input
    DDRC = 0x0;

    // Output address to address pins,
    PORTF = myAddress;
    PORTK = myAddress >> 8;

    // Pull OE_SRAM(PH6) to LOW
    PORTH &= ~(1 << 6);
    // Pull CE_SRAM(PH0) to LOW
    PORTH &= ~(1 << 0);

    // Hold address for at least 25ns and wait 150ns before access
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read byte
    uint8_t tempByte = PINC;

    // Pull CE_SRAM(PH0) HIGH
    PORTH |= (1 << 0);
    // Pull OE_SRAM(PH6) HIGH
    PORTH |= (1 << 6);

    return tempByte;
  }

  void writeByte(uint16_t myAddress, uint8_t myData)
  {
    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;
    // Set data port to output
    DDRC = 0xFF;

    // Output address to address pins
    PORTF = myAddress;
    PORTK = myAddress >> 8;
    // Output data to data pins
    PORTC = myData;

    // Wait till output is stable
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Pull WE_SRAM(PH5) to LOW
    PORTH &= ~(1 << 5);
    // Pull CE_SRAM(PH0) to LOW
    PORTH &= ~(1 << 0);

    // Leave WR low for at least 60ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Pull CE_SRAM(PH0) HIGH
    PORTH |= (1 << 0);
    // Pull WE_SRAM(PH5) HIGH
    PORTH |= (1 << 5);

    // Leave WR high for at least 50ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );
  }

  /******************************************
    GBA ROM Functions
  *****************************************/
  // Compute the checksum of rom header
  // "header" must contain at least the rom's first 188 bytes
  uint8_t checksumHeader(uint8_t const * header)
  {
    uint8_t result = 0x00;

    for (uint8_t n = 0xA0; n < 0xBD; n++)
    {
      result -= header[n];
    }

    return result - 0x19;
  }

  // Read info out of rom header
  bool getCartInfo()
  {
    uint16_t logoChecksum;

    // Compare Nintendo logo against known checksum, 156 bytes starting at 0x04
    do
    {
      cartOn();

      // Read Header into array
      for (int currWord = 0; currWord < 192; currWord += 2)
      {
        uint16_t tempWord = readWord(currWord);

        OSCR::Storage::Shared::buffer[currWord] = tempWord & 0xFF;
        OSCR::Storage::Shared::buffer[currWord + 1] = (tempWord >> 8) & 0xFF;
      }

      cartOff();

      logoChecksum = 0;

      for (int currByte = 0x4; currByte < 0xA0; currByte++)
      {
        logoChecksum += OSCR::Storage::Shared::buffer[currByte];
      }

      if (logoChecksum == 0x4B1B) break;

      switch(OSCR::Prompts::abortRetryContinue())
      {
      case OSCR::Prompts::AbortRetryContinue::Abort:    return false;

      case OSCR::Prompts::AbortRetryContinue::Retry:    continue; // repeat the loop
      case OSCR::Prompts::AbortRetryContinue::Continue: break;    // exit the switch and then exit the loop
      }

      useDefaultName();

      break; // Only reached when continue is selected above.
    }
    while (true);

    // Get cart ID
    cartID[0] = char(OSCR::Storage::Shared::buffer[0xAC]);
    cartID[1] = char(OSCR::Storage::Shared::buffer[0xAD]);
    cartID[2] = char(OSCR::Storage::Shared::buffer[0xAE]);
    cartID[3] = char(OSCR::Storage::Shared::buffer[0xAF]);

    bool searching = true;
    bool found = false;
    bool reverse = false;

    uint_fast16_t const indexMax = gbaCRDB->numRecords() - 1;
    uint_fast16_t firstIndex = 0; // The first index that was found
    uint_fast16_t previousIndex = 0; // The previous index that was found
    uint_fast16_t currentIndex = 0; // The current index

    // Loop through file
    while (searching)
    {
      printHeader();
      OSCR::UI::printSync(FS(OSCR::Strings::Status::SearchingDatabase));

      if (!gbaCRDB->searchRecord(cartID, currentIndex, reverse))
      {
        if ((!found) || (currentIndex == previousIndex))
        {
          configureCart();
          return true;
        }

        currentIndex = previousIndex;
        continue;
      }

      OSCR::UI::clearLine(); // Clear "Searching Database..."

      currentIndex = gbaCRDB->getRecordIndex();
      romRecord = gbaCRDB->record();
      romDetail = romRecord->data();

#   if CRDB_DEBUGGING
      OSCR::Serial::printSync(F("Found ROM: "));
      OSCR::Serial::printLineSync(romDetail->name);

      romRecord->debug();
#   endif /* CRDB_DEBUGGING */

      // Print current database entry
      OSCR::UI::printLine(romDetail->name);
      OSCR::UI::printValue(OSCR::Strings::Common::ID, romDetail->serial);
      OSCR::UI::printSize(OSCR::Strings::Common::ROM, ((uint32_t)romDetail->size) * 1024 * 1024);
      OSCR::UI::printType(OSCR::Strings::Common::Save, romDetail->saveType);

#   if HAS_OUTPUT_LINE_ALIGNMENT
      if (found)
      {
        if (currentIndex == previousIndex)
        {
          OSCR::UI::printCenter_P(OSCR::Strings::Errors::OnlyMatchingRecord);
        }
        else if (currentIndex == firstIndex)
        {
          OSCR::UI::printCenter_P(OSCR::Strings::Errors::BackAtFirst);
        }
      }

      OSCR::UI::gotoLast();

      char tmpText[14];

      snprintf_P(tmpText, 14, OSCR::Strings::Templates::WordsPP, OSCR::Strings::Symbol::ArrowLeft, OSCR::Strings::MenuOptions::Previous);

      OSCR::UI::print(tmpText);

      snprintf_P(tmpText, 14, OSCR::Strings::Templates::WordsPP, OSCR::Strings::MenuOptions::Next, OSCR::Strings::Symbol::Arrow);

      OSCR::UI::printRight(tmpText);
#   else
      if (found)
      {
        if (currentIndex == previousIndex)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Errors::OnlyMatchingRecord));
        }
        else if (currentIndex == firstIndex)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Errors::BackAtFirst));
        }
      }

      OSCR::UI::gotoLast();

      char tmpText[32];
      snprintf_P(tmpText, 32, OSCR::Strings::Templates::WordsPPPPP, OSCR::Strings::Symbol::ArrowLeft, OSCR::Strings::MenuOptions::Previous, OSCR::Strings::Symbol::MenuSpaces, OSCR::Strings::MenuOptions::Next, OSCR::Strings::Symbol::Arrow);

      OSCR::UI::print(tmpText);
#   endif

      if (!found)
      {
        found = true;
        firstIndex = currentIndex;
      }

      previousIndex = currentIndex;

      switch (OSCR::UI::waitInput())
      {
      case OSCR::UI::UserInput::kUserInputConfirm: // Confirmed
      case OSCR::UI::UserInput::kUserInputConfirmShort:
      case OSCR::UI::UserInput::kUserInputConfirmLong:
        searching = false; // Exit the loop
        continue;

      case OSCR::UI::UserInput::kUserInputNext: // Read next entry
        reverse = false;

        if (currentIndex == indexMax)
        {
          currentIndex = 0;
        }
        else
        {
          currentIndex++;
        }
        continue;

      case OSCR::UI::UserInput::kUserInputBack: // Read previous entry
        reverse = true;

        if (currentIndex == 0)
        {
          currentIndex = indexMax;
        }
        else
        {
          currentIndex--;
        }
        continue;

      case OSCR::UI::UserInput::kUserInputUnknown: // Should not happen.
      case OSCR::UI::UserInput::kUserInputIgnore:
        OSCR::Util::unreachable();
      }
    }

    cartSize = romDetail->size * 0x100000;

    setOutName((char *)&OSCR::Storage::Shared::buffer[0xA0], 12);

    // Get ROM version
    romVersion = OSCR::Storage::Shared::buffer[0xBC];

    // Calculate Checksum
    checksum = checksumHeader(OSCR::Storage::Shared::buffer);

    // Compare checksum
    if (OSCR::Storage::Shared::buffer[0xBD] != checksum)
    {
      printHeader();

      OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);
      OSCR::UI::printHexLine(checksum);
      OSCR::UI::error(FS(OSCR::Strings::Errors::IncorrectChecksum));
    }

    /*
      Save types in ROM
      EEPROM_Vnnn    EEPROM 512 bytes or 8 Kbytes (4Kbit or 64Kbit)
      SRAM_Vnnn      SRAM 32 Kbytes (256Kbit)
      SRAM_F_Vnnn    FRAM 32 Kbytes (256Kbit)
      FLASH_Vnnn     FLASH 64 Kbytes (512Kbit) (ID used in older files)
      FLASH512_Vnnn  FLASH 64 Kbytes (512Kbit) (ID used in newer files)
      FLASH1M_Vnnn   FLASH 128 Kbytes (1Mbit)

      Save types in Cart Reader Code
      0 = Unknown or no save
      1 = 4K EEPROM
      2 = 64K EEPROM
      3 = 256K SRAM
      4 = 512K FLASH
      5 = 1M FLASH
      6 = 512K SRAM
    */

    saveType = romDetail->saveType;

    if (saveType == 1)
    {
      // Test if 4kbit or 64kbit EEPROM
      cartOn();

      // Disable interrupts for more uniform clock pulses
      noInterrupts();

      readBlock(0, 64);

      interrupts();

      delay(1000); // ?

      cartOff();

      // Reading 4kbit EEPROM as 64kbit just gives the same 8 bytes repeated
      for (int currByte = 0; currByte < 512 - 8; currByte++)
      {
        if (OSCR::Storage::Shared::buffer[currByte] != OSCR::Storage::Shared::buffer[currByte + 8])
        {
          saveType = 2;
          break;
        }
      }
    }

    return true;
  }

  void configureROM()
  {
    constexpr uint8_t const romOptionsGBASize[] = { 1, 2, 4, 8, 16, 32, };
    cartSize = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeMB), romOptionsGBASize, sizeofarray(romOptionsGBASize));
    cartSize *= 0x100000;
  }

  void configureSave()
  {
    constexpr uint8_t const saveOptionsGBAType[] = { 0, 1, 2, 3, 6, 4, 5 };
    uint8_t selection = OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetCartType), saveOptionsGBA, sizeofarray(saveOptionsGBA));
    saveType = saveOptionsGBAType[selection];
  }

  void configureCart()
  {
    configureROM();

    if (OSCR::Prompts::askYesNo(OSCR::Strings::MenuOptions::SetSaveType))
    {
      configureSave();
    }
    else
    {
      saveType = 0;
    }
  }

  // Dump ROM
  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoyAdvance), FS(OSCR::Strings::Directory::ROM), fileName);

    OSCR::UI::ProgressBar::init((uint32_t)(cartSize), 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    cartOn();

    // Read rom
    for (uint32_t myAddress = 0; myAddress < cartSize; myAddress += 512)
    {
      for (int currWord = 0; currWord < 512; currWord += 2)
      {
        uint16_t tempWord = readWord(myAddress + currWord);

        OSCR::Storage::Shared::buffer[currWord] = tempWord & 0xFF;
        OSCR::Storage::Shared::buffer[currWord + 1] = (tempWord >> 8) & 0xFF;
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    cartOff();

    // Fix unmapped ROM area of cartridges with 32 MB ROM + EEPROM save type
    if ((cartSize == 0x2000000) && ((saveType == 1) || (saveType == 2)))
    {
      uint8_t padding_byte[256];
      char tempStr[20];

      OSCR::Storage::Shared::sharedFile.seekSet(0x1FFFEFF);
      OSCR::Storage::Shared::sharedFile.read(padding_byte, 1);

      sprintf_P(tempStr, PSTR("Fix Padding (0x%02X)"), padding_byte[0]);

      OSCR::UI::printLine(tempStr);

      memset(padding_byte + 1, padding_byte[0], 255);

      OSCR::Storage::Shared::sharedFile.write(padding_byte, 256);
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::ProgressBar::finish();

    // Overwrite the progress bar
    OSCR::UI::setLineRel(-1);
    OSCR::UI::clearLine();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    OSCR::Storage::Shared::rewind();

    OSCR::Storage::Shared::readBuffer();

    OSCR::Storage::Shared::close();

    uint8_t calcChecksum = checksumHeader(OSCR::Storage::Shared::buffer);

    OSCR::UI::clearLine();

    OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);

    if (calcChecksum == checksum)
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::Arrow));
      OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
      OSCR::UI::printHexLine(checksum);
      OSCR::UI::error(FS(OSCR::Strings::Errors::IncorrectChecksum));
    }

    // CRC32
    if (gbaCRDB->matchCRC(OSCR::CRC32::current))
    {
      OSCR::Storage::Shared::rename_P(gbaCRDB->record()->data()->name, FS(OSCR::Strings::FileType::GameBoyAdvance));
    }
  }

  void readSave()
  {
    switch (saveType)
    {
    case 1: // 4K EEPROM
      readEeprom(4);
      break;

    case 2: // 64K EEPROM
      readEeprom(64);
      break;

    case 3: // 256K SRAM/FRAM
      readSRAM(32768, 0);
      break;

    case 4: // 512K Flash
      readFlash();
      break;

    case 5: // 1M Flash (divided into two banks)
      readFlash();
      break;

    case 6: // 512K SRAM/FRAM
      readSRAM(65536, 0);
      break;
    }
  }

  void writeSave()
  {
    switch (getSaveType())
    {
    case 1: // 4K EEPROM
      writeEeprom(4);
      verifyEEP(4);
      setROM();
      break;

    case 2: // 64K EEPROM
      writeEeprom(64);
      verifyEEP(64);
      break;

    case 3: // 256K SRAM/FRAM
      writeSRAM(32768, 0);
      break;

    case 4: // 512K Flash
      idFlash();
      resetFlash();

      if (flashid == 0x1F3D)
      {
        printFlashTypeAndWait(PSTR("Atmel AT29LV512"));
      }
      else if (flashid == 0xBFD4)
      {
        printFlashTypeAndWait(PSTR("SST 39VF512"));
      }
      else if (flashid == 0xC21C)
      {
        printFlashTypeAndWait(PSTR("Macronix MX29L512"));
      }
      else if (flashid == 0x321B)
      {
        printFlashTypeAndWait(PSTR("Panasonic MN63F805MNP"));
      }
      else
      {
        printFlashTypeAndWait(OSCR::Strings::Common::Unknown);
      }

      if (flashid == 0x1F3D) // Atmel
      {
        writeFlash(1, 65536, 0, 1);
        verifyFlash(65536, 0);
      }
      else
      {
        eraseFlash();

        const uint32_t blankCheckSize = (strcmp_P(cartID, PSTR("PEAJ")) == 0) ? 65408 : 65536;

        if (blankcheckFlash(blankCheckSize))
        {
          writeFlash(1, blankCheckSize, 0, 0);
          verifyFlash(blankCheckSize, 0);
        }
      }
      break;

    case 5: // 1M Flash
      idFlash();
      resetFlash();

      if (flashid == 0xC209)
      {
        printFlashTypeAndWait(PSTR("Macronix MX29L010"));
      }
      else if (flashid == 0x6213)
      {
        printFlashTypeAndWait(PSTR("SANYO LE26FV10N1TS"));
      }
      else
      {
        printFlashTypeAndWait(OSCR::Strings::Common::Unknown);
      }

      eraseFlash();

      // 131072 bytes are divided into two 65536 byte banks
      for (uint8_t bank = 0; bank < 2; bank++)
      {
        switchBank(bank);
        setROM();

        // Card e-Reader+ does not allow access to last 128 bytes of a bank
        if (strcmp_P(cartID, PSTR("PSAE")) == 0 || strcmp_P(cartID, PSTR("PSAJ")) == 0) // Japanese e-reader+, USA e-reader
        {
          if (!blankcheckFlash(65408))
            break;
          writeFlash(!bank, 65408, bank ? 65536 : 0, 0);
          if (verifyFlash(65408, bank ? 65536 : 0))
            break;
        }
        else
        {
          if (!blankcheckFlash(65536))
            break;
          writeFlash(!bank, 65536, bank ? 65536 : 0, 0);
          if (verifyFlash(65536, bank ? 65536 : 0))
            break;
        }
      }
      break;

    case 6: // 512K SRAM/FRAM
      writeSRAM(65536, 0);
      break;
    }

    setROM();
  }

  /******************************************
    GBA SRAM SAVE Functions
  *****************************************/
  void readSRAM(uint32_t sramSize, uint32_t pos)
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoyAdvance), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::SaveRAM));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    cartOn();

    // Seek to a new position in the file
    if (pos != 0)
    {
      OSCR::Storage::Shared::sharedFile.seekCur(pos);
    }

    for (uint32_t currAddress = 0; currAddress < sramSize; currAddress += 512)
    {
      for (int c = 0; c < 512; c++)
      {
        // Read byte
        OSCR::Storage::Shared::buffer[c] = readByte(currAddress + c);
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    setROM();

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void writeSRAM(uint32_t sramSize, uint32_t pos)
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    // Seek to a new position in the file
    if (pos != 0)
    {
      OSCR::Storage::Shared::sharedFile.seekCur(pos);
    }

    for (uint32_t currAddress = 0; currAddress < sramSize; currAddress += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++)
      {
        // Write byte
        writeByte(currAddress + c, OSCR::Storage::Shared::buffer[c]);
      }
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    OSCR::Storage::Shared::rewind();

    // Variable for errors
    writeErrors = 0;

    // Seek to a new position in the file
    if (pos != 0)
    {
      OSCR::Storage::Shared::sharedFile.seekCur(pos);
    }

    for (uint32_t currAddress = 0; currAddress < sramSize; currAddress += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++)
      {
        // Read byte
        if (readByte(currAddress + c) != OSCR::Storage::Shared::buffer[c])
        {
          writeErrors++;
        }
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    if (writeErrors != 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
      return;
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
  }

  /******************************************
    GBA FRAM SAVE Functions
  *****************************************/
  // MB85R256 FRAM (Ferroelectric Random Access Memory) 32,768 words x 8 bits
  void readFRAM(uint32_t framSize)
  {
    // Output a HIGH signal on CS_ROM(PH3) WE_SRAM(PH5)
    PORTH |= (1 << 3) | (1 << 5);

    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;

    // Set data pins to input
    DDRC = 0x00;

    // Output a LOW signal on  CE_SRAM(PH0) and OE_SRAM(PH6)
    PORTH &= ~((1 << 0) | (1 << 6));

    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoyAdvance), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveRAM));

    for (uint32_t currAddress = 0; currAddress < framSize; currAddress += 512) {
      for (int c = 0; c < 512; c++) {
        // Pull OE_SRAM(PH6) HIGH
        PORTH |= (1 << 6);

        // Set address
        PORTF = (currAddress + c) & 0xFF;
        PORTK = ((currAddress + c) >> 8) & 0xFF;

        // Arduino running at 16Mhz -> one nop = 62.5ns
        // Leave CS_SRAM HIGH for at least 85ns
        AVR_ASM(
          AVR_INS("nop")
          AVR_INS("nop")
        );

        // Pull OE_SRAM(PH6) LOW
        PORTH &= ~(1 << 6);

        // Hold address for at least 25ns and wait 150ns before access
        AVR_ASM(
          AVR_INS("nop")
          AVR_INS("nop")
          AVR_INS("nop")
        );

        // Read byte
        OSCR::Storage::Shared::buffer[c] = PINC;
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    cartOff();

    // Close the file:
    OSCR::Storage::Shared::close();

    // Signal end of process
    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  // Write file to FRAM
  void writeFRAM(uint32_t framSize)
  {
    // Output a HIGH signal on CS_ROM(PH3) and OE_SRAM(PH6)
    PORTH |= (1 << 3) | (1 << 6);

    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;

    // Set data port to output
    DDRC = 0xFF;

    // Output a LOW signal on CE_SRAM(PH0) and WE_SRAM(PH5)
    PORTH &= ~((1 << 0) | (1 << 5));

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    for (uint32_t currAddress = 0; currAddress < framSize; currAddress += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++)
      {
        // Output Data on PORTC
        PORTC = OSCR::Storage::Shared::buffer[c];

        // Arduino running at 16Mhz -> one nop = 62.5ns
        // Data setup time 50ns
        AVR_ASM(AVR_INS("nop"));

        // Pull WE_SRAM (PH5) HIGH
        PORTH |= (1 << 5);

        // Set address
        PORTF = (currAddress + c) & 0xFF;
        PORTK = ((currAddress + c) >> 8) & 0xFF;

        // Leave WE_SRAM (PH5) HIGH for at least 85ns
        AVR_ASM(
          AVR_INS("nop")
          AVR_INS("nop")
        );

        // Pull WE_SRAM (PH5) LOW
        PORTH &= ~(1 << 5);

        // Hold address for at least 25ns and wait 150ns before next write
        AVR_ASM(
          AVR_INS("nop")
          AVR_INS("nop")
          AVR_INS("nop")
        );
      }
    }

    // Output a HIGH signal on CS_ROM(PH3) WE_SRAM(PH5)
    PORTH |= (1 << 3) | (1 << 5);

    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;

    // Set data pins to input
    DDRC = 0x00;

    // Output a LOW signal on  CE_SRAM(PH0) and OE_SRAM(PH6)
    PORTH &= ~((1 << 0) | (1 << 6));

    OSCR::Storage::Shared::rewind();

    // Variable for errors
    writeErrors = 0;

    for (uint32_t currAddress = 0; currAddress < framSize; currAddress += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512; c++) {
        // Pull OE_SRAM(PH6) HIGH
        PORTH |= (1 << 6);

        // Set address
        PORTF = (currAddress + c) & 0xFF;
        PORTK = ((currAddress + c) >> 8) & 0xFF;

        // Arduino running at 16Mhz -> one nop = 62.5ns
        // Leave CS_SRAM HIGH for at least 85ns
        AVR_ASM(
          AVR_INS("nop")
          AVR_INS("nop")
        );

        // Pull OE_SRAM(PH6) LOW
        PORTH &= ~(1 << 6);

        // Hold address for at least 25ns and wait 150ns before access
        AVR_ASM(
          AVR_INS("nop")
          AVR_INS("nop")
          AVR_INS("nop")
        );

        // Read byte
        if (PINC != OSCR::Storage::Shared::buffer[c])
        {
          writeErrors++;
        }
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    if (writeErrors != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }
  }

  /******************************************
    GBA FLASH SAVE Functions
  *****************************************/
  void initOutputFlash()
  {
    // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
    PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;
    // Set data pins to output
    DDRC = 0xFF;

    // Output a LOW signal on CE_FLASH(PH0)
    PORTH &= ~(1 << 0);
  }

  // SST 39VF512 Flashrom
  void idFlash()
  {
    initOutputFlash();

    // ID command sequence
    writeByteFlash(0x5555, 0xAA);
    writeByteFlash(0x2AAA, 0x55);
    writeByteFlash(0x5555, 0x90);

    // Set data pins to input
    DDRC = 0x00;

    // Output a LOW signal on OE_FLASH(PH6)
    PORTH &= ~(1 << 6);

    // Wait 150ns before reading ID
    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read the two id bytes into a string
    flashid = readByteFlash(0) << 8;
    flashid |= readByteFlash(1);

    // Set CS_FLASH(PH0) high
    PORTH |= (1 << 0);
  }

  // Reset FLASH
  void resetFlash()
  {
    initOutputFlash();

    // Reset command sequence
    writeByteFlash(0x5555, 0xAA);
    writeByteFlash(0x2AAA, 0x55);
    writeByteFlash(0x5555, 0xF0);
    writeByteFlash(0x5555, 0xF0);

    // Set CS_FLASH(PH0) high
    PORTH |= (1 << 0);

    // Wait
    delay(100);
  }

  uint8_t readByteFlash(uint16_t myAddress)
  {
    // Set address
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;

    // Wait until byte is ready to read
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read byte
    uint8_t tempByte = PINC;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    return tempByte;
  }

  void writeByteFlash(uint16_t myAddress, uint8_t myData)
  {
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTC = myData;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Switch WE_FLASH(PH5) to LOW
    PORTH &= ~(1 << 5);

    // Leave WE low for at least 40ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Switch WE_FLASH(PH5) to HIGH
    PORTH |= (1 << 5);

    // Leave WE high for a bit
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );
  }

  // Erase Flash
  void eraseFlash()
  {
    initOutputFlash();

    // Erase command sequence
    writeByteFlash(0x5555, 0xAA);
    writeByteFlash(0x2AAA, 0x55);
    writeByteFlash(0x5555, 0x80);
    writeByteFlash(0x5555, 0xAA);
    writeByteFlash(0x2AAA, 0x55);
    writeByteFlash(0x5555, 0x10);

    // Set CS_FLASH(PH0) high
    PORTH |= (1 << 0);

    // Wait until all is erased
    delay(500);
  }

  bool blankcheckFlash(uint32_t flashSize)
  {
    // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
    PORTH |= (1 << 3) | (1 << 5);

    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;
    // Set address to 0
    PORTF = 0x00;
    PORTK = 0x00;

    // Set data pins to input
    DDRC = 0x00;
    // Disable Pullups
    //PORTC = 0x00;

    bool blank = 1;

    // Output a LOW signal on  CE_FLASH(PH0)
    PORTH &= ~(1 << 0);

    // Output a LOW signal on OE_FLASH(PH6)
    PORTH &= ~(1 << 6);

    for (uint32_t currAddress = 0; currAddress < flashSize; currAddress += 512)
    {
      for (int c = 0; c < 512; c++)
      {
        // Read byte
        OSCR::Storage::Shared::buffer[c] = readByteFlash(currAddress + c);
      }

      // Check buffer
      for (uint32_t currByte = 0; currByte < 512 && (currByte + currAddress) < flashSize; currByte++)
      {
        if (OSCR::Storage::Shared::buffer[currByte] != 0xFF)
        {
          OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
          currByte = 512;
          currAddress = flashSize;
          blank = 0;
        }
      }
    }

    // Set CS_FLASH(PH0) high
    PORTH |= (1 << 0);

    return blank;
  }

  // The MX29L010 is 131072 bytes in size and has 16 sectors per bank
  // each sector is 4096 bytes, there are 32 sectors total
  // therefore the bank size is 65536 bytes, so we have two banks in total
  void switchBank(uint8_t bankNum)
  {
    initOutputFlash();

    // Switch bank command sequence
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0xB0);
    writeByte(0x0000, bankNum);

    // Set CS_FLASH(PH0) high
    PORTH |= (1 << 0);
  }

  void readFlash()
  {
    constexpr uint32_t const chipSize = 65536;
    uint8_t const flashChips = (saveType == 5) ? 2 : 1;
    uint32_t const flashSize = chipSize * flashChips;

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoyAdvance), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveFlash));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    cartOn();

    // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
    PORTH |= (1 << 3) | (1 << 5);

    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;
    // Set address to 0
    PORTF = 0x00;
    PORTK = 0x00;

    // Set data pins to input
    DDRC = 0x00;

    for (uint8_t chip = 0; chip < flashChips; chip++)
    {
      if (flashChips > 1)
      {
        switchBank(chip);
        setROM();
      }

      PORTH &= ~(1 << 0); // Output a LOW signal on CE_FLASH(PH0)
      PORTH &= ~(1 << 6); // Output a LOW signal on OE_FLASH(PH6)

      for (uint32_t currAddress = 0; currAddress < chipSize; currAddress += 512)
      {
        for (int c = 0; c < 512; c++)
        {
          OSCR::Storage::Shared::buffer[c] = readByteFlash(currAddress + c);
        }

        OSCR::Storage::Shared::writeBuffer();
      }

      PORTH |= (1 << 0); // Set CS_FLASH(PH0) high
    }

    setROM();

    OSCR::Storage::Shared::close();

    cartOff();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void busyCheck(uint16_t currByte)
  {
    // Set data pins to input
    DDRC = 0x00;

    // Output a LOW signal on OE_FLASH(PH6)
    PORTH &= ~(1 << 6);

    // Read PINC
    while (PINC != OSCR::Storage::Shared::buffer[currByte]);

    // Output a HIGH signal on OE_FLASH(PH6)
    PORTH |= (1 << 6);

    // Set data pins to output
    DDRC = 0xFF;
  }

  void writeFlash(bool browseFile, uint32_t flashSize, uint32_t pos, bool isAtmel)
  {
    // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
    PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;
    // Set data port to output
    DDRC = 0xFF;

    if (browseFile)
    {
      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);
    }
    else if (!OSCR::Storage::Shared::sharedFile.isOpen())
    {
      OSCR::UI::fatalErrorStorage();
    }

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    // Seek to a new position in the file
    if (pos != 0)
    {
      OSCR::Storage::Shared::sharedFile.seekCur(pos);
    }

    // Output a LOW signal on CE_FLASH(PH0)
    PORTH &= ~(1 << 0);

    if (!isAtmel)
    {
      for (uint32_t currAddress = 0; currAddress < flashSize; currAddress += 512)
      {
        OSCR::Storage::Shared::fill();

        for (int c = 0; c < 512 && (currAddress + c) < flashSize; c++)
        {
          // Write command sequence
          writeByteFlash(0x5555, 0xAA);
          writeByteFlash(0x2AAA, 0x55);
          writeByteFlash(0x5555, 0xA0);
          // Write current byte
          writeByteFlash(currAddress + c, OSCR::Storage::Shared::buffer[c]);

          // Wait
          busyCheck(c);
        }
      }
    }
    else
    {
      for (uint32_t currAddress = 0; currAddress < flashSize; currAddress += 128)
      {
        OSCR::Storage::Shared::readBuffer(128);

        // Write command sequence
        writeByteFlash(0x5555, 0xAA);
        writeByteFlash(0x2AAA, 0x55);
        writeByteFlash(0x5555, 0xA0);

        for (int c = 0; c < 128; c++)
        {
          writeByteFlash(currAddress + c, OSCR::Storage::Shared::buffer[c]);
        }

        delay(15);
      }
    }

    // Set CS_FLASH(PH0) high
    PORTH |= (1 << 0);

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  // Check if the Flashrom was written without any error
  uint32_t verifyFlash(uint32_t flashSize, uint32_t pos)
  {
    // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
    PORTH |= (1 << 3) | (1 << 5);

    // Set address ports to output
    DDRF = 0xFF;
    DDRK = 0xFF;

    // Set data pins to input
    DDRC = 0x00;

    // Output a LOW signal on CE_FLASH(PH0) and  OE_FLASH(PH6)
    PORTH &= ~((1 << 0) | (1 << 6));

    // Signal beginning of process
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    uint32_t wrError = 0;

    OSCR::Storage::Shared::openRO();

    // Seek to a new position in the file
    if (pos != 0)
    {
      OSCR::Storage::Shared::sharedFile.seekCur(pos);
    }

    for (uint32_t currAddress = 0; currAddress < flashSize; currAddress += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int c = 0; c < 512 && (currAddress + c) < flashSize; c++)
      {
        // Read byte
        if (OSCR::Storage::Shared::buffer[c] != readByteFlash(currAddress + c))
        {
          wrError++;
        }
      }
    }

    // Set CS_FLASH(PH0) high
    PORTH |= (1 << 0);

    cartOff();

    OSCR::Storage::Shared::close();

    if (wrError == 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::Lang::printErrorVerifyBytes(wrError);
    }

    return wrError;
  }

  /******************************************
    GBA Eeprom SAVE Functions
  *****************************************/
  // Write eeprom from file
  void writeEeprom(uint16_t eepSize)
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    cartOn();

    for (uint16_t i = 0; i < eepSize * 16; i += 64)
    {
      OSCR::Storage::Shared::fill();

      // Disable interrupts for more uniform clock pulses
      noInterrupts();

      // Write 512 bytes
      writeBlock(i, eepSize);

      interrupts();

      // Wait
      delayMicroseconds(200);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  // Read eeprom to file
  void readEeprom(uint16_t eepSize)
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoyAdvance), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveEEPROM));

    cartOn();

    // Each block contains 8 Bytes, so for a 8KB eeprom 1024 blocks need to be read
    for (uint16_t currAddress = 0; currAddress < eepSize * 16; currAddress += 64)
    {
      // Disable interrupts for more uniform clock pulses
      noInterrupts();
      // Fill sd Buffer
      readBlock(currAddress, eepSize);
      interrupts();

      OSCR::Storage::Shared::writeBuffer();

      // Wait
      delayMicroseconds(200);
    }

    setROM();

    cartOff();

    OSCR::Storage::Shared::close();
  }

  // Send address as bits to eeprom
  void send(uint16_t currAddr, uint16_t numBits)
  {
    for (uint16_t addrBit = numBits; addrBit > 0; addrBit--) {
      // If you want the k-th bit of n, then do
      // (n & ( 1 << k )) >> k
      if (((currAddr & (1 << (addrBit - 1))) >> (addrBit - 1))) {
        // Set A0(PF0) to High
        PORTF |= (1 << 0);
        // Set WR(PH5) to LOW
        PORTH &= ~(1 << 5);
        // Set WR(PH5) to High
        PORTH |= (1 << 5);
      } else {
        // Set A0(PF0) to Low
        PORTF &= ~(1 << 0);
        // Set WR(PH5) to LOW
        PORTH &= ~(1 << 5);
        // Set WR(PH5) to High
        PORTH |= (1 << 5);
      }
    }
  }

  // Write 512K eeprom block
  void writeBlock(uint16_t startAddr, uint16_t eepSize)
  {
    // Setup
    // Set CS_ROM(PH3) WR(PH5) RD(PH6) to Output
    DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
    // Set A0(PF0) to Output
    DDRF |= (1 << 0);
    // Set A23/D7(PC7) to Output
    DDRC |= (1 << 7);

    // Set CS_ROM(PH3) WR(PH5) RD(PH6) to High
    PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
    // Set A0(PF0) to High
    PORTF |= (1 << 0);
    // Set A23/D7(PC7) to High
    PORTC |= (1 << 7);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Write 64*8=512 bytes
    for (uint16_t currAddr = startAddr; currAddr < startAddr + 64; currAddr++) {
      // Set CS_ROM(PH3) to LOW
      PORTH &= ~(1 << 3);

      // Send write request "10"
      // Set A0(PF0) to High
      PORTF |= (1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~(1 << 5);
      // Set WR(PH5) to High
      PORTH |= (1 << 5);
      // Set A0(PF0) to LOW
      PORTF &= ~(1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~(1 << 5);
      // Set WR(PH5) to High
      PORTH |= (1 << 5);

      // Send either 6 or 14 bit address
      if (eepSize == 4) {
        send(currAddr, 6);
      } else {
        send(currAddr, 14);
      }

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Send data
      for (uint8_t currByte = 0; currByte < 8; currByte++)
      {
        send(OSCR::Storage::Shared::buffer[(currAddr - startAddr) * 8 + currByte], 8);
      }

      // Send stop bit
      // Set A0(PF0) to LOW
      PORTF &= ~(1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~(1 << 5);
      // WR(PH5) to High
      PORTH |= (1 << 5);

      // Set CS_ROM(PH3) to High
      PORTH |= (1 << 3);

      // Wait until done
      // Set A0(PF0) to Input
      DDRF &= ~(1 << 0);

      do {
        // Set  CS_ROM(PH3) RD(PH6) to LOW
        PORTH &= ~((1 << 3) | (1 << 6));
        // Set  CS_ROM(PH3) RD(PH6) to High
        PORTH |= (1 << 3) | (1 << 6);
      } while ((PINF & 0x1) == 0);

      // Set A0(PF0) to Output
      DDRF |= (1 << 0);
    }
  }

  // Reads 512 bytes from eeprom
  void readBlock(uint16_t startAddress, uint16_t eepSize)
  {
    // Setup
    // Set CS_ROM(PH3) WR(PH5) RD(PH6) to Output
    DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
    // Set A0(PF0) to Output
    DDRF |= (1 << 0);
    // Set A23/D7(PC7) to Output
    DDRC |= (1 << 7);

    // Set CS_ROM(PH3) WR(PH5) RD(PH6) to High
    PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
    // Set A0(PF0) to High
    PORTF |= (1 << 0);
    // Set A23/D7(PC7) to High
    PORTC |= (1 << 7);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read 64*8=512 bytes
    for (uint16_t currAddr = startAddress; currAddr < startAddress + 64; currAddr++) {
      // Set CS_ROM(PH3) to LOW
      PORTH &= ~(1 << 3);

      // Send read request "11"
      // Set A0(PF0) to High
      PORTF |= (1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~(1 << 5);
      // Set WR(PH5) to High
      PORTH |= (1 << 5);
      // Set WR(PH5) to LOW
      PORTH &= ~(1 << 5);
      // Set WR(PH5) to High
      PORTH |= (1 << 5);

      // Send either 6 or 14 bit address
      if (eepSize == 4) {
        send(currAddr, 6);
      } else {
        send(currAddr, 14);
      }

      // Send stop bit
      // Set A0(PF0) to LOW
      PORTF &= ~(1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~(1 << 5);
      // WR(PH5) to High
      PORTH |= (1 << 5);

      // Set CS_ROM(PH3) to High
      PORTH |= (1 << 3);

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Read data
      // Set A0(PF0) to Input
      DDRF &= ~(1 << 0);
      // Set CS_ROM(PH3) to low
      PORTH &= ~(1 << 3);

      // Array that holds the bits
      bool tempBits[65];

      // Ignore the first 4 bits
      for (uint8_t i = 0; i < 4; i++) {
        // Set RD(PH6) to LOW
        PORTH &= ~(1 << 6);
        // Set RD(PH6) to High
        PORTH |= (1 << 6);
      }

      // Read the remaining 64bits into array
      for (uint8_t currBit = 0; currBit < 64; currBit++) {
        // Set RD(PH6) to LOW
        PORTH &= ~(1 << 6);
        // Set RD(PH6) to High
        PORTH |= (1 << 6);

        // Read bit from A0(PF0)
        tempBits[currBit] = (PINF & 0x1);
      }

      // Set CS_ROM(PH3) to High
      PORTH |= (1 << 3);
      // Set A0(PF0) to High
      PORTF |= (1 << 0);
      // Set A0(PF0) to Output
      DDRF |= (1 << 0);

      // OR 8 bits into one byte for a total of 8 bytes
      for (uint8_t j = 0; j < 64; j += 8)
      {
        OSCR::Storage::Shared::buffer[((currAddr - startAddress) * 8) + (j / 8)] = tempBits[0 + j] << 7 | tempBits[1 + j] << 6 | tempBits[2 + j] << 5 | tempBits[3 + j] << 4 | tempBits[4 + j] << 3 | tempBits[5 + j] << 2 | tempBits[6 + j] << 1 | tempBits[7 + j];
      }
    }
  }

  // Check if the SRAM was written without any error
  uint32_t verifyEEP(uint16_t eepSize)
  {
    uint32_t wrError = 0;

    //open file on sd card
    OSCR::Storage::Shared::openRO();

    // Fill sd Buffer
    for (uint16_t currAddress = 0; currAddress < eepSize * 16; currAddress += 64)
    {
      // Disable interrupts for more uniform clock pulses
      noInterrupts();
      readBlock(currAddress, eepSize);
      interrupts();

      // Compare
      for (int currByte = 0; currByte < 512; currByte++)
      {
        if (OSCR::Storage::Shared::buffer[currByte] != OSCR::Storage::Shared::sharedFile.read())
        {
          wrError++;
        }
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    if (wrError != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(wrError);
    }

    return wrError;
  }

# if HAS_FLASH
  /******************************************
    GBA REPRO Functions (32MB Intel 4000L0YBQ0 and 16MB MX29GL128E)
  *****************************************/
  // Reset to read mode
  void resetIntel(uint32_t partitionSize)
  {
    for (uint32_t currPartition = 0; currPartition < cartSize; currPartition += partitionSize) {
      writeWord(currPartition, 0xFFFF);
    }
  }

  void resetMX29GL128E()
  {
    writeWord_GAB(0, 0xF0);
  }

  bool sectorCheckMX29GL128E()
  {
    bool sectorProtect = 0;
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x90);
    for (uint32_t currSector = 0x0; currSector < 0xFFFFFF; currSector += 0x20000) {
      if (readWord_GAB(currSector + 0x04) != 0x0)
        sectorProtect = 1;
    }
    resetMX29GL128E();
    return sectorProtect;
  }

  void idFlashrom()
  {
    // Send Intel ID command to flashrom
    writeWord(0, 0x90);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read flashrom ID
    flashid = readWord(0x2) & 0xFF00;
    flashid |= readWord(0x4) & 0xFF;

    // Intel Strataflash
    if (flashid == 0x8802 || (flashid == 0x8816))
    {
      cartSize = 0x2000000;
    }
    else if (flashid == 0x8812) // F0088H0
    {
      cartSize = 0x10000000;
    }
    else
    {
      // Send swapped MX29GL128E/MSP55LV128 ID command to flashrom
      writeWord_GAB(0xAAA, 0xAA);
      writeWord_GAB(0x555, 0x55);
      writeWord_GAB(0xAAA, 0x90);

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Read flashrom ID
      flashid = readWord_GAB(0x2);

      // MX29GL128E or MSP55LV128
      if (flashid == 0x227E)
      {
        // MX is 0xC2 and MSP is 0x4 or 0x1
        romType = (readWord_GAB(0x0) & 0xFF);
        cartSize = 0x1000000;
        resetMX29GL128E();
      }
      else
      {
        OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));

        OSCR::UI::printLabel(OSCR::Strings::Common::ID);
        OSCR::UI::printHexLine(flashid);

        OSCR::UI::fatalError(FS(OSCR::Strings::Errors::UnknownType));
      }
    }
  }

  bool blankcheckFlashrom()
  {
    bool blank = 1;

    for (uint32_t currByte = 0; currByte < fileSize; currByte += 2)
    {
      // Check if all bytes are 0xFFFF
      if (readWord(currByte) != 0xFFFF)
      {
        currByte = fileSize;
        blank = 0;
      }
    }

    return (blank == 1);
  }

  void eraseIntel4000()
  {
    // If the game is smaller than 16Mbit only erase the needed blocks
    uint32_t lastBlock = 0xFFFFFF;
    if (fileSize < 0xFFFFFF)
    {
      lastBlock = fileSize;
    }

    // Erase 4 blocks with 16kwords each
    for (uint32_t currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000)
    {
      // Unlock Block
      writeWord(currBlock, 0x60);
      writeWord(currBlock, 0xD0);

      // Erase Command
      writeWord(currBlock, 0x20);
      writeWord(currBlock, 0xD0);

      // Read the status register
      uint16_t statusReg = readWord(currBlock);

      while ((statusReg | 0xFF7F) != 0xFFFF)
      {
        statusReg = readWord(currBlock);
      }
    }

    // Erase 126 blocks with 64kwords each
    for (uint32_t currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF)
    {
      // Unlock Block
      writeWord(currBlock, 0x60);
      writeWord(currBlock, 0xD0);

      // Erase Command
      writeWord(currBlock, 0x20);
      writeWord(currBlock, 0xD0);

      // Read the status register
      uint16_t statusReg = readWord(currBlock);

      while ((statusReg | 0xFF7F) != 0xFFFF)
      {
        statusReg = readWord(currBlock);
      }
    }

    // Erase the second chip
    if (fileSize > 0xFFFFFF)
    {
      // 126 blocks with 64kwords each
      for (uint32_t currBlock = 0x1000000; currBlock < 0x1FDFFFF; currBlock += 0x1FFFF)
      {
        // Unlock Block
        writeWord(currBlock, 0x60);
        writeWord(currBlock, 0xD0);

        // Erase Command
        writeWord(currBlock, 0x20);
        writeWord(currBlock, 0xD0);

        // Read the status register
        uint16_t statusReg = readWord(currBlock);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          statusReg = readWord(currBlock);
        }
      }

      // 4 blocks with 16kword each
      for (uint32_t currBlock = 0x1FE0000; currBlock < 0x1FFFFFF; currBlock += 0x8000)
      {
        // Unlock Block
        writeWord(currBlock, 0x60);
        writeWord(currBlock, 0xD0);

        // Erase Command
        writeWord(currBlock, 0x20);
        writeWord(currBlock, 0xD0);

        // Read the status register
        uint16_t statusReg = readWord(currBlock);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          statusReg = readWord(currBlock);
        }
      }
    }
  }

  void eraseIntel4400()
  {
    // If the game is smaller than 32Mbit only erase the needed blocks
    uint32_t lastBlock = 0x1FFFFFF;
    if (fileSize < 0x1FFFFFF)
      lastBlock = fileSize;

    // Erase 4 blocks with 16kwords each
    for (uint32_t currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000)
    {
      // Unlock Block
      writeWord(currBlock, 0x60);
      writeWord(currBlock, 0xD0);

      // Erase Command
      writeWord(currBlock, 0x20);
      writeWord(currBlock, 0xD0);

      // Read the status register
      uint16_t statusReg = readWord(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF)
      {
        statusReg = readWord(currBlock);
      }
    }

    // Erase 255 blocks with 64kwords each
    for (uint32_t currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF)
    {
      // Unlock Block
      writeWord(currBlock, 0x60);
      writeWord(currBlock, 0xD0);

      // Erase Command
      writeWord(currBlock, 0x20);
      writeWord(currBlock, 0xD0);

      // Read the status register
      uint16_t statusReg = readWord(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF)
      {
        statusReg = readWord(currBlock);
      }
    }

    /* No need to erase the second chip as max rom size is 32MB
      if (fileSize > 0x2000000) {
      // 255 blocks with 64kwords each
      for (uint32_t currBlock = 0x2000000; currBlock < 0x3FDFFFF; currBlock += 0x1FFFF) {
        // Unlock Block
        writeWord(currBlock, 0x60);
        writeWord(currBlock, 0xD0);

        // Erase Command
        writeWord(currBlock, 0x20);
        writeWord(currBlock, 0xD0);

        // Read the status register
        uint16_t statusReg = readWord(currBlock);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          statusReg = readWord(currBlock);
        }
      }

      // 4 blocks with 16kword each
      for (uint32_t currBlock = 0x3FE0000; currBlock < 0x3FFFFFF; currBlock += 0x8000) {
        // Unlock Block
        writeWord(currBlock, 0x60);
        writeWord(currBlock, 0xD0);

        // Erase Command
        writeWord(currBlock, 0x20);
        writeWord(currBlock, 0xD0);

        // Read the status register
        uint16_t statusReg = readWord(currBlock);
        while ((statusReg | 0xFF7F) != 0xFFFF)
        {
          statusReg = readWord(currBlock);
        }
      }
      }*/
  }

  void sectorEraseMSP55LV128()
  {
    uint32_t lastSector = 0xFFFFFF;

    // Erase 256 sectors with 64kbytes each
    for (uint32_t currSector = 0x0; currSector < lastSector; currSector += 0x10000)
    {
      writeWord_GAB(0xAAA, 0xAA);
      writeWord_GAB(0x555, 0x55);
      writeWord_GAB(0xAAA, 0x80);
      writeWord_GAB(0xAAA, 0xAA);
      writeWord_GAB(0x555, 0x55);
      writeWord_GAB(currSector, 0x30);

      // Read the status register
      uint16_t statusReg = readWord_GAB(currSector);

      while ((statusReg | 0xFF7F) != 0xFFFF)
      {
        statusReg = readWord_GAB(currSector);
      }
    }
  }

  void sectorEraseMX29GL128E()
  {
    uint32_t lastSector = 0xFFFFFF;

    // Erase 128 sectors with 128kbytes each
    for (uint32_t currSector = 0x0; currSector < lastSector; currSector += 0x20000)
    {
      writeWord_GAB(0xAAA, 0xAA);
      writeWord_GAB(0x555, 0x55);
      writeWord_GAB(0xAAA, 0x80);
      writeWord_GAB(0xAAA, 0xAA);
      writeWord_GAB(0x555, 0x55);
      writeWord_GAB(currSector, 0x30);

      // Read the status register
      uint16_t statusReg = readWord_GAB(currSector);

      while ((statusReg | 0xFF7F) != 0xFFFF)
      {
        statusReg = readWord_GAB(currSector);
      }
    }
  }

  void writeIntel4000()
  {
    for (uint32_t currBlock = 0; currBlock < fileSize; currBlock += 0x20000)
    {
      // Write to flashrom
      for (uint32_t currSdBuffer = 0; currSdBuffer < 0x20000; currSdBuffer += 512)
      {
        OSCR::Storage::Shared::fill();

        // Write 32 words at a time
        for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64)
        {
          // Unlock Block
          writeWord(currBlock + currSdBuffer + currWriteBuffer, 0x60);
          writeWord(currBlock + currSdBuffer + currWriteBuffer, 0xD0);

          // Buffered program command
          writeWord(currBlock + currSdBuffer + currWriteBuffer, 0xE8);

          // Check Status register
          uint16_t statusReg = readWord(currBlock + currSdBuffer + currWriteBuffer);

          while ((statusReg | 0xFF7F) != 0xFFFF)
          {
            statusReg = readWord(currBlock + currSdBuffer + currWriteBuffer);
          }

          // Write word count (minus 1)
          writeWord(currBlock + currSdBuffer + currWriteBuffer, 0x1F);

          // Write buffer
          for (uint8_t currByte = 0; currByte < 64; currByte += 2)
          {
            // Join two bytes into one word
            uint16_t currWord = ((OSCR::Storage::Shared::buffer[currWriteBuffer + currByte + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currWriteBuffer + currByte] & 0xFF);
            writeWord(currBlock + currSdBuffer + currWriteBuffer + currByte, currWord);
          }

          // Write Buffer to Flash
          writeWord(currBlock + currSdBuffer + currWriteBuffer + 62, 0xD0);

          // Read the status register at last written address
          statusReg = readWord(currBlock + currSdBuffer + currWriteBuffer + 62);
          while ((statusReg | 0xFF7F) != 0xFFFF)
          {
            statusReg = readWord(currBlock + currSdBuffer + currWriteBuffer + 62);
          }
        }
      }
    }
  }

  void writeMSP55LV128()
  {
    for (uint32_t currSector = 0; currSector < fileSize; currSector += 0x10000)
    {
      // Write to flashrom
      for (uint32_t currSdBuffer = 0; currSdBuffer < 0x10000; currSdBuffer += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::fill();

        // Write 16 words at a time
        for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 32)
        {
          // Write Buffer command
          writeWord_GAB(0xAAA, 0xAA);
          writeWord_GAB(0x555, 0x55);
          writeWord_GAB(currSector, 0x25);

          // Write word count (minus 1)
          writeWord_GAB(currSector, 0xF);

          // Write buffer
          uint16_t currWord;

          for (uint8_t currByte = 0; currByte < 32; currByte += 2)
          {
            // Join two bytes into one uint16_t
            currWord = ((OSCR::Storage::Shared::buffer[currWriteBuffer + currByte + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currWriteBuffer + currByte] & 0xFF);
            writeWord(currSector + currSdBuffer + currWriteBuffer + currByte, currWord);
          }

          // Confirm write buffer
          writeWord_GAB(currSector, 0x29);

          // Read the status register
          uint16_t statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);

          while ((statusReg | 0xFF7F) != (currWord | 0xFF7F))
          {
            statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);
          }
        }
      }
    }
  }

  void writeMX29GL128E()
  {
    for (uint32_t currSector = 0; currSector < fileSize; currSector += 0x20000)
    {
      // Write to flashrom
      for (uint32_t currSdBuffer = 0; currSdBuffer < 0x20000; currSdBuffer += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::fill();

        // Write 32 words at a time
        for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64)
        {
          // Write Buffer command
          writeWord_GAB(0xAAA, 0xAA);
          writeWord_GAB(0x555, 0x55);
          writeWord_GAB(currSector, 0x25);

          // Write word count (minus 1)
          writeWord_GAB(currSector, 0x1F);

          // Write buffer
          uint16_t currWord;

          for (uint8_t currByte = 0; currByte < 64; currByte += 2)
          {
            // Join two bytes into one word
            currWord = ((OSCR::Storage::Shared::buffer[currWriteBuffer + currByte + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currWriteBuffer + currByte] & 0xFF);
            writeWord(currSector + currSdBuffer + currWriteBuffer + currByte, currWord);
          }

          // Confirm write buffer
          writeWord_GAB(currSector, 0x29);

          // Read the status register
          uint16_t statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 62);

          while ((statusReg | 0xFF7F) != (currWord | 0xFF7F))
          {
            statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 62);
          }
        }
      }
    }
  }

  bool verifyFlashrom()
  {
    OSCR::Storage::Shared::openRO();

    writeErrors = 0;

    for (uint32_t currSector = 0; currSector < fileSize; currSector += 131072)
    {
      for (uint32_t currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::fill();

        for (int currByte = 0; currByte < 512; currByte += 2)
        {
          // Join two bytes into one word
          uint16_t currWord = ((OSCR::Storage::Shared::buffer[currByte + 1] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currByte] & 0xFF);

          // Compare both
          if (readWord(currSector + currSdBuffer + currByte) != currWord)
          {
            writeErrors++;
          }
        }
      }
    }

    OSCR::Storage::Shared::close();

    return (writeErrors == 0);
  }

  //******************************************
  // 369in1 Repro functions
  //******************************************
  void reset369in1()
  {
    writeWord(0, 0xFF);
  }

  void mapBlock369in1(uint32_t offset)
  {
    // Taken from gbabf
    uint32_t chipAddr = (offset / 32 * 0x10000000) + (0x4000C0 + (offset & 31) * 0x20202);
    union {
      uint32_t addr;
      uint8_t byte[4];
    } addr;
    addr.addr = chipAddr;

    writeByte(0x2, addr.byte[3]);
    writeByte(0x3, addr.byte[2]);
    writeByte(0x4, addr.byte[1]);
    delay(500);
    setROM();
  }

  uint8_t selectBlockNumber(bool option)
  {
    uint8_t blockNumber;

    if (option) blockNumber = OSCR::UI::rangeSelect(F("BLOCK NUMBER"), 0, 63);
    else blockNumber = OSCR::UI::rangeSelect(FS(OSCR::Strings::Headings::SelectCartSize), 0, 32);

    if (option)
    {
      OSCR::UI::printValue(OSCR::Strings::Common::Block, blockNumber);
    }
    else
    {
      OSCR::UI::printSize(OSCR::Strings::Common::Block, blockNumber * 1024 * 1024);
    }

    delay(200);

    return blockNumber;
  }

  // Read 369-in-1 repro
  void read369in1(uint8_t blockNumber, uint8_t fileSizeByte)
  {
    uint8_t readBuffer[1024];
    strcpy_P(fileName, PSTR("369in1"));

    if (blockNumber != 0)
    {
      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoyAdvance), FS(OSCR::Strings::Directory::ROM), fileName, blockNumber);
    }
    else
    {
      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoyAdvance), FS(OSCR::Strings::Directory::ROM), fileName);
    }

    cartOn();

    if (fileSizeByte == 0)
      fileSize = 0x10000000;
    else
      fileSize = (uint32_t)fileSizeByte * 1024 * 1024;

    // 64 blocks at 4MB each
    uint32_t startBank = (((uint32_t)blockNumber * 4) / 32) * 0x2000000;
    uint32_t startBlock = ((uint32_t)blockNumber * 4 * 1024 * 1024) - startBank;
    uint32_t lastBlock = 0x2000000;
    if (fileSize < lastBlock)
      lastBlock = startBlock + fileSize;
    uint32_t lastBuffer = 0x400000;
    if (fileSize < lastBuffer)
      lastBuffer = fileSize;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    // 256MB repro size
    for (uint32_t currBank = startBank; currBank < startBank + fileSize; currBank += 0x2000000)
    {
      // 32MB bank
      for (uint32_t currBlock = startBlock; currBlock < lastBlock; currBlock += 0x400000)
      {
        // Set-up 369-in-1 mapper
        mapBlock369in1((currBank + currBlock) / 1024 / 1024);

        // 4MB Block
        for (uint32_t currBuffer = 0; currBuffer < lastBuffer; currBuffer += 1024)
        {
          // 1024 byte readBuffer
          for (int currWord = 0; currWord < 1024; currWord += 2)
          {
            uint16_t tempWord = readWord(currBlock + currBuffer + currWord);
            readBuffer[currWord] = tempWord & 0xFF;
            readBuffer[currWord + 1] = (tempWord >> 8) & 0xFF;
          }

          // Write to SD
          OSCR::Storage::Shared::sharedFile.write(readBuffer, 1024);

          OSCR::UI::ProgressBar::advance(1024);
        }
      }
    }

    OSCR::UI::ProgressBar::finish();

    cartOff();

    OSCR::Storage::Shared::close();
  }

  // Erase 369-in-1 repro
  void erase369in1(uint8_t blockNumber)
  {
    // 64 blocks at 4MB each
    uint32_t startBank = (((uint32_t)blockNumber * 4) / 32) * 0x2000000;
    uint32_t startBlock = ((uint32_t)blockNumber * 4 * 1024 * 1024) - startBank;
    uint32_t lastBlock = 0x2000000;
    if (fileSize < lastBlock)
      lastBlock = startBlock + fileSize;
    uint32_t lastSector = 0x400000;
    if (fileSize < lastSector)
      lastSector = fileSize;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    // 256MB repro size
    for (uint32_t currBank = startBank; currBank < startBank + fileSize; currBank += 0x2000000) {
      // 32MB bank
      for (uint32_t currBlock = startBlock; currBlock < lastBlock; currBlock += 0x400000) {
        // Set-up 369-in-1 mapper
        mapBlock369in1((currBank + currBlock) / 1024 / 1024);
        // 256KB flashrom sector size
        for (uint32_t currSector = 0; currSector < lastSector; currSector += 0x40000) {
          // Unlock Sector
          writeWord(currBlock + currSector, 0x60);
          writeWord(currBlock + currSector, 0xD0);

          // Erase Command
          writeWord(currBlock + currSector, 0x20);
          writeWord(currBlock + currSector, 0xD0);

          // Read the status register
          uint16_t statusReg = readWord(currBlock + currSector);
          while ((statusReg | 0xFF7F) != 0xFFFF) {
            statusReg = readWord(currBlock + currSector);
          }

          // update progress bar
          OSCR::UI::ProgressBar::advance(0x40000);
        }
      }
    }

    OSCR::UI::ProgressBar::finish();

    cartOff();
  }

  void write369in1(uint8_t blockNumber)
  {
    uint8_t writeBuffer[1024];

    // 64 blocks at 4MB each
    uint32_t startBank = (((uint32_t)blockNumber * 4) / 32) * 0x2000000;
    uint32_t startBlock = ((uint32_t)blockNumber * 4 * 1024 * 1024) - startBank;
    uint32_t lastBlock = 0x2000000;
    if (fileSize < lastBlock)
      lastBlock = startBlock + fileSize;
    uint32_t lastSector = 0x400000;
    if (fileSize < lastSector)
      lastSector = fileSize;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    // 32MB max GBA bank size
    for (uint32_t currBank = startBank; currBank < startBank + fileSize; currBank += 0x2000000)
    {
      // 4MB minimum repro block size
      for (uint32_t currBlock = startBlock; currBlock < lastBlock; currBlock += 0x400000)
      {
        // Set-up 369-in-1 mapper
        mapBlock369in1((currBank + currBlock) / 1024 / 1024);

        // 256KB flashrom sector size
        for (uint32_t currSector = 0; currSector < lastSector; currSector += 0x40000)
        {
          // Unlock Sector
          //writeWord(currBlock + currSector, 0x60);
          //writeWord(currBlock + currSector, 0xD0);

          // 1024B writeBuffer
          for (uint32_t currWriteBuffer = 0; currWriteBuffer < 0x40000; currWriteBuffer += 1024)
          {
            // Fill writeBuffer from SD card
            OSCR::Storage::Shared::sharedFile.read(writeBuffer, 1024);

            // Buffered program command
            writeWord(currBlock + currSector + currWriteBuffer, 0xEA);

            // Check Status register
            uint16_t statusReg = readWord(currBlock + currSector + currWriteBuffer);
            while ((statusReg | 0xFF7F) != 0xFFFF) {
              statusReg = readWord(currBlock + currSector + currWriteBuffer);
            }

            // Write word count (minus 1)
            writeWord(currBlock + currSector + currWriteBuffer, 0x1FF);

            // Send writeBuffer to flashrom
            for (uint16_t currByte = 0; currByte < 1024; currByte += 2)
            {
              // Join two bytes into one word
              uint16_t currWord = ((writeBuffer[currByte + 1] & 0xFF) << 8) | (writeBuffer[currByte] & 0xFF);

              writeWord(currBlock + currSector + currWriteBuffer + currByte, currWord);
            }

            // Write buffer to flash
            writeWord(currBlock + currSector + currWriteBuffer + 1022, 0xD0);

            // Read the status register at last written address
            statusReg = readWord(currBlock + currSector + currWriteBuffer + 1022);
            while ((statusReg | 0xFF7F) != 0xFFFF) {
              statusReg = readWord(currBlock + currSector + currWriteBuffer + 1022);
            }
          }

          OSCR::UI::ProgressBar::advance(0x40000);
        }
      }
    }

    OSCR::UI::ProgressBar::finish();

    cartOff();
  }

  //******************************************
  // Flash Repro function
  //******************************************
  void flashRepro(bool option)
  {
    // Check flashrom ID's
    idFlashrom();

    if ((flashid == 0x8802) || (flashid == 0x8816) || (flashid == 0x227E) || (flashid == 0x8812))
    {
      uint8_t blockNum = 0;

      OSCR::UI::printLabel(OSCR::Strings::Common::ID);
      OSCR::UI::printHexLine(flashid);

      OSCR::UI::printSize(OSCR::Strings::Common::Flash, cartSize);

      // MX29GL128E or MSP55LV128(N)
      if (flashid == 0x227E)
      {
        // MX is 0xC2 and MSP55LV128 is 0x4 and MSP55LV128N 0x1
        if (romType == 0xC2)
        {
          OSCR::UI::printLine(F("Macronix MX29GL128E"));
        }
        else if ((romType == 0x1) || (romType == 0x4))
        {
          OSCR::UI::printLine(F("Fujitsu MSP55LV128N"));
        }
        else if ((romType == 0x89))
        {
          OSCR::UI::printLine(F("Intel PC28F256M29"));
        }
        else if ((romType == 0x20))
        {
          OSCR::UI::printLine(F("ST M29W128GH"));
        }
        else
        {
          OSCR::UI::printType(OSCR::Strings::Common::Flash);
          OSCR::UI::printHexLine(romType);
          OSCR::UI::fatalError(FS(OSCR::Strings::Errors::UnknownType));
        }
      }
      else if (flashid == 0x8802) // Intel 4000L0YBQ0
      {
        OSCR::UI::printLine(F("Intel 4000L0YBQ0"));
      }
      else if (flashid == 0x8816) // Intel 4400L0ZDQ0
      {
        OSCR::UI::printLine(F("Intel 4400L0ZDQ0"));
      }
      else if (flashid == 0x8812) // F0088H0
      {
        OSCR::UI::printLine(F("F0088H0"));
      }

      OSCR::UI::waitButton();

      if (!OSCR::Prompts::confirmErase()) return;

      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

      // Get rom size from file
      fileSize = OSCR::Storage::Shared::getSize();

      OSCR::UI::printSize(OSCR::Strings::Common::ROM, fileSize);

      // Erase needed sectors
      if (flashid == 0x8802)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));
        eraseIntel4000();
        resetIntel(0x200000);
      }
      else if (flashid == 0x8816)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));
        eraseIntel4400();
        resetIntel(0x200000);
      }
      else if (flashid == 0x8812)
      {
        if (option)
        {
          blockNum = selectBlockNumber(1);
          OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));
          erase369in1(blockNum);
        }
        else
        {
          OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));
          erase369in1(0);
        }

        // Reset or blankcheck will fail
        reset369in1();
      }
      else if (flashid == 0x227E)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        if ((romType == 0xC2) || (romType == 0x89) || (romType == 0x20))
        {
          //MX29GL128E
          //PC28F256M29 (0x89)
          sectorEraseMX29GL128E();
        }
        else if ((romType == 0x1) || (romType == 0x4))
        {
          //MSP55LV128(N)
          sectorEraseMSP55LV128();
        }
      }

      //Write flashrom

      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

      if ((flashid == 0x8802) || (flashid == 0x8816))
      {
        writeIntel4000();
      }
      else if (flashid == 0x8812)
      {
        write369in1(option ? blockNum : 0);
        reset369in1();
      }
      else if (flashid == 0x227E)
      {
        if ((romType == 0xC2) || (romType == 0x89) || (romType == 0x20))
        {
          //MX29GL128E (0xC2)
          //PC28F256M29 (0x89)
          writeMX29GL128E();
        }
        else if ((romType == 0x1) || (romType == 0x4))
        {
          //MSP55LV128(N)
          writeMSP55LV128();
        }
      }

      OSCR::Storage::Shared::close();

      if (flashid != 0x8812)
      {
        // Verify
        OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

        if (flashid == 0x8802)
        {
          // Don't know the correct size so just take some guesses
          resetIntel(0x8000);

          delay(1000);

          resetIntel(0x100000);

          delay(1000);

          resetIntel(0x200000);
        }
        else if (flashid == 0x8816)
        {
          resetIntel(0x200000);
        }
        else if (flashid == 0x8812)
        {
          reset369in1();
        }
        else if (flashid == 0x227E)
        {
          resetMX29GL128E();
        }

        delay(1000);

        if (verifyFlashrom() == 1)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
        }
        else
        {
          OSCR::UI::fatalError(FS(OSCR::Strings::Common::FAIL));
        }
      }
    }
    else
    {
      OSCR::UI::print(OSCR::Strings::Common::ID);
      OSCR::UI::printHexLine(flashid);

      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::UnknownType));
    }
  }
# endif /* HAS_FLASH */
}

#endif
