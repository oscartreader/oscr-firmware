//******************************************
// GAME BOY MODULE
//******************************************
#include "config.h"

#if HAS_GBX
# include "cores/include.h"
# include "cores/GameBoy.h"
# include "cores/GameBoyAdvance.h"
# include "cores/GameBoyMem.h"
# include "cores/GameBoySmart.h"
# include "cores/Flash.h"

namespace OSCR::Cores
{
  namespace GameBoyX
  {
    /******************************************
       Variables
    *****************************************/

    /******************************************
       Menu
    *****************************************/
    // GBx start menu
    constexpr char const PROGMEM gbxMenuItem3[] = "Flash Repro";
    constexpr char const PROGMEM gbxMenuItem4[] = "NPower GB Memory";
    constexpr char const PROGMEM gbxMenuItem5[] = "Flash Codebreaker";
    constexpr char const PROGMEM gbxMenuItem6[] = "Flash Datel Device";

    constexpr char const * const PROGMEM menuOptionsGBx[] = {
      OSCR::Strings::Cores::GameBoy,
      OSCR::Strings::Cores::GameBoyAdvance,
      gbxMenuItem3,
      gbxMenuItem4,
      gbxMenuItem5,
      gbxMenuItem6,
      OSCR::Strings::MenuOptions::Back,
    };

# if HAS_FLASH
    // GB Flash items
    constexpr char const PROGMEM GBFlashItem1[] = "GB 29F/39SF Repro";
    constexpr char const PROGMEM GBFlashItem2[] = "GB CFI Repro";
    constexpr char const PROGMEM GBFlashItem3[] = "GB CFI and Save";
    constexpr char const PROGMEM GBFlashItem4[] = "GB Smart";
    constexpr char const PROGMEM GBFlashItem5[] = "GBA Repro (3V)";
    constexpr char const PROGMEM GBFlashItem6[] = "GBA 369-in-1 (3V)";

    constexpr char const * const PROGMEM menuOptionsGBFlash[] = {
      GBFlashItem1,
      GBFlashItem2,
      GBFlashItem3,
      GBFlashItem4,
      GBFlashItem5,
      GBFlashItem6,
      OSCR::Strings::MenuOptions::Back,
    };

    // 29F Flash items
    constexpr char const PROGMEM GBFlash29Item1[] = "DIY MBC3 (WR)";
    constexpr char const PROGMEM GBFlash29Item2[] = "DIY MBC5 (WR)";
    constexpr char const PROGMEM GBFlash29Item3[] = "HDR MBC30 (Audio)";
    constexpr char const PROGMEM GBFlash29Item4[] = "HDR GameBoy Cam";
    constexpr char const PROGMEM GBFlash29Item5[] = "Orange FM (WR)";
    constexpr char const PROGMEM GBFlash29Item6[] = "39SF MBC5 (Audio)";

    constexpr char const * const PROGMEM menuOptionsGBFlash29[] = {
      GBFlash29Item1,
      GBFlash29Item2,
      GBFlash29Item3,
      GBFlash29Item4,
      GBFlash29Item5,
      GBFlash29Item6,
      OSCR::Strings::MenuOptions::Back,
    };
# endif /* HAS_FLASH */

    // Pelican Codebreaker, Brainboy, and Monster Brain Operation Menu
    constexpr char const * const PROGMEM menuOptionsGBPelican[] = {
      OSCR::Strings::MenuOptions::Read,
      OSCR::Strings::MenuOptions::Write,
    };

    // Datel Device Operation Menu
    constexpr char const PROGMEM MegaMemRead[] = "Read Mega Memory";
    constexpr char const PROGMEM MegaMemWrite[] = "Write Mega Memory";
    constexpr char const PROGMEM GameSharkRead[] = "Read GBC GameShark";
    constexpr char const PROGMEM GameSharkWrite[] = "Write GBC GameShark";
    constexpr char const * const PROGMEM menuOptionsGBDatel[] = { MegaMemRead, MegaMemWrite, GameSharkRead, GameSharkWrite };

    constexpr char const PROGMEM TemplateMBC[] = "MBC%" PRIu8;

# if HAS_FLASH
    bool flashCFI()
    {
      // Flash CFI
      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

      if (!OSCR::Cores::GameBoy::identifyCFI()) return false;

      if (!OSCR::Cores::GameBoy::writeCFI()) return false;

      if (!OSCR::Cores::GameBoy::checkCart()) return false;

      OSCR::Cores::GameBoy::cartOff();

      return true;
    }
# endif /* HAS_FLASH */

    // Start menu for both GB and GBA
    void menu()
    {
      do
      {
        OSCR::Cores::GameBoy::audioWE = 0;

        switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::GameBoy), menuOptionsGBx, sizeofarray(menuOptionsGBx)))
        {
        case 0:
          OSCR::Cores::GameBoy::menu();
          continue;

        case 1:
          OSCR::Cores::GameBoyAdvance::menu();
          continue;

# if HAS_FLASH
        case 2:
          menuFlash();
          continue;
# endif /* HAS_FLASH */

        case 3: // Flash GB Memory
          OSCR::Cores::GameBoyMem::menu();
          continue;

        case 4:
          menuPelican();
          continue;

        case 5:
          menuDatel();
          continue;

        case 6:
          return;
        }

        //OSCR::UI::waitButton();
      }
      while(true);
    }

    void menuPelican()
    {
      do
      {
        switch (OSCR::UI::menu(FS(gbxMenuItem5), menuOptionsGBPelican, sizeofarray(menuOptionsGBPelican)))
        {
        case 0:
          OSCR::Cores::GameBoy::readPelican();
          break;

        case 1:
          OSCR::Cores::GameBoy::writePelican();
          break;
        }

        OSCR::UI::waitButton();
      }
      while(true);
    }

    void menuDatel()
    {
      do
      {
        switch (OSCR::UI::menu(FS(gbxMenuItem6), menuOptionsGBDatel, sizeofarray(menuOptionsGBDatel)))
        {
        case 0:
          OSCR::Cores::GameBoy::readMegaMem();
          break;

        case 1:
          OSCR::Cores::GameBoy::writeMegaMem();
          break;

        case 2:
          OSCR::Cores::GameBoy::readGameshark();
          break;

        case 3:
          OSCR::Cores::GameBoy::writeGameshark();
          break;

        case 4:
          return;
        }

        OSCR::UI::waitButton();
      }
      while(true);
    }

# if HAS_FLASH
    void menuFlash()
    {
      do
      {
        switch (OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetCartType), menuOptionsGBFlash, sizeofarray(menuOptionsGBFlash)))
        {
        case 0:
          menu29F();
          continue;

        case 1:
          // Flash CFI
          if (!flashCFI()) continue;
          break;

        case 2:
          // Flash CFI and save
          if (!flashCFI()) continue;

          // Does cartridge have SRAM
          if (OSCR::Cores::GameBoy::lastByteAddress > 0)
          {
            if (!OSCR::Prompts::askYesNo(OSCR::Strings::MenuOptions::WriteSave)) continue;

            OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

            OSCR::Cores::GameBoy::writeSRAM();
            continue;
          }

          OSCR::UI::error(FS(OSCR::Strings::Errors::NoSave));
          break;

        case 3: // Flash GB Smart
          OSCR::Cores::GameBoySmart::menu();
          continue;

        case 4: // Flash GBA Repro
          OSCR::Cores::GameBoyAdvance::reproMenu();
          break; // has no waitButton();

        case 5:
          // Read/Write GBA 369-in-1 Repro
          OSCR::Cores::GameBoyAdvance::repro369in1Menu();
          break;

        case 6:
          return;
        }
      }
      while (true);
    }

    void menu29F()
    {
      do
      {
        switch (OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetCartType), menuOptionsGBFlash29, sizeofarray(menuOptionsGBFlash29)))
        {
        case 0: //Flash MBC3
          //MBC3, standard command set, with erase
          OSCR::Cores::GameBoy::writeFlash(3, 0, 1);
          break;

        case 1: //Flash MBC5
          //MBC5, standard command set, with erase
          OSCR::Cores::GameBoy::writeFlash(5, 0, 1);
          break;

        case 2: // Flash MBC3 with flash WE connected to audio pin

          //Tell OSCR::Cores::GameBoy::writeByte function to pulse Audio-In
          OSCR::Cores::GameBoy::audioWE = 1;

          //MBC3, standard command set, with erase
          OSCR::Cores::GameBoy::writeFlash(3, 0, 1);
          break;

        case 3: //Flash GB Camera
          //Flash first bank with erase
          //MBC3, standard command set, with erase
          OSCR::Cores::GameBoy::writeFlash(3, 0, 1);

          OSCR::UI::waitButton();

          if (!OSCR::Prompts::askYesNo(PSTR("Flash second game?"))) continue;

          //MBC3, standard command set, without erase
          OSCR::Cores::GameBoy::writeFlash(3, 0, 0);
          break;

        case 4: //Flash 39SF010 cart without MBC
          // No MBC, 39SF040 command set, with erase
          OSCR::Cores::GameBoy::writeFlash(0, 1, 1);
          break;

        case 5: //Flash MBC5 cart with 39SF040 and WE on Audio
          //Tell OSCR::Cores::GameBoy::writeByte function to pulse Audio-In
          OSCR::Cores::GameBoy::audioWE = 1;

          //MBC5, 39SF040 command set, with erase
          OSCR::Cores::GameBoy::writeFlash(5, 1, 1);
          break;

        case 6:
          return;
        }

        OSCR::UI::waitButton();
      }
      while (true);
    }

