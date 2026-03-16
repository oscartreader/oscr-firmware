//******************************************
// Super A'can MODULE
// Only tested with HW3 and HW5
//******************************************
#include "config.h"

#if HAS_SUPRACAN
# include "cores/include.h"
# include "cores/SuperAcan.h"

namespace OSCR::Cores::SuperAcan
{
  using OSCR::Cores::MegaDrive::dataOut;
  using OSCR::Cores::MegaDrive::dataIn;

  /******************************************
    Menu
  *****************************************/
  constexpr char const PROGMEM menuOptionUM6650[] = "UM6650";

  constexpr char const * const PROGMEM menuOptionsUM6650[] = {
    OSCR::Strings::MenuOptions::Read,
    OSCR::Strings::MenuOptions::Write,
    OSCR::Strings::MenuOptions::Back,
  };

  uint8_t refreshCart(__FlashStringHelper const * menuOptions[], MenuOption * const menuOptionMap)
  {
    bool canFlash;
    uint8_t option = 0;

    while (!checkROM())
    {
      switch(OSCR::Prompts::abortRetryContinue())
      {
      case OSCR::Prompts::AbortRetryContinue::Abort:
        // Set the menu options to only be "Back" so we return to the main menu.
        menuOptions[option] = FS(OSCR::Strings::MenuOptions::Back);
        menuOptionMap[option] = MenuOption::Back;
        return 1;

      case OSCR::Prompts::AbortRetryContinue::Retry:    continue; // repeat the loop
      case OSCR::Prompts::AbortRetryContinue::Continue: break;    // exit the switch and then exit the loop
      }

      break; // Only reached when continue is selected above.
    }

    canFlash = (*((uint32_t *)(eepbit + 4)) > 0);

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadROM);
    menuOptionMap[option] = MenuOption::ReadROM;
    option++;

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadSave);
    menuOptionMap[option] = MenuOption::ReadSave;
    option++;

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::WriteSave);
    menuOptionMap[option] = MenuOption::WriteSave;
    option++;

    if (canFlash)
    {
      menuOptions[option] = FS(OSCR::Strings::MenuOptions::WriteFlash);
      menuOptionMap[option] = MenuOption::WriteFlash;
      option++;
    }

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::RefreshCart);
    menuOptionMap[option] = MenuOption::RefreshCart;
    option++;

    menuOptions[option] = FS(menuOptionUM6650);
    menuOptionMap[option] = MenuOption::UM6650;
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
    uint8_t menuOptionCount = refreshCart(menuOptions, menuOptionMap);

    do
    {
      // If there is only one option, automatically select it. Otherwise, create a menu as normal.
      uint8_t selected = (menuOptionCount == 1) ? 0 : OSCR::UI::menu(FS(OSCR::Strings::Cores::SuperAcan), menuOptions, menuOptionCount);

      switch (menuOptionMap[selected])
      {
      case MenuOption::ReadROM:
        readROM();
        break;

      case MenuOption::ReadSave:
        readSRAM();
        break;

      case MenuOption::WriteSave:
        writeSRAM();
        break;

      case MenuOption::WriteFlash:
        flashCart();
        // fall through
      case MenuOption::RefreshCart:
        menuOptionCount = refreshCart(menuOptions, menuOptionMap);
        break;

      case MenuOption::UM6650: // We use a submenu so we can make use of existing strings
        switch (OSCR::UI::menu(FS(menuOptionUM6650), menuOptionsUM6650, sizeofarray(menuOptionsUM6650)))
        {
        case 0: // UM6650 > Read
          readUM6650();
          break;

        case 1: // UM6650 > Write
          writeUM6650();
          break;

        case 2: // Back
          continue; // skips waitButton()
        }
        break;

      case MenuOption::Back:
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::SuperAcan));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // addr as output
    DDRF = 0xFF;  // A0 - A7
    DDRK = 0xFF;  // A8 - A15
    DDRL = 0xFF;  // A16 - A23

    // data as input
    DDRC = 0xFF;
    DDRA = 0xFF;
    PORTC = 0x00;  // disable internal pull-up
    PORTA = 0x00;
    DDRC = 0x00;  // D0 - D7
    DDRA = 0x00;  // D8 - D15

    // set /RST(PH0), /CS(PH3), C27(PH4), R/W(PH5), /AS(PH6) output
    DDRH |= ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
    PORTH |= ((1 << 3) | (1 << 5) | (1 << 6));
    PORTH &= ~((1 << 0) | (1 << 4));

    // set 6619_124(PE4) input
    DDRE &= ~(1 << 4);

    // detect if flash chip exists
    PORTG |= (1 << 5);
    DDRG |= (1 << 5);
    PORTG |= (1 << 5);

    writeCommand(0, 0x9090);

    eepbit[0] = readWord(0x2);
    eepbit[1] = readWord(0x0);

    writeWord(0x0, 0xF0F0);

    dataIn();

    // set /CARTIN(PG5) input
    PORTG &= ~(1 << 5);
    DDRG &= ~(1 << 5);

    *((uint32_t *)(eepbit + 4)) = getFlashChipSize(*((uint16_t *)eepbit));