# endif /* HAS_FLASH */

  } /* namespace OSCR::Cores::GameBoyX */


  namespace GameBoy
  {
    // Game Boy
    uint16_t sramBanks;
    uint16_t romBanks;
    uint16_t lastByteAddress = 0;
    bool audioWE = 0;

    using OSCR::Cores::Flash::flashid;

# if HAS_FLASH
    using OSCR::Cores::Flash::flashSwitchLastBits;
    using OSCR::Cores::Flash::flashX16Mode;
    using OSCR::Cores::Flash::flashBanks;
# endif /* HAS_FLASH */

    void printHeader()
    {
      OSCR::UI::printHeader(FS(OSCR::Strings::Cores::GameBoy));
    }

    // Common code for setting up GB port used by GB carts, Datel,
    // Pelican Codebreaker, MonsterBrain
    void setupPort()
    {
      // Set Address Pins to Output
      //A0-A7
      DDRF = 0xFF;
      //A8-A15
      DDRK = 0xFF;

      // Set Control Pins to Output RST(PH0) CLK(PH1) CS(PH3) WR(PH5) RD(PH6)
      DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 6);
      // Output a high signal on all pins, pins are active low therefore everything is disabled now
      PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
      // Output a low signal on CLK(PH1) to disable writing GB Camera RAM
      // Output a low signal on RST(PH0) to initialize MMC correctly
      PORTH &= ~((1 << 0) | (1 << 1));

      /* FIXME setup sets this up too. Should this also for Datel and others?
        // Set Audio-In(PH4) to Input
        DDRH &= ~(1 << 4);
        // Enable Internal Pullup
        PORTH |= (1 << 4);
      */

      // Set Data Pins (D0-D7) to Input
      DDRC = 0x00;
      // Enable Internal Pullups
      PORTC = 0xFF;
    }

    uint8_t refreshCart(__FlashStringHelper const * menuOptions[], MenuOption * const menuOptionMap)
    {
      uint8_t option = 0;

      while (!checkCart())
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

      printCartInfo();

      menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadROM);
      menuOptionMap[option] = MenuOption::ReadROM;
      option++;

      if (lastByteAddress)
      {
        menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadSave);
        menuOptionMap[option] = MenuOption::ReadSave;
        option++;

        menuOptions[option] = FS(OSCR::Strings::MenuOptions::WriteSave);
        menuOptionMap[option] = MenuOption::WriteSave;
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
      __FlashStringHelper const * menuOptions[kMenuOptionMax] = {};
      MenuOption menuOptionMap[kMenuOptionMax] = {};
      uint8_t menuOptionCount = refreshCart(menuOptions, menuOptionMap);

      openCRDB();

      do
      {
        // If there is only one option, automatically select it. Otherwise, create a menu as normal.
        uint8_t selected = (menuOptionCount == 1) ? 0 : OSCR::UI::menu(FS(OSCR::Strings::Cores::GameBoy), menuOptions, menuOptionCount);

        switch (menuOptionMap[selected])
        {
        case MenuOption::ReadROM:
          if (!readROM()) continue;
          break;

        case MenuOption::ReadSave:
          if (!readSave()) continue;
          break;

        case MenuOption::WriteSave:
          if (!writeSave()) continue;
          break;

        case MenuOption::RefreshCart:
          menuOptionCount = refreshCart(menuOptions, menuOptionMap);
          break;

        case MenuOption::Back:
          closeCRDB();
          return;
        }

        OSCR::UI::waitButton();
      }
      while (true);
    }

    /******************************************
       Setup
    *****************************************/
    void cartOn()
    {
      // Request 5V
      OSCR::Power::setVoltage(OSCR::Voltage::k5V);
      OSCR::Power::enableCartridge();

      setupPort();

      // FIXME for now setupPort doesn't set these ones up
      // Set Audio-In(PH4) to Input
      DDRH &= ~(1 << 4);
      // Enable Internal Pullup
      PORTH |= (1 << 4);

      delay(400);

      // RST(PH0) to HIGH
      PORTH |= (1 << 0);

      if (audioWE)
      {
        //Setup Audio-In(PH4) as Output
        DDRH |= (1 << 4);

        // Output a high signal on Audio-In(PH4)
        PORTH |= (1 << 4);
      }

      // MMM01 initialize
      if (romType >= 11 && romType <= 13)
      {
        writeByte(0x3FFF, 0x00);
        writeByte(0x5FFF, 0x40);
        writeByte(0x7FFF, 0x01);
        writeByte(0x1FFF, 0x3A);
        writeByte(0x1FFF, 0x7A);
      }
    }

    void cartOff()
    {
      OSCR::Power::disableCartridge();
    }

    void openCRDB()
    {
      OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::GameBoy));
    }

    void closeCRDB()
    {
      resetGlobals();
    }

    void printCartInfo()
    {
      printHeader();

      if (checksum != 0)
      {
        OSCR::UI::printValue(OSCR::Strings::Common::Name, fileName);

        if (cartID[0] != 0)
        {
          OSCR::UI::printValue(OSCR::Strings::Common::ID, cartID);
        }

        OSCR::UI::printValue(OSCR::Strings::Common::Revision, romVersion);

        char const * romTypeName = nullptr;
        uint8_t mbc = 0;

        switch (romType)
        {
        case 0:
        case 8:
        case 9:
          romTypeName = OSCR::Strings::Common::None;
          break;

        case 1:
        case 2:
        case 3:
          romTypeName = OSCR::Cores::GameBoyX::TemplateMBC;
          mbc = 1;
          break;

        case 5:
        case 6:
          romTypeName = OSCR::Cores::GameBoyX::TemplateMBC;
          mbc = 2;
          break;

        case 11:
        case 12:
        case 13:
          romTypeName = PSTR("MMM01");
          break;

        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
          romTypeName = OSCR::Cores::GameBoyX::TemplateMBC;
          mbc = 3;
          break;

        case 21:
        case 22:
        case 23:
          romTypeName = OSCR::Cores::GameBoyX::TemplateMBC;
          mbc = 4;
          break;

        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 309:
          romTypeName = OSCR::Cores::GameBoyX::TemplateMBC;
          mbc = 5;
          break;

        case 32:
          romTypeName = OSCR::Cores::GameBoyX::TemplateMBC;
          mbc = 6;
          break;

        case 34:
          romTypeName = OSCR::Cores::GameBoyX::TemplateMBC;
          mbc = 7;
          break;

        case 252:
          romTypeName = PSTR("Camera");
          break;

        case 253:
          romTypeName = PSTR("TAMA5");
          break;

        case 254:
          romTypeName = PSTR("HuC-3");
          break;

        case 255:
          romTypeName = PSTR("HuC-1");
          break;

        // --

        case 0x101:
        case 0x103:
          romTypeName = PSTR("MBC1M");
          break;

        case 0x104:
          romTypeName = PSTR("M161");
          break;
        }

        if (romTypeName != nullptr)
        {
          OSCR::UI::printLabel(OSCR::Strings::Common::Mapper);

          if (romTypeName == OSCR::Cores::GameBoyX::TemplateMBC)
          {
            char buff[5];

            snprintf_P(BUFFN(buff), OSCR::Cores::GameBoyX::TemplateMBC, mbc);

            OSCR::UI::printLine(buff);
          }
          else
          {
            OSCR::UI::printLine(FS(romTypeName));
          }
        }

        OSCR::UI::printSize(OSCR::Strings::Common::ROM, (uint32_t)((1UL << (5UL + romSize)) * 1024UL));

        uint32_t sramSizeBytes = 0;

        switch (sramSize)
        {
        case 0:
          if (romType == 6)
          {
            sramSizeBytes = 512;
          }
          else if (romType == 0x22)
          {
            sramSizeBytes = lastByteAddress;
          }
          else if (romType == 0xFD)
          {
            sramSizeBytes = 32;
          }
          else
          {
            sramSizeBytes = 0;
          }
          break;

        case 1:
          sramSizeBytes = 2UL * 1024UL;
          break;

        case 2:
          sramSizeBytes = 8UL * 1024UL;
          break;

        case 3:
          if (romType == 0x20)
          {
            sramSizeBytes = 1079296;
            //OSCR::UI::print(F("1.03"));
            //OSCR::UI::print(FS(OSCR::Strings::Units::MB));
          }
          else
          {
            sramSizeBytes = 32UL * 1024UL;
          }
          break;

        case 4:
          sramSizeBytes = 128UL * 1024UL;
          break;

        case 5:
          sramSizeBytes = 64UL * 1024UL;
          break;

        default:
          sramSizeBytes = 0;
          break;
        }

        if (sramSizeBytes > 0)
        {
          OSCR::UI::printSize(OSCR::Strings::Common::Save, sramSizeBytes, true);
        }

        OSCR::UI::waitButton();
      }
      else
      {
        OSCR::UI::fatalError(FS(OSCR::Strings::Headings::CartridgeError));
      }
    }

    /******************************************
      Low level functions
    *****************************************/
    uint8_t readByte(uint16_t myAddress)
    {
      // Set address
      PORTF = myAddress & 0xFF;
      PORTK = (myAddress >> 8) & 0xFF;
      // Switch data pins to input
      DDRC = 0x00;
      // Enable pullups
      PORTC = 0xFF;

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Switch RD(PH6) to LOW
      PORTH &= ~(1 << 6);

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Read
      uint8_t tempByte = PINC;

      // Switch and RD(PH6) to HIGH
      PORTH |= (1 << 6);

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      return tempByte;
    }

    void writeByte(int myAddress, uint8_t myData)
    {
      writeByte(myAddress, myData, 0);
    }

    void writeByte(int myAddress, uint8_t myData, bool audio_as_WE)
    {
      DDRC = 0xFF;

      // Set address
      PORTF = myAddress & 0xFF;
      PORTK = (myAddress >> 8) & 0xFF;
      // Set data
      PORTC = myData;
      // Switch data pins to output
      DDRC = 0xFF;

      // Wait till output is stable
      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      if (audio_as_WE) // Pull Audio-In(PH4) low
      {
        PORTH &= ~(1 << 4);
      }
      else // Pull WR(PH5) low
      {
        PORTH &= ~(1 << 5);
      }

      // Leave WR low for at least 60ns
      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      if (audio_as_WE) // Pull Audio-In(PH4) HIGH
      {
        PORTH |= (1 << 4);
      }
      else // Pull WR(PH5) HIGH
      {
        PORTH |= (1 << 5);
      }

      // Leave WR high for at least 50ns
      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Switch data pins to input
      DDRC = 0x00;
      // Enable pullups
      PORTC = 0xFF;
    }

    // Triggers CS and CLK pin
    uint8_t readByteSRAM(uint16_t myAddress)
    {
      PORTF = myAddress & 0xFF;
      PORTK = (myAddress >> 8) & 0xFF;
      // Switch data pins to input
      DDRC = 0x00;
      // Enable pullups
      PORTC = 0xFF;

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Pull CS(PH3) CLK(PH1)(for FRAM MOD) LOW
      PORTH &= ~((1 << 3) | (1 << 1));
      // Pull RD(PH6) LOW
      PORTH &= ~(1 << 6);

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Read
      uint8_t tempByte = PINC;

      // Pull RD(PH6) HIGH
      PORTH |= (1 << 6);
      if (romType == 252) {
        // Pull CS(PH3) HIGH
        PORTH |= (1 << 3);
      } else {
        // Pull CS(PH3) CLK(PH1)(for FRAM MOD) HIGH
        PORTH |= (1 << 3) | (1 << 1);
      }

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      return tempByte;
    }

    // Triggers CS and CLK pin
    void writeByteSRAM(int myAddress, uint8_t myData)
    {
      // Set address
      PORTF = myAddress & 0xFF;
      PORTK = (myAddress >> 8) & 0xFF;
      // Set data
      PORTC = myData;
      // Switch data pins to output
      DDRC = 0xFF;

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      if (romType == 252 || romType == 253) {
        // Pull CS(PH3) LOW
        PORTH &= ~(1 << 3);
        // Pull CLK(PH1)(for GB CAM) HIGH
        PORTH |= (1 << 1);
        // Pull WR(PH5) low
        PORTH &= ~(1 << 5);
      } else {
        // Pull CS(PH3) CLK(PH1)(for FRAM MOD) LOW
        PORTH &= ~((1 << 3) | (1 << 1));
        // Pull WR(PH5) low
        PORTH &= ~(1 << 5);
      }

      // Leave WR low for at least 60ns
      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      if (romType == 252 || romType == 253)
      {
        // Pull WR(PH5) HIGH
        PORTH |= (1 << 5);
        // Pull CS(PH3) HIGH
        PORTH |= (1 << 3);
        // Pull  CLK(PH1) LOW (for GB CAM)
        PORTH &= ~(1 << 1);
      }
      else
      {
        // Pull WR(PH5) HIGH
        PORTH |= (1 << 5);
        // Pull CS(PH3) CLK(PH1)(for FRAM MOD) HIGH
        PORTH |= (1 << 3) | (1 << 1);
      }

      // Leave WR high for at least 50ns
      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Switch data pins to input
      DDRC = 0x00;
      // Enable pullups
      PORTC = 0xFF;
    }

    /******************************************
      Game Boy functions
    *****************************************/
    // Read Cartridge Header
    bool checkCart()
    {
      do
      {
        cartOn();

        // Read Header into array
        for (uint16_t currByte = 0x100; currByte < 0x150; currByte++)
        {
          OSCR::Storage::Shared::buffer[currByte] = readByte(currByte);
        }

        // Calculate header checksum
        uint8_t headerChecksum = 0;

        for (uint16_t currByte = 0x134; currByte < 0x14D; currByte++)
        {
          headerChecksum = headerChecksum - OSCR::Storage::Shared::buffer[currByte] - 1;
        }

        if (headerChecksum != OSCR::Storage::Shared::buffer[0x14D])
        {
          // Read Header into array a second time
          for (uint16_t currByte = 0x100; currByte < 0x150; currByte++)
          {
            OSCR::Storage::Shared::buffer[currByte] = readByte(currByte);
          }

          // Calculate header checksum a second time
          headerChecksum = 0;

          for (uint16_t currByte = 0x134; currByte < 0x14D; currByte++)
          {
            headerChecksum = headerChecksum - OSCR::Storage::Shared::buffer[currByte] - 1;
          }
        }

        if (headerChecksum != OSCR::Storage::Shared::buffer[0x14D])
        {
          cartOff();

          switch(OSCR::Prompts::abortRetryContinue())
          {
          case OSCR::Prompts::AbortRetryContinue::Abort: return false; // go back to main menu
          case OSCR::Prompts::AbortRetryContinue::Retry: continue; // try again

          case OSCR::Prompts::AbortRetryContinue::Continue: // ignore error and continue
            cartOn(); // Turn the cart back on
            break;
          }
        }
        break;
      }
      while (true);

      romType = OSCR::Storage::Shared::buffer[0x147];
      romSize = OSCR::Storage::Shared::buffer[0x148];
      sramSize = OSCR::Storage::Shared::buffer[0x149];

      // Get Checksum as string
      eepbit[6] = OSCR::Storage::Shared::buffer[0x14E];
      eepbit[7] = OSCR::Storage::Shared::buffer[0x14F];

      checksum = (eepbit[6] << 8) | (eepbit[7]);

      // ROM banks
      romBanks = 2;
      if (romSize >= 0x01 && romSize <= 0x08)
      {
        romBanks = OSCR::Util::power<2>(romSize + 1);
      }

      // SRAM banks
      sramBanks = 0;
      if (romType == 6)
      {
        sramBanks = 1;
      }

      // SRAM size
      switch (sramSize)
      {
      case 2:
        sramBanks = 1;
        break;

      case 3:
        sramBanks = 4;
        break;

      case 4:
        sramBanks = 16;
        break;

      case 5:
        sramBanks = 8;
        break;
      }

      // Last byte of SRAM
      if (romType == 6)
      {
        lastByteAddress = 0xA1FF;
      }

      if (sramSize == 1)
      {
        lastByteAddress = 0xA7FF;
      }
      else if (sramSize > 1)
      {
        lastByteAddress = 0xBFFF;
      }

      // MBC6
      if (romType == 32)
      {
        sramBanks = 8;
        lastByteAddress = 0xAFFF;
      }
      else if (romType == 34) // MBC7
      {
        lastByteAddress = (*((uint16_t*)(eepbit + 6)) == 0xA5BE ? 512 : 256);  // Only "Command Master" use LC66 EEPROM
      }

      // Get name
      uint8_t myByte = 0;
      uint8_t myLength = 0;
      uint8_t x = 0;

      if (OSCR::Storage::Shared::buffer[0x143] == 0x80 || OSCR::Storage::Shared::buffer[0x143] == 0xC0)
      {
        x++;
      }

      for (int addr = 0x0134; addr <= 0x0143 - x; addr++)
      {
        myByte = OSCR::Storage::Shared::buffer[addr];

        if (isprint(myByte) && myByte != '<' && myByte != '>' && myByte != ':' && myByte != '"' && myByte != '/' && myByte != '\\' && myByte != '|' && myByte != '?' && myByte != '*')
        {
          fileName[myLength++] = char(myByte);
        }
        else if (myLength == 0 || fileName[myLength - 1] != '_')
        {
          fileName[myLength++] = '_';
        }
      }

      // Find Game Serial
      cartID[0] = 0;
      if (OSCR::Storage::Shared::buffer[0x143] == 0x80 || OSCR::Storage::Shared::buffer[0x143] == 0xC0)
      {
        if ((fileName[myLength - 4] == 'A' || fileName[myLength - 4] == 'B' || fileName[myLength - 4] == 'H' || fileName[myLength - 4] == 'K' || fileName[myLength - 4] == 'V') && (fileName[myLength - 1] == 'A' || fileName[myLength - 1] == 'B' || fileName[myLength - 1] == 'D' || fileName[myLength - 1] == 'E' || fileName[myLength - 1] == 'F' || fileName[myLength - 1] == 'I' || fileName[myLength - 1] == 'J' || fileName[myLength - 1] == 'K' || fileName[myLength - 1] == 'P' || fileName[myLength - 1] == 'S' || fileName[myLength - 1] == 'U' || fileName[myLength - 1] == 'X' || fileName[myLength - 1] == 'Y'))
        {
          cartID[0] = fileName[myLength - 4];
          cartID[1] = fileName[myLength - 3];
          cartID[2] = fileName[myLength - 2];
          cartID[3] = fileName[myLength - 1];
          myLength -= 4;
          fileName[myLength] = 0;
        }
      }

      // Strip trailing white space
      while (myLength && (fileName[myLength - 1] == '_' || fileName[myLength - 1] == ' '))
      {
        myLength--;
      }

      fileName[myLength] = 0;

      // M161 (Mani 4 in 1)
      if (strncmp_P(fileName, PSTR("TETRIS SET"), 10) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0x3F)
      {
        romType = 0x104;
      }

      // MMM01 (Mani 4 in 1)
      if (
        (strncmp_P(fileName, PSTR("BOUKENJIMA2 SET"), 15) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0) ||
        (strncmp_P(fileName, PSTR("BUBBLEBOBBLE SET"), 16) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0xC6) ||
        (strncmp_P(fileName, PSTR("GANBARUGA SET"), 13) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0x90) ||
        (strncmp_P(fileName, PSTR("RTYPE 2 SET"), 11) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0x32)
      )
      {
        romType = 0x0B;
      }

      // MBC1M
      if (
          (strncmp_P(fileName, PSTR("MOMOCOL"), 7) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0x28) ||
          (strncmp_P(fileName, PSTR("BOMCOL"), 6) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0x86) ||
          (strncmp_P(fileName, PSTR("GENCOL"), 6) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0x8A) ||
          (strncmp_P(fileName, PSTR("SUPERCHINESE 123"), 16) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0xE4) ||
          (strncmp_P(fileName, PSTR("MORTALKOMBATI&II"), 16) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0xB9) ||
          (strncmp_P(fileName, PSTR("MORTALKOMBAT DUO"), 16) == 0 && OSCR::Storage::Shared::buffer[0x14D] == 0xA7)
        )
        {
        romType += 0x100;
      }

      // ROM revision
      romVersion = OSCR::Storage::Shared::buffer[0x14C];

      cartOff();

      return true;
    }

    /******************************************
      ROM functions
    *****************************************/
    // Read ROM
    bool readROM()
    {
      uint16_t endAddress = 0x7FFF;
      uint16_t romAddress = 0;
      uint16_t startBank = 1;

      printCartInfo();

      cartOn();

      printHeader();

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoy), FS(OSCR::Strings::Directory::ROM), fileName);

      OSCR::UI::ProgressBar::init(((uint32_t)romBanks)*16384, 1);

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

      // M161 banks are double size and start with 0
      if (romType == 0x104)
      {
        startBank = 0;
        romBanks >>= 1;
        endAddress = 0x7FFF;
      }
      else if (romType == 32) // MBC6 banks are half size
      {
        romBanks <<= 1;
        endAddress = 0x3FFF;
      }

      for (uint16_t currBank = startBank; currBank < romBanks; currBank++)
      {
        // Second bank starts at 0x4000
        if (currBank > 1)
        {
          romAddress = 0x4000;

          // MBC6 banks are half size
          if (romType == 32)
          {
            endAddress = 0x5FFF;
          }
        }

        // Set ROM bank for M161
        if (romType == 0x104)
        {
          romAddress = 0;
          PORTH &= ~(1 << 0);
          delay(50);
          PORTH |= (1 << 0);
          writeByte(0x4000, currBank & 0x7);
        }
        else if (romType == 0x101 || romType == 0x103) // Set ROM bank for MBC1M
        {
          if (currBank < 10)
          {
            writeByte(0x4000, currBank >> 4);
            writeByte(0x2000, (currBank & 0x1F));
          }
          else
          {
            writeByte(0x4000, currBank >> 4);
            writeByte(0x2000, 0x10 | (currBank & 0x1F));
          }
        }
        else if (romType == 32) // Set ROM bank for MBC6
        {
          writeByte(0x2800, 0);
          writeByte(0x3800, 0);
          writeByte(0x2000, currBank);
          writeByte(0x3000, currBank);
        }
        else if (romType == 0xFD) // Set ROM bank for TAMA5
        {
          writeByteSRAM(0xA001, 0);
          writeByteSRAM(0xA000, currBank & 0x0F);
          writeByteSRAM(0xA001, 1);
          writeByteSRAM(0xA000, (currBank >> 4) & 0x0F);
        }
        else if (romType >= 5) // Set ROM bank for MBC2/3/4/5
        {
          if (romType >= 11 && romType <= 13)
          {
            if ((currBank & 0x1F) == 0)
            {
              // reset MMM01
              PORTH &= ~(1 << 0);
              PORTH |= (1 << 0);

              // remap to higher 4Mbits ROM
              writeByte(0x3FFF, 0x20);
              writeByte(0x5FFF, 0x40);
              writeByte(0x7FFF, 0x01);
              writeByte(0x1FFF, 0x3A);
              writeByte(0x1FFF, 0x7A);

              // for every 4Mbits ROM, restart from 0x0000
              romAddress = 0x0000;
              currBank++;
            }
            else
            {
              writeByte(0x6000, 0);
              writeByte(0x2000, (currBank & 0x1F));
            }
          }
          else
          {
            if ((romType >= 0x19 && romType <= 0x1E) && (currBank == 0 || currBank == 256))
            {
              writeByte(0x3000, (currBank >> 8) & 0xFF);
            }

            writeByte(0x2100, currBank & 0xFF);
          }
        }
        else // Set ROM bank for MBC1
        {
          writeByte(0x6000, 0);
          writeByte(0x4000, currBank >> 5);
          writeByte(0x2000, currBank & 0x1F);
        }

        while (romAddress <= endAddress) // Read banks and save to SD
        {
          for (int i = 0; i < 512; i++)
          {
            OSCR::Storage::Shared::buffer[i] = readByte(romAddress + i);
          }

          OSCR::Storage::Shared::writeBuffer();

          romAddress += 512;

          OSCR::UI::ProgressBar::advance(512);
        }
      }

      cartOff();

      OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

      OSCR::UI::ProgressBar::finish();

      OSCR::UI::setLineRel(-1);
      OSCR::UI::clearLine();
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Checksum));

      OSCR::Storage::Shared::rewind();

      // Internal ROM checksum
      uint16_t calcChecksum = 0;
      uint32_t const fileBlocks = OSCR::Storage::Shared::getSize() / 512;

      for (uint16_t i = fileBlocks; i; --i)
      {
        OSCR::Storage::Shared::fill();

        for (uint16_t c = 0; c < 512; ++c)
        {
          calcChecksum += OSCR::Storage::Shared::buffer[c];
        }
      }

      OSCR::Storage::Shared::close();

      // Subtract checksum bytes
      calcChecksum -= eepbit[6];
      calcChecksum -= eepbit[7];

      OSCR::UI::clearLine();
      OSCR::UI::print(FS(OSCR::Strings::Common::Checksum));
      OSCR::UI::printHex(calcChecksum);

      if (calcChecksum != checksum)
      {
        OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
        OSCR::UI::printHexLine(checksum);
        OSCR::UI::error(FS(OSCR::Strings::Common::Invalid));
        return false;
      }

      OSCR::UI::print(FS(OSCR::Strings::Symbol::Arrow));
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

      return OSCR::Databases::Basic::matchCRC();
    }

    /******************************************
      SRAM functions
    *****************************************/
    void disableFlashSaveMemory()
    {
      writeByte(0x1000, 0x01);
      writeByte(0x0C00, 0x00);
      writeByte(0x1000, 0x00);
      writeByte(0x2800, 0x00);
      writeByte(0x3800, 0x00);
    }

    bool readSave()
    {
      if (!lastByteAddress)
      {
        OSCR::UI::error(FS(OSCR::Strings::Errors::NoSave));
        return false;
      }

      switch (romType)
      {
      case 32: return readSRAMFLASH_MBC6();
      case 34: return readEEPROM_MBC7();
      default: return readSRAM();
      }
    }

    bool writeSave()
    {
      if (!lastByteAddress)
      {
        OSCR::UI::error(FS(OSCR::Strings::Errors::NoSave));
        return false;
      }

      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

      switch (romType)
      {
      case 32: return writeSRAMFLASH_MBC6();
      case 34: return writeEEPROM_MBC7();
      default: return writeSRAM();
      }
    }

    // Read RAM
    bool readSRAM()
    {
      printHeader();

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoy), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::Save));

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

      cartOn();

      // MBC2 Fix
      readByte(0x0134);

      if (romType <= 4 || (romType >= 11 && romType <= 13))
      {
        writeByte(0x6000, 1);
      }

      // Initialise MBC
      writeByte(0x0000, 0x0A);

      // Switch SRAM banks
      for (uint8_t currBank = 0; currBank < sramBanks; currBank++)
      {
        writeByte(0x4000, currBank);

        // Read SRAM
        for (uint16_t sramAddress = 0xA000; sramAddress <= lastByteAddress; sramAddress += 64)
        {
          for (uint8_t i = 0; i < 64; i++)
          {
            OSCR::Storage::Shared::buffer[i] = readByteSRAM(sramAddress + i);
          }

          OSCR::Storage::Shared::writeBuffer(64);
        }
      }

      // Disable SRAM
      writeByte(0x0000, 0x00);

      cartOff();

      OSCR::Storage::Shared::close();

      OSCR::UI::print(FS(OSCR::Strings::Common::DONE));

      return true;
    }

    // Write RAM
    bool writeSRAM()
    {
      printHeader();

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

      cartOn();

      // MBC2 Fix
      readByte(0x0134);

      // Enable SRAM for MBC1
      if (romType <= 4 || (romType >= 11 && romType <= 13))
      {
        writeByte(0x6000, 1);
      }

      // Initialise MBC
      writeByte(0x0000, 0x0A);

      // Switch RAM banks
      for (uint8_t currBank = 0; currBank < sramBanks; currBank++)
      {
        writeByte(0x4000, currBank);

        // Write RAM
        for (uint16_t sramAddress = 0xA000; sramAddress <= lastByteAddress; sramAddress++)
        {
          writeByteSRAM(sramAddress, OSCR::Storage::Shared::sharedFile.read());
        }
      }

      // Disable SRAM
      writeByte(0x0000, 0x00);

      OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

      OSCR::Storage::Shared::rewind();

      // Variable for errors
      writeErrors = 0;

      // MBC2 Fix
      readByte(0x0134);

      // Check SRAM size
      if (lastByteAddress > 0)
      {
        if (romType <= 4) // MBC1
        {
          writeByte(0x6000, 1);  // Set RAM Mode
        }

        // Initialise MBC
        writeByte(0x0000, 0x0A);

        // Switch SRAM banks
        for (uint8_t currBank = 0; currBank < sramBanks; currBank++)
        {
          writeByte(0x4000, currBank);

          // Read SRAM
          for (uint16_t sramAddress = 0xA000; sramAddress <= lastByteAddress; sramAddress += 64)
          {
            //fill OSCR::Storage::Shared::buffer
            OSCR::Storage::Shared::readBuffer(64);

            for (int c = 0; c < 64; c++)
            {
              if (readByteSRAM(sramAddress + c) != OSCR::Storage::Shared::buffer[c])
              {
                writeErrors++;
              }
            }
          }
        }
        // Disable RAM
        writeByte(0x0000, 0x00);
      }

      cartOff();

      OSCR::Storage::Shared::close();

      if (writeErrors > 0)
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
        return false;
      }

      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
      return true;
    }

    // Read SRAM + FLASH save data of MBC6
    bool readSRAMFLASH_MBC6()
    {
      printHeader();

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoy), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::Save));

      cartOn();

      OSCR::UI::ProgressBar::init(0x108000, 1);

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

      // Enable Mapper and SRAM
      writeByte(0x0000, 0x0A);

      // Switch SRAM banks
      for (uint8_t currBank = 0; currBank < sramBanks; currBank++)
      {
        writeByte(0x0400, currBank);
        writeByte(0x0800, currBank);

        // Read SRAM
        for (uint16_t sramAddress = 0xA000; sramAddress <= lastByteAddress; sramAddress += 64)
        {
          for (uint8_t i = 0; i < 64; i++)
          {
            OSCR::Storage::Shared::buffer[i] = readByteSRAM(sramAddress + i);
          }

          OSCR::Storage::Shared::writeBuffer(64);
          OSCR::UI::ProgressBar::advance(64);
        }
      }

      // Disable SRAM
      writeByte(0x0000, 0x00);

      // Enable flash save memory (map to ROM)
      writeByte(0x1000, 0x01);
      writeByte(0x0C00, 0x01);
      writeByte(0x1000, 0x00);
      writeByte(0x2800, 0x08);
      writeByte(0x3800, 0x08);

      // Switch FLASH banks
      for (uint8_t currBank = 0; currBank < 128; currBank++)
      {
        uint16_t romAddress = 0x4000;

        writeByte(0x2000, currBank);
        writeByte(0x3000, currBank);

        // Read banks and save to SD
        while (romAddress <= 0x5FFF)
        {
          for (int i = 0; i < 512; i++)
          {
            OSCR::Storage::Shared::buffer[i] = readByte(romAddress + i);
          }

          OSCR::Storage::Shared::writeBuffer();

          romAddress += 512;

          OSCR::UI::ProgressBar::advance(512);
        }
      }

      disableFlashSaveMemory();

      cartOff();

      OSCR::Storage::Shared::close();

      OSCR::UI::ProgressBar::finish();

      return true;
    }

    bool waitSRAMFLASH_MBC6(uint16_t address)
    {
      for (uint8_t lives = 100; (readByte(address) != 0x80); lives--)
      {
        if (!lives)
        {
          disableFlashSaveMemory();
          cartOff();

          OSCR::Storage::Shared::close();

          OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
          OSCR::UI::error(FS(OSCR::Strings::Errors::TimedOut));

          return false;
        }

        delay(1);
      }

      return true;
    }

    // Write RAM
    bool writeSRAMFLASH_MBC6()
    {
      printHeader();

      OSCR::UI::ProgressBar::init(0x108000, 1);

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

      cartOn();

      // Enable Mapper and SRAM
      writeByte(0x0000, 0x0A);

      // Switch SRAM banks
      for (uint8_t currBank = 0; currBank < sramBanks; currBank++)
      {
        writeByte(0x0400, currBank);
        writeByte(0x0800, currBank);

        // Write SRAM
        for (uint16_t sramAddress = 0xA000; sramAddress <= lastByteAddress; sramAddress++)
        {
          writeByteSRAM(sramAddress, OSCR::Storage::Shared::sharedFile.read());
        }

        OSCR::UI::ProgressBar::advance((lastByteAddress + 1) - 0xA000);
      }

      // Disable SRAM
      writeByte(0x0000, 0x00);

      // Enable flash save memory (map to ROM)
      writeByte(0x1000, 0x01);
      writeByte(0x0C00, 0x01);
      writeByte(0x1000, 0x01);
      writeByte(0x2800, 0x08);
      writeByte(0x3800, 0x08);

      for (uint8_t currBank = 0; currBank < 128; currBank++)
      {
        uint16_t romAddress = 0x4000;

        // Erase FLASH sector
        if (((OSCR::UI::ProgressBar::current - 0x8000) % 0x20000) == 0)
        {
          OSCR::UI::clearLine();
          OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

          writeByte(0x2800, 0x08);
          writeByte(0x3800, 0x08);
          writeByte(0x2000, 0x01);
          writeByte(0x3000, 0x02);
          writeByte(0x7555, 0xAA);
          writeByte(0x4AAA, 0x55);
          writeByte(0x7555, 0x80);
          writeByte(0x7555, 0xAA);
          writeByte(0x4AAA, 0x55);
          writeByte(0x2800, 0x08);
          writeByte(0x3800, 0x08);
          writeByte(0x2000, currBank);
          writeByte(0x3000, currBank);
          writeByte(0x4000, 0x30);

          if (!waitSRAMFLASH_MBC6(0x4000)) return false;
        }
        else
        {
          writeByte(0x2800, 0x08);
          writeByte(0x3800, 0x08);
          writeByte(0x2000, currBank);
          writeByte(0x3000, currBank);
        }

        OSCR::UI::clearLine();
        OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

        // Write to FLASH
        while (romAddress <= 0x5FFF)
        {
          writeByte(0x2000, 0x01);
          writeByte(0x3000, 0x02);
          writeByte(0x7555, 0xAA);
          writeByte(0x4AAA, 0x55);
          writeByte(0x7555, 0xA0);
          writeByte(0x2800, 0x08);
          writeByte(0x3800, 0x08);
          writeByte(0x2000, currBank);
          writeByte(0x3000, currBank);

          for (int i = 0; i < 128; i++)
          {
            writeByte(romAddress++, OSCR::Storage::Shared::sharedFile.read());
          }

          writeByte(romAddress - 1, 0x00);

          if (!waitSRAMFLASH_MBC6(romAddress - 1)) return false;

          writeByte(romAddress - 1, 0xF0);

          OSCR::UI::ProgressBar::advance(128);
        }
      }

      disableFlashSaveMemory();
      cartOff();

      OSCR::Storage::Shared::close();

      OSCR::UI::print(FS(OSCR::Strings::Common::DONE));

      OSCR::UI::ProgressBar::finish();

      return true;
    }

    bool readEEPROM_MBC7()
    {
      printHeader();

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoy), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::Save));

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

      cartOn();

      uint16_t* data = (uint16_t*)OSCR::Storage::Shared::buffer;

      // enable 0xA000 - 0xBFFF access
      writeByte(0x0000, 0x0A);
      writeByte(0x4000, 0x40);

      // set CS high
      writeByte(0xA080, 0x00);
      writeByte(0xA080, 0x80);

      sendMBC7EEPROM_Inst(2, 0, 0);

      // read data from EEPROM
      for (uint16_t i = 0; i < lastByteAddress; i += 2, data++)
      {
        *data = 0;

        for (uint8_t j = 0; j < 16; j++)
        {
          writeByte(0xA080, 0x80);
          writeByte(0xA080, 0xC0);

          *data <<= 1;
          *data |= (readByte(0xA080) & 0x01);
        }
      }

      // deselect chip and ram access
      writeByte(0xA080, 0x00);
      writeByte(0x0000, 0x00);
      writeByte(0x4000, 0x00);

      cartOff();

      OSCR::Storage::Shared::writeBuffer(lastByteAddress);

      OSCR::Storage::Shared::close();

      OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

      return true;
    }

    bool writeEEPROM_MBC7()
    {
      printHeader();

      OSCR::Storage::Shared::readBuffer(lastByteAddress);

      OSCR::Storage::Shared::close();

      uint16_t * data = (uint16_t*)OSCR::Storage::Shared::buffer;

      cartOn();

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Unlocking));

      // enable 0xA000 - 0xBFFF access
      writeByte(0x0000, 0x0A);
      writeByte(0x4000, 0x40);

      // set CS high
      writeByte(0xA080, 0x00);
      writeByte(0xA080, 0x80);

      // unlock EEPROM by sending EWEN instruction
      sendMBC7EEPROM_Inst(0, 0xC0, 0);

      // set CS low
      writeByte(0xA080, 0x00);

      OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

      // erase entire EEPROM by sending ERAL instruction
      sendMBC7EEPROM_Inst(0, 0x80, 0);

      // set CS low
      writeByte(0xA080, 0x00);
      // set CS high
      writeByte(0xA080, 0x80);

      // wait for erase complete
      do
      {
        writeByte(0xA080, 0x80);
        writeByte(0xA080, 0xC0);
      }
      while ((readByte(0xA080) & 0x01) == 0);

      OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

      // write data to EEPROM by sending WRITE instruction word by word
      for (uint16_t i = 0; i < lastByteAddress; i += 2, data++)
      {
        sendMBC7EEPROM_Inst(1, (i >> 1), *data);

        // set CS low
        writeByte(0xA080, 0x00);
        // set CS high
        writeByte(0xA080, 0x80);

        // wait for writing complete
        do
        {
          writeByte(0xA080, 0x80);
          writeByte(0xA080, 0xC0);
        }
        while ((readByte(0xA080) & 0x01) == 0);
      }

      OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Locking));

      // re-lock EEPROM
      sendMBC7EEPROM_Inst(0, 0, 0);
      writeByte(0xA080, 0x00);

      // disable ram access
      writeByte(0x0000, 0x00);
      writeByte(0x4000, 0x00);

      cartOff();

      OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

      return true;
    }

    void sendMBC7EEPROM_Inst(uint8_t op, uint8_t addr, uint16_t data)
    {
      bool send_data = false;
      uint8_t i;

      op = op & 0x03;

      if (op == 0)
      {
        addr &= 0xC0;
        if (addr == 0x40)
          send_data = true;
      }
      else if (op == 1)
      {
        send_data = true;
      }

      // send start bit
      writeByte(0xA080, 0x82);
      writeByte(0xA080, 0xC2);

      // send op
      for (i = 0; i < 2; i++, op <<= 1)
      {
        eepbit[1] = (op & 0x02);
        eepbit[0] = (eepbit[1] | 0x80);
        eepbit[1] |= 0xC0;

        writeByte(0xA080, eepbit[0]);
        writeByte(0xA080, eepbit[1]);
      }

      // send addr
      for (i = 0; i < 8; i++, addr <<= 1)
      {
        eepbit[1] = ((addr & 0x80) >> 6);
        eepbit[0] = (eepbit[1] | 0x80);
        eepbit[1] |= 0xC0;

        writeByte(0xA080, eepbit[0]);
        writeByte(0xA080, eepbit[1]);
      }

      if (send_data)
      {
        for (i = 0; i < 16; i++, data <<= 1)
        {
          eepbit[1] = ((data & 0x8000) >> 14);
          eepbit[0] = (eepbit[1] | 0x80);
          eepbit[1] |= 0xC0;

          writeByte(0xA080, eepbit[0]);
          writeByte(0xA080, eepbit[1]);
        }
      }
    }

# if HAS_FLASH
    /******************************************
      29F016/29F032/29F033/39SF040 flashrom functions
    *****************************************/
    void sendFlashCommand(uint8_t cmd, bool commandSet)
    {
      if (commandSet == 0)
      {
        //29F016/29F032/29F033
        writeByte(0x555, 0xAA, audioWE);
        writeByte(0x2AA, 0x55, audioWE);
        writeByte(0x555, cmd, audioWE);
      }
      else if (commandSet == 1)
      {
        //39SF040
        writeByte(0x5555, 0xAA, audioWE);
        writeByte(0x2AAA, 0x55, audioWE);
        writeByte(0x5555, cmd, audioWE);
      }
    }

    // Read the status register
    void busyCheck(uint32_t address, uint8_t data)
    {
      uint8_t statusReg = readByte(address);

      //byte count = 0;
      while ((statusReg & 0x80) != (data & 0x80))
      {
        // Update Status
        statusReg = readByte(address);
        /* Debug
        count++;
        if (count > 250)
        {
          OSCR::UI::printLine();
          OSCR::UI::printValue(OSCR::Strings::Common::Bank, currBank);
          OSCR::UI::printValue(OSCR::Strings::Common::Address, currAddr + currByte);
          OSCR::UI::wait();
        }
        */
      }
    }

    // Write AMD type flashrom
    // A0-A13 directly connected to cart edge -> 16384(0x0-0x3FFF) bytes per bank -> 256(0x0-0xFF) banks
    // A14-A21 connected to MBC5
    void writeFlash(uint8_t MBC, bool commandSet, bool flashErase)
    {
      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

      printHeader();

      cartOn();

      // Get rom size from file
      OSCR::Storage::Shared::sharedFile.seekCur(0x147);

      romType = OSCR::Storage::Shared::sharedFile.read();
      romSize = OSCR::Storage::Shared::sharedFile.read();

      // Go back to file beginning
      OSCR::Storage::Shared::rewind();

      // ROM banks
      romBanks = 2;
      if (romSize >= 0x01 && romSize <= 0x07)
      {
        romBanks = OSCR::Util::power<2>(romSize + 1);
      }

      if (MBC > 0)
      {
        // Set ROM bank hi 0
        writeByte(0x3000, 0);
        // Set ROM bank low 0
        writeByte(0x2000, 0);
        delay(100);
      }

      // Reset flash
      sendFlashCommand(0xF0, commandSet);
      delay(100);

      // ID command sequence
      sendFlashCommand(0x90, commandSet);

      // Read the two id bytes into a string
      flashid = readByte(0) << 8;
      flashid |= readByte(1);

      if (OSCR::Cores::Flash::getFlashDetail())
      {
        OSCR::UI::printLine(OSCR::Databases::Basic::mapperDetail->name);
        OSCR::UI::printValue(OSCR::Strings::Common::Banks, (uint32_t)romBanks, (uint32_t)(OSCR::Cores::Flash::flashSize / 0x4000));
      }
      else
      {
        OSCR::UI::printLabel(OSCR::Strings::Common::ID);
        OSCR::UI::printHexLine(flashid);

        OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
        return;
      }

      // Reset flash
      sendFlashCommand(0xF0, commandSet);

      delay(100);

      if (flashErase)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        // Erase flash
        sendFlashCommand(0x80, commandSet);
        sendFlashCommand(0x10, commandSet);

        // Wait until erase is complete
        busyCheck(0, 0x80);

        // Blankcheck
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

        // Read x number of banks
        for (uint16_t currBank = 0; currBank < romBanks; currBank++)
        {
          if (MBC > 0)
          {
            // Set ROM bank
            writeByte(0x2000, currBank);
          }

          for (uint16_t currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512)
          {
            for (int currByte = 0; currByte < 512; currByte++)
            {
              OSCR::Storage::Shared::buffer[currByte] = readByte(currAddr + currByte);
            }

            for (int j = 0; j < 512; j++)
            {
              if (OSCR::Storage::Shared::buffer[j] != 0xFF)
              {
                OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::BlankCheck));
                OSCR::UI::fatalError(FS(OSCR::Strings::Common::NotBlank));
              }
            }
          }
        }
      }

      if (MBC == 3)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));
        //OSCR::UI::printLineSync((audioWE) ? F("MBC30 (Audio)") : F("MBC3 (WR)"));

        // Write flash
        uint16_t currAddr = 0;
        uint16_t endAddr = 0x3FFF;

        // Initialize progress bar
        OSCR::UI::ProgressBar::init((uint32_t)(romBanks) * 16384);

        for (uint16_t currBank = 0; currBank < romBanks; currBank++)
        {
          // Set ROM bank
          writeByte(0x2100, currBank);

          // Map SRAM Bank to prevent getting stuck at 0x2A8000
          if (romBanks > 128)
            writeByte(0x4000, 0x0);

          if (currBank > 0)
          {
            currAddr = 0x4000;
            endAddr = 0x7FFF;
          }

          while (currAddr <= endAddr)
          {
            OSCR::Storage::Shared::fill();

            for (int currByte = 0; currByte < 512; currByte++)
            {
              // Write command sequence
              sendFlashCommand(0xA0, commandSet);

              // Write current byte
              writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte], audioWE);

              // Set OE/RD(PH6) LOW
              PORTH &= ~(1 << 6);

              // Busy check
              busyCheck(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);

              // Switch OE/RD(PH6) to HIGH
              PORTH |= (1 << 6);
            }

            currAddr += 512;

            OSCR::UI::ProgressBar::advance(512);
          }
        }

        OSCR::UI::ProgressBar::finish();
      }
      else if (MBC == 5)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));
        //OSCR::UI::printLineSync((audioWE) ? F("MBC5 (Audio)") : F("MBC5 (WR)"));

        OSCR::UI::ProgressBar::init((uint32_t)(romBanks)*16384);

        for (uint16_t currBank = 0; currBank < romBanks; currBank++)
        {
          // Set ROM bank
          writeByte(0x2000, currBank);
          // 0x2A8000 fix
          writeByte(0x4000, 0x0);

          for (uint16_t currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512)
          {
            OSCR::Storage::Shared::fill();

            for (int currByte = 0; currByte < 512; currByte++)
            {
              // Write command sequence
              sendFlashCommand(0xA0, commandSet);
              // Write current byte
              writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte], audioWE);

              // Set OE/RD(PH6) LOW
              PORTH &= ~(1 << 6);

              // Busy check
              busyCheck(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);

              // Switch OE/RD(PH6) to HIGH
              PORTH |= (1 << 6);
            }

            OSCR::UI::ProgressBar::advance(512);
          }
        }

        OSCR::UI::ProgressBar::finish();
      }
      else if (MBC == 0)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));
        //OSCR::UI::printLineSync((audioWE) ? F("Flash (Audio)") : F("Flash (WR)"));

        // Limit file size to 32KB
        romBanks = 2;

        OSCR::UI::ProgressBar::init((uint32_t)(romBanks)*16384);

        for (uint16_t currAddr = 0; currAddr < 0x7FFF; currAddr += 512)
        {
          OSCR::Storage::Shared::fill();

          for (int currByte = 0; currByte < 512; currByte++)
          {
            // Write command sequence
            sendFlashCommand(0xA0, commandSet);
            // Write current byte
            writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte], audioWE);

            // Set OE/RD(PH6) LOW
            PORTH &= ~(1 << 6);

            // Busy check
            busyCheck(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);

            // Switch OE/RD(PH6) to HIGH
            PORTH |= (1 << 6);
          }

          OSCR::UI::ProgressBar::advance(512);
        }

        OSCR::UI::ProgressBar::finish();
      }

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

      OSCR::Storage::Shared::rewind();

      writeErrors = 0;

      //
      // Verify flashrom
      //

      // Read number of banks and switch banks
      for (uint16_t bank = 1, romAddress = 0; bank < romBanks; bank++)
      {
        if (MBC > 0)
        {
          if (romType >= 5) // MBC2 and above
          {
            writeByte(0x2100, bank); // Set ROM bank
          }
          else // MBC1
          {
            writeByte(0x6000, 0);            // Set ROM Mode
            writeByte(0x4000, bank >> 5);    // Set bits 5 & 6 (01100000) of ROM bank
            writeByte(0x2000, bank & 0x1F);  // Set bits 0 & 4 (00011111) of ROM bank
          }

          if (bank > 1)
          {
            romAddress = 0x4000;
          }
        }
        // Read up to 7FFF per bank
        while (romAddress <= 0x7FFF)
        {
          // Fill OSCR::Storage::Shared::buffer
          OSCR::Storage::Shared::fill();

          // Compare
          for (int i = 0; i < 512; i++)
          {
            if (readByte(romAddress + i) != OSCR::Storage::Shared::buffer[i])
            {
              writeErrors++;
            }
          }

          romAddress += 512;
        }
      }

      OSCR::Storage::Shared::close();

      cartOff();

      if (writeErrors == 0)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
      }
      else
      {
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
      }
    }

    /******************************************
      CFU flashrom functions
    *****************************************/
    void sendCFICommand(uint8_t cmd)
    {
      writeByteCompensated(0xAAA, 0xAA);
      writeByteCompensated(0x555, 0x55);
      writeByteCompensated(0xAAA, cmd);
    }

    /*
      Flash chips can either be in x8 mode or x16 mode and sometimes the two
      least significant bits on flash cartridges' data lines are swapped.
      This function reads a byte and compensates for the differences.
      This is only necessary for commands to the flash, not for data read from the flash, the MBC or SRAM.

      address needs to be the x8 mode address of the flash register that should be read.
    */
    uint8_t readByteCompensated(int address)
    {
      uint8_t data = readByte(address >> (flashX16Mode ? 1 : 0));
      if (flashSwitchLastBits) {
        return (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
      }
      return data;
    }

    /*
      Flash chips can either be in x8 mode or x16 mode and sometimes the two
      least significant bits on flash cartridges' data lines are swapped.
      This function writes a byte and compensates for the differences.
      This is only necessary for commands to the flash, not for data written to the flash, the MBC or SRAM.
      .
      address needs to be the x8 mode address of the flash register that should be read.
    */
    void writeByteCompensated(int address, uint8_t data)
    {
      if (flashSwitchLastBits)
      {
        data = (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
      }

      writeByte(address >> (flashX16Mode ? 1 : 0), data);
    }

    /* Identify the different flash chips.
      Sets the global variables flashBanks, flashX16Mode and flashSwitchLastBits
    */
    bool identifyCFI()
    {
      cartOn();

      // Reset flash
      writeByte(0x6000, 0);  // Set ROM Mode
      writeByte(0x2000, 0);  // Set Bank to 0
      writeByte(0x3000, 0);

      OSCR::Cores::Flash::startCFIMode(false);  // Trying x8 mode first

      // Trying x8 mode first
      if (!OSCR::Cores::Flash::identifyCFIbyIds(false))
      {
        // Try x16 mode next
        if (!OSCR::Cores::Flash::identifyCFIbyIds(true))
        {
          cartOff();

          OSCR::UI::error(F("CFI Query failed!"));

          return false;
        }
      }

      flashBanks = 1 << (readByteCompensated(0x4E) - 14);  // - flashX16Mode);

      // Reset flash
      writeByteCompensated(0xAAA, 0xF0);
      delay(100);

      cartOff();

      return true;
    }

    // Write 29F032 flashrom
    // A0-A13 directly connected to cart edge -> 16384(0x0-0x3FFF) bytes per bank -> 256(0x0-0xFF) banks
    // A14-A21 connected to MBC5
    // identifyFlash() needs to be run before this!
    bool writeCFI()
    {
      printHeader();

      OSCR::Storage::Shared::openRO();

      // Get rom size from file
      OSCR::Storage::Shared::sharedFile.seekCur(0x147);

      romType = OSCR::Storage::Shared::sharedFile.read();
      romSize = OSCR::Storage::Shared::sharedFile.read();

      // Go back to file beginning
      OSCR::Storage::Shared::rewind();

      // ROM banks
      romBanks = 2;

      if (romSize >= 0x01 && romSize <= 0x07)
      {
        romBanks = OSCR::Util::power<2>(romSize + 1);
      }

      if (romBanks > flashBanks)
      {
        OSCR::UI::error(FS(OSCR::Strings::Errors::NotLargeEnough));
        return false;
      }

      OSCR::UI::printValue(OSCR::Strings::Common::Banks, (uint32_t)romBanks, (uint32_t)flashBanks);

      cartOn();

      // Set ROM bank hi 0
      writeByte(0x3000, 0);

      // Set ROM bank low 0
      writeByte(0x2000, 0);

      delay(100);

      // Reset flash
      writeByteCompensated(0xAAA, 0xF0);
      delay(100);

      // Reset flash
      writeByte(0x555, 0xF0);
      delay(100);

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

      // Erase flash
      sendCFICommand(0x80);
      sendCFICommand(0x10);

      // Read the status register
      uint8_t statusReg = readByte(0);

      // After a completed erase D7 will output 1
      while ((statusReg & 0x80) != 0x80)
      {
        delay(100);

        // Update Status
        statusReg = readByte(0);
      }

      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

      // Read x number of banks
      for (uint16_t currBank = 0; currBank < romBanks; currBank++)
      {
        // Set ROM bank
        writeByte(0x2000, currBank);

        for (uint16_t currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512)
        {
          for (int currByte = 0; currByte < 512; currByte++)
          {
            OSCR::Storage::Shared::buffer[currByte] = readByte(currAddr + currByte);
          }

          for (int j = 0; j < 512; j++)
          {
            if (OSCR::Storage::Shared::buffer[j] != 0xFF)
            {
              OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
              OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
              return false;
            }
          }
        }
      }

      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

      // Write flash
      uint16_t currAddr = 0;
      uint16_t endAddr = 0x3FFF;

      for (uint16_t currBank = 0; currBank < romBanks; currBank++)
      {
        // Set ROM bank
        writeByte(0x2100, currBank);

        // 0x2A8000 fix
        writeByte(0x4000, 0x0);

        if (currBank > 0) {
          currAddr = 0x4000;
          endAddr = 0x7FFF;
        }

        while (currAddr <= endAddr)
        {
          OSCR::Storage::Shared::fill();

          for (int currByte = 0; currByte < 512; currByte++)
          {
            // Write command sequence
            sendCFICommand(0xA0);

            // Write current byte
            writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);

            // Setting CS(PH3) and OE/RD(PH6) LOW
            PORTH &= ~((1 << 3) | (1 << 6));

            // Busy check
            short i = 0;

            while ((PINC & 0x80) != (OSCR::Storage::Shared::buffer[currByte] & 0x80))
            {
              i++;

              if (i > 500)
              {
                if (currAddr < 0x4000) // This happens when trying to flash an MBC5 as if it was an MBC3. Retry to flash as MBC5, starting from last successfull byte.
                {
                  currByte--;
                  currAddr += 0x4000;
                  endAddr = 0x7FFF;
                  break;
                }
                else // If a timeout happens while trying to flash MBC5-style, flashing failed.
                {
                  OSCR::Cores::GameBoy::cartOff();
                  OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
                  OSCR::UI::error(FS(OSCR::Strings::Errors::TimedOut));
                  return false;
                }
              }
            }

            // Switch CS(PH3) and OE/RD(PH6) to HIGH
            PORTH |= (1 << 3) | (1 << 6);

            // Waste a few CPU cycles to remove write errors
            AVR_ASM(
              AVR_INS("nop")
              AVR_INS("nop")
              AVR_INS("nop")
            );
          }
          currAddr += 512;
        }
      }

      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

      // Verify flashrom

      OSCR::Storage::Shared::rewind();

      writeErrors = 0;

      // Read number of banks and switch banks
      for (uint16_t bank = 1, romAddress = 0; bank < romBanks; bank++)
      {
        if (romType >= 5) // MBC2 and above
        {
          writeByte(0x2100, bank); // Set ROM bank
        }
        else // MBC1
        {
          writeByte(0x6000, 0);            // Set ROM Mode
          writeByte(0x4000, bank >> 5);    // Set bits 5 & 6 (01100000) of ROM bank
          writeByte(0x2000, bank & 0x1F);  // Set bits 0 & 4 (00011111) of ROM bank
        }

        if (bank > 1)
        {
          romAddress = 0x4000;
        }

        // Read up to 7FFF per bank
        while (romAddress <= 0x7FFF)
        {
          // Fill OSCR::Storage::Shared::buffer
          OSCR::Storage::Shared::fill();

          // Compare
          for (int i = 0; i < 512; i++)
          {
            if (readByte(romAddress + i) != OSCR::Storage::Shared::buffer[i])
            {
              writeErrors++;
            }
          }

          romAddress += 512;
        }
      }

      cartOff();

      OSCR::Storage::Shared::close();

      if (writeErrors)
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
        return false;
      }

      OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));

      return true;
    }