#if defined(ENABLE_CLOCKGEN)
    if (!OSCR::ClockGen::initialize())
    {
      // Error out/halt the program (reset)
      //OSCR::UI::fatalError(FS(OSCR::Strings::Errors::ClockGenMissing));

      // Warn the user that there's an issue and allow them to continue.
      OSCR::UI::printNotificationHeader(FS(OSCR::Strings::Headings::HardwareProblem));
      OSCR::UI::notification(FS(OSCR::Strings::Errors::ClockGenMissing));
    }
    else
    {
      OSCR::ClockGen::clockgen.set_freq(1073863500ULL, SI5351_CLK1);  // cpu
      OSCR::ClockGen::clockgen.set_freq(357954500ULL, SI5351_CLK2);   // subcarrier
      OSCR::ClockGen::clockgen.set_freq(5369317500ULL, SI5351_CLK0);  // master clock

      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 1);
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK2, 1);
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK0, 0);

      // Wait for clock generator
      OSCR::ClockGen::clockgen.update_status();

      delay(500);
    }
#endif
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
    dataDir = DataDirection::Unknown;
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::SuperAcan));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  void writeCommand(uint32_t offset, uint16_t command)
  {
    writeWord(offset + 0xAAAA, 0xAAAA);
    writeWord(offset + 0x5555, 0x5555);
    writeWord(offset + 0xAAAA, command);
  }

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SuperAcan), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    OSCR::UI::ProgressBar::init((uint32_t)(cartSize));

    for (uint32_t addr = 0; addr < cartSize; addr += 512, OSCR::UI::ProgressBar::advance(512))
    {
      for (uint32_t i = 0; i < 512; i += 2)
      {
        *((uint16_t *)(OSCR::Storage::Shared::buffer + i)) = readWord(addr + i);
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::ProgressBar::finish();

    OSCR::UI::print(FS(OSCR::Strings::Labels::CRCSum));
    OSCR::UI::printLine(OSCR::Storage::Shared::getCRC32());
  }

  void readSRAM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SuperAcan), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    for (uint32_t i = 0; i < 0x10000; i += 1024)
    {
      for (uint32_t j = 0; j < 1024; j += 2)
      {
        OSCR::Storage::Shared::buffer[(j >> 1)] = readWord(0xEC0000 + i + j);
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void writeSRAM()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    cartOn();

    for (uint32_t i = 0; i < 0x10000 && OSCR::Storage::Shared::available(); i += 1024)
    {
      OSCR::Storage::Shared::fill();

      for (uint32_t j = 0; j < 1024; j += 2)
        writeWord(0xEC0000 + i + j, OSCR::Storage::Shared::buffer[(j >> 1)]);
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    OSCR::Storage::Shared::rewind();

    writeErrors = 0;

    for (uint32_t i = 0; i < 0x10000 && OSCR::Storage::Shared::available(); i += 1024)
    {
      OSCR::Storage::Shared::fill();

      for (uint32_t j = 0; j < 1024; j += 2)
      {
        if (readWord(0xEC0000 + i + j) != OSCR::Storage::Shared::buffer[(j >> 1)])
        {
          writeErrors++;
        }
      }
    }

    cartOff();

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

  void readUM6650()
  {
    printHeader();

    setOutName_P(menuOptionUM6650);

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SuperAcan), FS(menuOptionUM6650), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    for (uint16_t i = 0; i < 256; i++)
    {
      writeWord(0xEB0D03, i);

      OSCR::Storage::Shared::buffer[i] = readWord(0xEB0D01);
    }

    OSCR::Storage::Shared::writeBuffer(256);

    OSCR::Storage::Shared::rewind();

    printHeader();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    for (uint16_t i = 0, len = OSCR::Storage::Shared::readBuffer(256); i < len; i++)
    {
      writeWord(0xEB0D03, i);

      if (readWord(0xEB0D01) != OSCR::Storage::Shared::buffer[i])
      {
        writeErrors++;
      }
    }

    cartOff();

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

  void writeUM6650()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    cartOn();

    uint16_t len = OSCR::Storage::Shared::readBuffer(256);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    for (uint16_t i = 0; i < len; i++)
    {
      writeWord(0xEB0D03, i);
      writeWord(0xEB0D01, OSCR::Storage::Shared::buffer[i]);

      delay(10);  // for AT28C64B write
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void flashCart()
  {
    uint32_t * flash_size = (uint32_t *)(eepbit + 4);

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    cartOn();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    uint32_t i, j, k, file_length = OSCR::Storage::Shared::getSize();
    uint16_t data;

    DDRG |= (1 << 5);
    PORTG |= (1 << 5);

    OSCR::UI::ProgressBar::init((uint32_t)(file_length));

    for (i = 0; i < file_length; i += *flash_size)
    {
      // erase chip
      writeCommand(i, 0x8080);
      writeCommand(i, 0x1010);

      while (readWord(i) != 0xFFFF);

      for (j = 0; j < *flash_size; j += 512)
      {
        OSCR::Storage::Shared::fill();

        for (k = 0; k < 512; k += 2)
        {
          data = *((uint16_t *)(OSCR::Storage::Shared::buffer + k));

          writeCommand(i, 0xA0A0);
          writeWord(i + j + k, data);

          while (readWord(i + j + k) != data);
        }

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::ProgressBar::finish();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  bool checkROM()
  {
    cartOn();

    // RST to 0
    PORTH &= ~(1 << 0);
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // /RST to 1
    PORTH |= (1 << 0);

    cartSize = getRomSize();
    romSize = cartSize >> 17;

    cartOff();

    return (cartSize > 0);
  }

  uint32_t getRomSize()
  {
    uint32_t addr = 0;
    crc32_t crc32;

    do
    {
      crc32.reset();

      for (uint32_t i = 0x2000; i < 0x2080; i += 2)
      {
        *((uint16_t *)OSCR::Storage::Shared::buffer) = readWord(addr + i);

        crc32 += OSCR::Storage::Shared::buffer[0];
        crc32 += OSCR::Storage::Shared::buffer[1];
      }

      crc32.done();

      if (crc32 == 0xA2BC9D7A)
      {
        if (addr > 0) break;
      }
      else
      {
        if (addr == 0) break;
      }

      addr += 0x80000;
    }
    while (addr < 0x800000);

    return addr;
  }

  void resetCart()
  {
    // set /CS(PH3), R/W(PH5), /AS(PH6) high
    // /RST(PH0) and C27(PH4) low
    PORTH |= ((1 << 3) | (1 << 5) | (1 << 6));
    PORTH &= ~((1 << 0) | (1 << 4));

#if defined(ENABLE_CLOCKGEN)
    if (OSCR::ClockGen::exists())
    {
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 0);  // CPU clock
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK2, 0);  // CIC clock
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK0, 0);  // master clock
    }
#endif
  }

  void writeWord(uint32_t addr, uint16_t data)
  {
    uint8_t *ptr = (uint8_t *)&addr;

    dataOut();

    PORTF = *ptr++;
    PORTK = *ptr++;
    PORTL = *ptr;

    if (*ptr < 0xE0)
    {
      // ROM area
      // /CS(PH3), C27(PH4), R/W(PH5), /AS(PH6) to L
      PORTH &= ~((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
    }
    else if (*ptr == 0xEC)
    {
      // save area
      // /CS(PH3) to H, C27(PH4), R/W(PH5), /AS(PH6) to L
      PORTH |= (1 << 3);
      PORTH &= ~((1 << 4) | (1 << 5) | (1 << 6));
    }
    else if (addr == 0x00EB0D03 || addr == 0x00EB0D01)
    {
      // UM6650 area
      // /CS(PH3), C27(PH4) to H, R/W(PH5), /AS(PH6) to L
      PORTH |= ((1 << 3) | (1 << 4));
      PORTH &= ~((1 << 5) | (1 << 6));
    }

    ptr = (uint8_t *)&data;
    PORTC = *ptr++;
    PORTA = *ptr;

    NOP;
    NOP;
    NOP;

    PORTH &= ~(1 << 4);
    PORTH |= ((1 << 3) | (1 << 5) | (1 << 6));
  }

  uint16_t readWord(uint32_t addr)
  {
    uint8_t *ptr = (uint8_t *)&addr;
    uint16_t data;

    dataIn();

    PORTF = *ptr++;
    PORTK = *ptr++;
    PORTL = *ptr;

    if (*ptr < 0xE0)
    {
      // ROM area
      // /CS(PH3), C27(PH4), /AS(PH6) to L
      PORTH &= ~((1 << 3) | (1 << 4) | (1 << 6));
    }
    else if (*ptr == 0xEC)
    {
      // save area
      // /CS(PH3) to H, C27(PH4), /AS(PH6) to L
      PORTH |= (1 << 3);
      PORTH &= ~((1 << 4) | (1 << 6));
    }
    else if (addr == 0x00EB0D03 || addr == 0x00EB0D01)
    {
      // UM6650 area
      // /CS(PH3), C27(PH4) to H, /AS(PH6) to L
      PORTH |= ((1 << 3) | (1 << 4));
      PORTH &= ~(1 << 6);
    }

    ptr = (uint8_t *)&data;
    NOP;
    NOP;
    NOP;

    *ptr++ = PINC;
    *ptr = PINA;

    PORTH &= ~(1 << 4);
    PORTH |= ((1 << 3) | (1 << 5) | (1 << 6));

    return data;
  }

  uint32_t getFlashChipSize(uint16_t chip_id)
  {
    // 0x0458 (8M), 0x01AB (4M), 0x01D8 (16M)
    switch (chip_id)
    {
    case 0x01AB:  return 524288;
    case 0x0458:  return 1048576;
    case 0x01D8:  return 2097152;
    default:      return 0;
    }
  }
} /* namespace OSCR::Cores::SuperAcan */

#endif /* HAS_SUPRACAN */