# endif /* HAS_FLASH */

    /**************************************************
      Pelican Gameboy Device Read Function
    **************************************************/
    void sendW29C020Command(uint8_t cmd)
    {
      writeByteSRAM(0xA000, 0x2);
      sendW29C020CommandSufix(cmd);
    }

    void sendW29C020CommandSufix(uint8_t cmd)
    {
      writeByte(0x3555, 0xAA);
      writeByteSRAM(0xA000, 0x1);
      writeByte(0x2AAA, 0x55);
      writeByteSRAM(0xA000, 0x2);
      writeByte(0x3555, cmd);
    }

    // Read Pelican GBC Device - All Brainboys, MonsterBrains, Codebreakers
    void readPelican()
    {
      constexpr uint16_t const bankAddress = 0xA000;
      constexpr uint16_t const startAddress = 0x2000;
      constexpr uint16_t const finalAddress = 0x3FFF;

      printHeader();

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoy), FS(OSCR::Strings::Directory::ROM), "Pelican");

      cartOn();

      //Enable bank addressing in the CPLD
      readByte(0x100);

      // W29C020 ID command sequence
      sendW29C020Command(0x80);
      sendW29C020CommandSufix(0x60);
      delay(10);

      // Read the two id bytes into a string
      writeByteSRAM(0xA000, 0x0);
      flashid = readByte(0) << 8;
      flashid |= readByte(1);

      // W29C020 Flash ID Mode Exit
      sendW29C020Command(0xF0);
      delay(100);

      if ((flashid != 0xDA45) && (flashid != 0xBF10) && (flashid != 0xBF04))
      {
        writeByteSRAM(0xA000, 0x2);
        writeByte(0x3555, 0xFF);
        delay(100);
        writeByteSRAM(0xA000, 0x2);
        writeByte(0x3555, 0x90);
        delay(100);
        flashid = readByte(0) << 8;
        flashid |= readByte(1);
        writeByteSRAM(0xA000, 0x2);
        writeByte(0x3555, 0xFF);
        delay(100);

        if (flashid != 0xBF04)
        {
          OSCR::UI::printLabel(OSCR::Strings::Common::ID);
          OSCR::UI::printHexLine(flashid);
          OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
          return;
        }
      }

      if (flashid == 0xDA45 || flashid == 0xBF10)
      {
        romSize = 0x40000;
        romBanks = 32;

        OSCR::UI::printLine(F("29EE020 / W29C020"));
      }
      else if (flashid == 0xBF04)
      {
        romSize = 0x80000;
        romBanks = 64;

        OSCR::UI::printLine(F("SST 28LF040"));
      }

      OSCR::UI::printValue(OSCR::Strings::Common::Banks, romBanks, (uint16_t)64);

      OSCR::UI::printSize(OSCR::Strings::Common::ROM, romSize, true);

      OSCR::UI::ProgressBar::init((uint32_t)(romBanks)*8192);

      for (size_t workBank = 0; workBank < romBanks; workBank++) // Loop over banks
      {
        writeByteSRAM(bankAddress, (workBank & 0xFF));

        // Read banks and save to SD
        for (uint16_t address = startAddress; address <= finalAddress; address+=512)
        {
          for (int i = 0; i < 512; i++)
          {
            OSCR::Storage::Shared::buffer[i] = readByte(address + i);
          }

          OSCR::Storage::Shared::writeBuffer();

          OSCR::UI::ProgressBar::advance(512);
        }
      }

      OSCR::UI::ProgressBar::finish();

      OSCR::Storage::Shared::close();
    }

    /******************************************
      Pelican Gameboy Device Write Function
    *****************************************/
    void send28LF040Potection(bool enable)
    {
      writeByteSRAM(0xA000, 0x0);
      readByte(0x3823);
      readByte(0x3820);
      readByte(0x3822);
      readByte(0x2418);
      readByte(0x241B);
      readByte(0x2419);
      readByte(enable ? 0x240A : 0x241A);
    }

    // Write Pelican GBC Device - All Brainboys, MonsterBrains, Codebreakers
    void writePelican()
    {
      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

      // Enable Rom Banks
      readByte(0x100);
      delay(100);

      // W29C020 ID command sequence
      sendW29C020Command(0x80);
      sendW29C020CommandSufix(0x60);
      delay(10);

      // Read the two id bytes into a string
      writeByteSRAM(0xA000, 0x0);
      flashid = readByte(0) << 8;
      flashid |= readByte(1);

      // W29C020 Flash ID Mode Exit
      sendW29C020Command(0xF0);
      delay(100);

      if (flashid == 0xDA45 || flashid == 0xBF10)
      {
        OSCR::UI::printLine(F("29EE020 / W29C020"));
        OSCR::UI::printLine(F("Banks Used: 32/64"));
        romBanks = 32;
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        if (flashid == 0xDA45)
        {
          // Disable BootBlock
          sendW29C020Command(0x80);
          sendW29C020CommandSufix(0x40);
          writeByteSRAM(0xA000, 0x1);
          writeByte(0x2AAA, 0xAA);
          delay(100);
        }

        // Erase flash
        sendW29C020Command(0x80);
        sendW29C020CommandSufix(0x10);
        delay(1000);
      }
      else
      {
        writeByteSRAM(0xA000, 0x2);
        writeByte(0x3555, 0xFF);
        delay(100);
        writeByteSRAM(0xA000, 0x2);
        writeByte(0x3555, 0x90);
        delay(100);
        flashid = readByte(0) << 8;
        flashid |= readByte(1);
        writeByteSRAM(0xA000, 0x2);
        writeByte(0x3555, 0xFF);
        delay(100);
        if (flashid != 0xBF04)
        {
          OSCR::UI::printLabel(OSCR::Strings::Common::ID);
          OSCR::UI::printHexLine(flashid);
          OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
          return;
        }
      }

      if (flashid == 0xBF04)
      {
        OSCR::UI::printLine(F("SST 28LF040"));
        OSCR::UI::printLine(F("Banks Used: 64/64"));
        romBanks = 64;
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        //Unprotect flash
        send28LF040Potection(false);
        delay(100);

        //Erase flash
        writeByteSRAM(0xA000, 0x2);
        writeByte(0x3555, 0x30);
        writeByte(0x3555, 0x30);
        delay(100);

        writeByteSRAM(0xA000, 0x2);
        writeByte(0x3555, 0xFF);
        delay(100);
      }

      // Blankcheck
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

      // Read x number of banks
      for (uint16_t currBank = 0; currBank < romBanks; currBank++)
      {
        // Set ROM bank
        writeByteSRAM(0xA000, currBank);

        for (uint16_t currAddr = 0x2000; currAddr < 0x4000; currAddr += 0x200)
        {
          for (int currByte = 0; currByte < 512; currByte++)
          {
            OSCR::Storage::Shared::buffer[currByte] = readByte(currAddr + currByte);
          }

          for (int j = 0; j < 512; j++)
          {
            if (OSCR::Storage::Shared::buffer[j] != 0xFF)
            {
              OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
              return;
            }
          }
        }
      }

      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

      // Write flash
      uint16_t currAddr = 0x2000;
      uint16_t endAddr = 0x3FFF;
      uint8_t byte1;
      uint8_t byte2;
      bool toggle = true;

      //Unprotect flash
      send28LF040Potection(false);
      delay(100);

      //Initialize progress bar
      OSCR::UI::ProgressBar::init((uint32_t)(romBanks) * 8192);

      for (uint16_t currBank = 0; currBank < romBanks; currBank++)
      {
        currAddr = 0x2000;

        if (flashid == 0xDA45 || flashid == 0xBF10)
        {
          while (currAddr <= endAddr) {
            OSCR::Storage::Shared::readBuffer(128);

            // Write command sequence
            sendW29C020Command(0xA0);

            // Set ROM bank
            writeByteSRAM(0xA000, currBank);

            for (int currByte = 0; currByte < 128; currByte++)
            {
              // Write current byte
              writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);
            }

            currAddr += 128;
            OSCR::UI::ProgressBar::advance(128);

            delay(10);
          }
        }

        if (flashid == 0xBF04)
        {
          while (currAddr <= endAddr)
          {
            OSCR::Storage::Shared::fill();

            for (int currByte = 0; currByte < 512; currByte++)
            {
              toggle = true;
              // Write current byte
              writeByteSRAM(0xA000, 0x2);
              writeByte(0x3555, 0x10);
              writeByteSRAM(0xA000, currBank);
              writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);

              while (toggle)
              {
                byte1 = readByte(currAddr + currByte);
                byte2 = readByte(currAddr + currByte);
                toggle = isToggle(byte1, byte2);
              }

              byte1 = readByte(currAddr + currByte);

              if (byte1 != OSCR::Storage::Shared::buffer[currByte])
              {
                writeByteSRAM(0xA000, 0x2);
                writeByte(0x3555, 0x10);
                writeByteSRAM(0xA000, currBank);
                writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);

                while (toggle)
                {
                  byte1 = readByte(currAddr + currByte);
                  byte2 = readByte(currAddr + currByte);
                  toggle = isToggle(byte1, byte2);
                }
              }
            }

            currAddr += 512;
            OSCR::UI::ProgressBar::advance(512);
          }
        }
      }

      if (flashid == 0xBF04)
      {
        //Protect flash
        send28LF040Potection(true);
        delay(100);
      }

      OSCR::UI::ProgressBar::finish();

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

      OSCR::Storage::Shared::rewind();

      writeErrors = 0;

      // Verify flashrom
      uint16_t romAddress = 0x2000;

      // Read number of banks and switch banks
      for (uint16_t bank = 0; bank < romBanks; bank++)
      {
        writeByteSRAM(0xA000, bank);  // Set ROM bank
        romAddress = 0x2000;

        // Read up to 3FFF per bank
        while (romAddress < 0x4000)
        {
          // Fill OSCR::Storage::Shared::buffer
          OSCR::Storage::Shared::fill();

          // Compare
          for (int i = 0; i < 512; i++)
          {
            if (readByte(romAddress + i) != OSCR::Storage::Shared::buffer[i])
            {
              writeErrors++;
            }
          }

          romAddress += 512;
        }
      }

      OSCR::Storage::Shared::close();

      if (writeErrors == 0)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
      }
      else
      {
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
      }
    }

    bool isToggle(uint8_t byte1, uint8_t byte2)
    {
      // XOR the two bytes to get the bits that are different
      uint8_t difference = byte1 ^ byte2;
      difference = difference & 0b00100000;

      // Check if only the 6th bit is different
      return difference == 0b00100000;
    }

    /******************************************************
      Datel Mega Memory Card Gameboy Device Read Function
    ******************************************************/
    // Read Mega Memory Card Rom and Save Backup Data
    void readMegaMem()
    {
      // Dump the Rom
      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoy), FS(OSCR::Strings::Directory::ROM), "MegaMem");

      uint16_t finalAddress = 0x3FFF;
      uint16_t startAddress = 0x0;

      // Initialize progress bar
      OSCR::UI::ProgressBar::init(16384);

      // Read banks and save to SD
      while (startAddress <= finalAddress)
      {
        for (int i = 0; i < 512; i++)
        {
          OSCR::Storage::Shared::buffer[i] = readByte(startAddress + i);
        }

        OSCR::Storage::Shared::writeBuffer();

        startAddress += 512;
        OSCR::UI::ProgressBar::advance(512);
      }

      OSCR::UI::ProgressBar::finish();

      OSCR::Storage::Shared::close();

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoy), FS(OSCR::Strings::Directory::ROM), "SaveData", FS(OSCR::Strings::FileType::Raw));

      printHeader();

      finalAddress = 0x7FFF;
      startAddress = 0x4000;
      uint16_t bankAddress = 0x2000;
      romBanks = 32;

      // Initialize progress bar
      OSCR::UI::ProgressBar::init((uint32_t)(romBanks)*8192);

      for (size_t workBank = 0; workBank < romBanks; workBank++) {  // Loop over banks

        startAddress = 0x4000;

        writeByte(bankAddress, (workBank & 0xFF));

        // Read banks and save to SD
        while (startAddress <= finalAddress)
        {
          for (int i = 0; i < 512; i++)
          {
            OSCR::Storage::Shared::buffer[i] = readByte(startAddress + i);
          }

          OSCR::Storage::Shared::writeBuffer();

          startAddress += 512;

          OSCR::UI::ProgressBar::advance(512);
        }
      }

      OSCR::UI::ProgressBar::finish();

      OSCR::Storage::Shared::close();
    }

    /*******************************************************
      Datel Mega Memory Card Gameboy Device Write Function
    *******************************************************/
    // Read Mega Memory Card Rom and Save Backup Data
    void writeMegaMem()
    {
      // Write Datel Mega Memory Card Save Storage Chip SST28LF040
      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

      cartOn();

      writeByte(0x2000, 0x1);
      writeByte(0x5555, 0xFF);
      delay(100);
      writeByte(0x2000, 0x1);
      writeByte(0x5555, 0x90);
      delay(100);
      writeByte(0x2000, 0x0);
      flashid = readByte(0x4000) << 8;
      flashid |= readByte(0x4001);
      writeByte(0x2000, 0x1);
      writeByte(0x5555, 0xFF);
      delay(100);

      if (flashid != 0xBF04)
      {
        OSCR::UI::printLabel(OSCR::Strings::Common::ID);
        OSCR::UI::printHexLine(flashid);
        OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
        return;
      }

      OSCR::UI::printLine(F("SST 28LF040"));
      romBanks = 32;

      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

      //Unprotect flash
      writeByte(0x2000, 0x0);
      readByte(0x5823);
      readByte(0x5820);
      readByte(0x5822);
      readByte(0x4418);
      readByte(0x441B);
      readByte(0x4419);
      readByte(0x441A);
      delay(100);

      //Erase flash
      writeByte(0x2000, 0x1);
      writeByte(0x5555, 0x30);
      writeByte(0x5555, 0x30);
      delay(100);

      writeByte(0x2000, 0x1);
      writeByte(0x5555, 0xFF);
      delay(100);

      // Blankcheck
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

      // Read x number of banks
      for (uint16_t currBank = 0; currBank < romBanks; currBank++)
      {
        // Set ROM bank
        writeByte(0x2000, currBank);

        for (uint16_t currAddr = 0x4000; currAddr < 0x8000; currAddr += 0x200)
        {
          for (int currByte = 0; currByte < 512; currByte++)
          {
            OSCR::Storage::Shared::buffer[currByte] = readByte(currAddr + currByte);
          }

          for (int j = 0; j < 512; j++)
          {
            if (OSCR::Storage::Shared::buffer[j] != 0xFF)
            {
              OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
              return;
            }
          }
        }
      }

      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

      // Write flash
      uint16_t currAddr = 0x4000;
      uint16_t endAddr = 0x7FFF;
      uint8_t byte1;
      uint8_t byte2;
      bool toggle = true;

      //Unprotect flash
      writeByte(0x2000, 0x0);
      readByte(0x5823);
      readByte(0x5820);
      readByte(0x5822);
      readByte(0x4418);
      readByte(0x441B);
      readByte(0x4419);
      readByte(0x441A);
      delay(100);

      //Initialize progress bar
      OSCR::UI::ProgressBar::init((uint32_t)(romBanks)*8192);

      for (uint16_t currBank = 0; currBank < romBanks; currBank++)
      {
        currAddr = 0x4000;

        while (currAddr <= endAddr)
        {
          OSCR::Storage::Shared::fill();

          for (int currByte = 0; currByte < 512; currByte++)
          {
            toggle = true;
            // Write current byte
            writeByte(0x2000, 0x1);
            writeByte(0x5555, 0x10);
            writeByte(0x2000, currBank);
            writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);
            while (toggle)
            {
              byte1 = readByte(currAddr + currByte);
              byte2 = readByte(currAddr + currByte);
              toggle = isToggle(byte1, byte2);
            }

            byte1 = readByte(currAddr + currByte);

            if (byte1 != OSCR::Storage::Shared::buffer[currByte])
            {
              writeByte(0x2000, 0x1);
              writeByte(0x5555, 0x10);
              writeByte(0x2000, currBank);
              writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);
              while (toggle)
              {
                byte1 = readByte(currAddr + currByte);
                byte2 = readByte(currAddr + currByte);
                toggle = isToggle(byte1, byte2);
              }
            }
          }

          currAddr += 512;
          OSCR::UI::ProgressBar::advance(512);
        }
      }

      OSCR::UI::ProgressBar::finish();

      // Protect flash
      writeByte(0x2000, 0x0);
      readByte(0x5823);
      readByte(0x5820);
      readByte(0x5822);
      readByte(0x4418);
      readByte(0x441B);
      readByte(0x4419);
      readByte(0x440A);
      delay(100);

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

      OSCR::Storage::Shared::rewind();

      writeErrors = 0;

      // Verify flashrom
      uint16_t romAddress = 0x4000;

      // Read number of banks and switch banks
      for (uint16_t bank = 0; bank < romBanks; bank++)
      {
        writeByte(0x2000, bank);  // Set ROM bank
        romAddress = 0x4000;

        // Read up to 3FFF per bank
        while (romAddress < 0x8000)
        {
          // Fill OSCR::Storage::Shared::buffer
          OSCR::Storage::Shared::fill();

          // Compare
          for (int i = 0; i < 512; i++)
          {
            if (readByte(romAddress + i) != OSCR::Storage::Shared::buffer[i])
            {
              writeErrors++;
            }
          }

          romAddress += 512;
        }
      }

      cartOff();

      OSCR::Storage::Shared::close();

      if (writeErrors == 0)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
      }
      else
      {
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
      }
    }

    /***************************************************
      Datel GBC Gameshark Gameboy Device Read Function
    ***************************************************/
    void sendGamesharkCommand(uint8_t cmd)
    {
      writeByte(0x7FE1, 0x2);
      writeByte(0x5555, 0xAA);
      writeByte(0x7FE1, 0x1);
      writeByte(0x4AAA, 0x55);
      writeByte(0x7FE1, 0x2);
      writeByte(0x5555, cmd);
    }

    // Read Datel GBC Gameshark Device
    void readGameshark()
    {
      uint16_t finalAddress = 0x5FFF;
      uint16_t startAddress = 0x4000;
      romBanks = 16;

      cartOn();

      //Enable bank addressing in the CPLD
      readByte(0x101);
      readByte(0x108);
      readByte(0x101);

      // SST 39SF010 ID command sequence
      sendGamesharkCommand(0x90);
      delay(10);

      // Read the two id bytes into a string
      writeByte(0x7FE1, 0x0);
      flashid = readByte(0x4000) << 8;
      flashid |= readByte(0x4001);

      // SST 39SF010 Flash ID Mode Exit
      sendGamesharkCommand(0xF0);
      delay(100);

      if (flashid == 0xBFB5)
      {
        OSCR::UI::clear();
        OSCR::UI::printLine(F("SST 39SF010"));
        OSCR::UI::printSize(OSCR::Strings::Common::ROM, 128UL * 1024UL, true);
        OSCR::UI::update();
      }
      else
      {
        cartOff();
        OSCR::UI::printLabel(OSCR::Strings::Common::ID);
        OSCR::UI::printHexLine(flashid);
        OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
        return;
      }

      // Get name, add extension and convert to char array for sd lib
      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GameBoy), FS(OSCR::Strings::Directory::ROM), "Gameshark");

      // Initialize progress bar
      OSCR::UI::ProgressBar::init((uint32_t)(romBanks)*8192);

      for (size_t workBank = 0; workBank < romBanks; workBank++) // Loop over banks
      {

        startAddress = 0x4000;

        writeByte(0x7FE1, (workBank & 0xFF));

        // Read banks and save to SD
        while (startAddress <= finalAddress)
        {
          for (int i = 0; i < 512; i++)
          {
            OSCR::Storage::Shared::buffer[i] = readByte(startAddress + i);
          }

          OSCR::Storage::Shared::writeBuffer();

          startAddress += 512;

          OSCR::UI::ProgressBar::advance(512);
        }
      }

      cartOff();

      OSCR::UI::ProgressBar::finish();

      OSCR::Storage::Shared::close();
    }

    /****************************************************
      Datel GBC Gameshark Gameboy Device Write Function
    ****************************************************/
    // Write Datel GBC Gameshark Device
    void writeGameshark()
    {
      cartOn();

      // Enable Rom Banks
      readByte(0x101);
      readByte(0x108);
      readByte(0x101);
      delay(100);

      // SST 39SF010 ID command sequence
      sendGamesharkCommand(0x90);
      delay(10);

      // Read the two id bytes into a string
      writeByte(0x7FE1, 0x0);
      flashid = readByte(0x4000) << 8;
      flashid |= readByte(0x4001);

      // SST 39SF010 Flash ID Mode Exit
      sendGamesharkCommand(0xF0);

      if (flashid != 0xBFB5)
      {
        OSCR::UI::printLabel(OSCR::Strings::Common::ID);
        OSCR::UI::printHexLine(flashid);
        OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
        return;
      }

      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

      uint8_t byte1;
      bool toggle = true;

      if (flashid == 0xBFB5)
      {
        OSCR::UI::printLine(F("SST 39SF010"));
        romBanks = 16;

        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

        // Erase flash
        sendGamesharkCommand(0x80);
        sendGamesharkCommand(0x10);
        delay(100);
      }

      // Blankcheck
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

      // Read x number of banks
      for (uint16_t currBank = 0; currBank < romBanks; currBank++)
      {
        // Set ROM bank
        writeByte(0x7FE1, currBank);

        for (uint16_t currAddr = 0x4000; currAddr < 0x6000; currAddr += 0x200)
        {
          for (int currByte = 0; currByte < 512; currByte++)
          {
            OSCR::Storage::Shared::buffer[currByte] = readByte(currAddr + currByte);
          }

          for (int j = 0; j < 512; j++)
          {
            if (OSCR::Storage::Shared::buffer[j] != 0xFF)
            {
              cartOff();
              OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
              return;
            }
          }
        }
      }

      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

      // Write flash
      uint16_t currAddr = 0x4000;
      uint16_t endAddr = 0x5FFF;

      //Initialize progress bar
      OSCR::UI::ProgressBar::init((uint32_t)(romBanks)*8192);

      for (uint16_t currBank = 0; currBank < romBanks; currBank++)
      {
        currAddr = 0x4000;

        while (currAddr <= endAddr)
        {
          OSCR::Storage::Shared::fill();

          for (int currByte = 0; currByte < 512; currByte++)
          {
            // Write command sequence
            sendGamesharkCommand(0xA0);

            // Set ROM bank
            writeByte(0x7FE1, currBank);

            toggle = true;

            // Write current byte
            writeByte(currAddr + currByte, OSCR::Storage::Shared::buffer[currByte]);

            while (toggle)
            {
              byte1 = readByte(currAddr + currByte);

              if (byte1 == OSCR::Storage::Shared::buffer[currByte])
              {
                toggle = false;
              }
            }
          }

          currAddr += 512;
          OSCR::UI::ProgressBar::advance(512);
        }
      }

      OSCR::UI::ProgressBar::finish();

      OSCR::UI::clear();
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

      OSCR::Storage::Shared::rewind();

      writeErrors = 0;

      // Verify flashrom
      uint16_t romAddress = 0x4000;

      // Read number of banks and switch banks
      for (uint16_t bank = 0; bank < romBanks; bank++)
      {
        writeByte(0x7FE1, bank);  // Set ROM bank
        romAddress = 0x4000;

        // Read up to 1FFF per bank
        while (romAddress < 0x6000)
        {
          // Fill OSCR::Storage::Shared::buffer
          OSCR::Storage::Shared::fill();

          // Compare
          for (int i = 0; i < 512; i++)
          {
            if (readByte(romAddress + i) != OSCR::Storage::Shared::buffer[i])
            {
              writeErrors++;
            }
          }

          romAddress += 512;
        }
      }

      cartOff();

      OSCR::Storage::Shared::close();

      if (writeErrors == 0)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
      }
      else
      {
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
      }
    }
  } /* namespace OSCR::Cores::GameBoy */
} /* namespace OSCR::Cores */

#endif /* ENABLE_GBX */
