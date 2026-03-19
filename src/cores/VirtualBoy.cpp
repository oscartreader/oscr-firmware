//******************************************
// VIRTUALBOY MODULE
//******************************************
#include "config.h"

#if HAS_VBOY
# include "cores/include.h"
# include "cores/VirtualBoy.h"

namespace OSCR::Cores::VirtualBoy
{
  // Nintendo VirtualBoy
  // Cartridge Pinout
  // 60P 2.00mm pitch connector
  //
  //                 TOP SIDE             BOTTOM SIDE
  //                           +-------+
  //                      GND -| 1   2 |- GND
  // /WE0 (SRAM WRITE ENABLE) -| 3   4 |- nc
  //                       nc -| 5   6 |- /CS1 (SRAM ENABLE)
  //               CS2 (SRAM) -| 7   8 |- VCC(+5V)
  //                       nc -| 9  10 |- A23
  //                      A19 -| 11 12 |- A22
  //                      A18 -| 13 14 |- A21
  //                       A8 -| 15 16 |- A20
  //                       A7 -| 17 18 |- A9
  //                       A6 -| 19 20 |- A10
  //                       A5 -| 21 22 |- A11
  //                       A4 -| 23 24 |- A12
  //                       A3 -| 25 26 |- A13
  //                       A2 -| 27 28 |- A14
  //                       A1 -| 29 30 |- A15
  //                /CE (ROM) -| 31 32 |- A16
  //                      GND -| 33 34 |- A17
  //                      /OE -| 35 36 |- VCC(+5V)
  //                       D8 -| 37 38 |- D7
  //                       D0 -| 39 40 |- D15
  //                       D9 -| 41 42 |- D6
  //                       D1 -| 43 44 |- D14
  //                      D10 -| 45 46 |- D5
  //                       D2 -| 47 48 |- D13
  //                      D11 -| 49 50 |- D4
  //                       D3 -| 51 52 |- D12
  //                 VCC(+5V) -| 53 54 |- VCC(+5V)
  //                       nc -| 55 56 |- nc
  //                       nc -| 57 58 |- nc
  //                      GND -| 59 60 |- GND
  //                           +-------+
  //

  // CONTROL PINS:
  // CS2(SRAM)               - (PH0) - VBOY PIN 7  - SNES RST
  // /CE(ROM)                - (PH3) - VBOY PIN 31 - SNES /CS
  // /CS1(SRAM ENABLE)       - (PH4) - VBOY PIN 6  - SNES /IRQ
  // /WE0(SRAM WRITE ENABLE) - (PH5) - VBOY PIN 3  - SNES /WR
  // /OE                     - (PH6) - VBOY PIN 35 - SNES /RD

  // NOT CONNECTED:
  // CLK(PH1) - N/C

  //******************************************
  //  MENU
  //******************************************

  // Manual Rom size selection Menu
  constexpr char const * const PROGMEM romSizeOptions[] = {
    OSCR::Strings::Units::Size64KB,
    OSCR::Strings::Units::Size512KB,
    OSCR::Strings::Units::Size1MB,
    OSCR::Strings::Units::Size2MB,
    OSCR::Strings::Units::Size4MB,
  };

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

      setRomSize();

      break; // Only reached when continue is selected above.
    }

    menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadROM);
    menuOptionMap[option] = MenuOption::ReadROM;
    option++;

    if (sramSize)
    {
      menuOptions[option] = FS(OSCR::Strings::MenuOptions::ReadSave);
      menuOptionMap[option] = MenuOption::ReadSave;
      option++;

      menuOptions[option] = FS(OSCR::Strings::MenuOptions::WriteSave);
      menuOptionMap[option] = MenuOption::WriteSave;
      option++;
    }

    // Not needed since it prompts when the cart is unknown,
    //  left here for ease of use when debugging/adding new
    //  carts.
    //menuOptions[option] = FS(OSCR::Strings::MenuOptions::SetSize);
    //menuOptionMap[option] = MenuOption::SetSize;
    //option++;

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

    // Print cart details (includes `waitButton()`)
    printDetails();

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

      case MenuOption::SetSize:
        setRomSize();
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

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::VirtualBoy));
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
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //A16-A23
    DDRL = 0xFF;

    // Set Control Pins to Output
    //      CS2(PH0)   ---(PH1)   /CE(PH3)   /CS1(PH4)  /WE0(PH5)  /OE(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |= (1 << 0);

    // Set Pins (D0-D15) to Input
    DDRC = 0x00;
    DDRA = 0x00;

    // Setting Control Pins to HIGH
    //       ---(PH1)   /CE(PH3)   /CS1(PH4)  /WE0(PH5)  /OE(PH6)
    PORTH |= (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    // Set CS2(PH0) to LOW
    PORTH &= ~(1 << 0);

    // Set Unused Pins HIGH
    PORTJ |= (1 << 0);  // TIME(PJ0)
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::VirtualBoy));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  //  LOW LEVEL FUNCTIONS
  //******************************************

  void writeByte(uint32_t myAddress, uint8_t myData)
  {
    DDRC = 0xFF;
    DDRA = 0xFF;

    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;

    __asm__("nop\n\t");

    PORTA = myData;

    // Set CS2(PH0), /CE(PH3), /OE(PH6) to HIGH
    PORTH |= (1 << 0) | (1 << 3) | (1 << 6);
    // Set /CS1(PH4), /WE0(PH5) to LOW
    PORTH &= ~(1 << 4) & ~(1 << 5);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Set CS2(PH0), /CS1(PH4), /WE0(PH5) to HIGH
    PORTH |= (1 << 0) | (1 << 4) | (1 << 5);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  }

  uint16_t readWord(uint32_t myAddress)
  {
    DDRC = 0x00;
    DDRA = 0x00;

    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;

    __asm__("nop\n\t");

    // Set CS2(PH0), /CS1(PH4), /WE0(PH5) to HIGH
    PORTH |= (1 << 0) | (1 << 4) | (1 << 5);
    // Set /CE(PH3), /OE(PH6) to LOW
    PORTH &= ~(1 << 3) & ~(1 << 6);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    uint16_t tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

    // Set /CE(PH3), /OE(PH6) to HIGH
    PORTH |= (1 << 3) | (1 << 6);
    // Setting CS2(PH0) LOW
    PORTH &= ~(1 << 0);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    return tempWord;
  }

  // SRAM BYTE
  uint8_t readByte(uint32_t myAddress)
  {
    DDRC = 0x00;
    DDRA = 0x00;

    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTL = (myAddress >> 16) & 0xFF;

    __asm__("nop\n\t");

    // Set CS2(PH0), /CE(PH3), /WE0(PH5) to HIGH
    PORTH |= (1 << 0) | (1 << 3) | (1 << 5);
    // Set /CS1(PH4), /OE(PH6) to LOW
    PORTH &= ~(1 << 4) & ~(1 << 6);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    uint8_t tempByte = PINA;

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Set /CS1(PH4), /OE(PH6) to HIGH
    PORTH |= (1 << 3) | (1 << 6);
    // Setting CS2(PH0) LOW
    PORTH &= ~(1 << 0);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    return tempByte;
  }

  //******************************************
  // CART INFO
  //******************************************

  bool checkCart()
  {
    cartSize = 0;
    sramSize = 0;

    useDefaultName();

    cartOn();

    for (uint32_t address = 0x10000; address <= 0x400000; address *= 2)
    {
      // Get Serial
      uint16_t vbSerial = readWord((address - 0x204) / 2);  // Cart Serial

      // Get Rom/Sram sizes for known serials
      switch (vbSerial)
      {
      case 0x3431:           // 41 = Robonaut
        cartSize = 0x10000;  // 64KB
        break;

      case 0x4D54:           // MT = Mario's Tennis
      case 0x4832:           // H2 = Panic Bomber/Tobidase! Panibomb
      case 0x5350:           // SP = Space Invaders
      case 0x5353:           // SS = Space Squash
      case 0x5452:           // TR = V-Tetris
        cartSize = 0x80000;  // 512KB
        break;

      case 0x494D:            // IM = Insmouse no Yakata
      case 0x4A42:            // JB = Jack Bros.
      case 0x4D43:            // MC = Mario Clash
      case 0x5245:            // RE = Red Alarm
      case 0x4833:            // H3 = Vertical Force
      case 0x5642:            // VB = Virtual Bowling
      case 0x4A56:            // JV = Virtual Lab
      case 0x5650:            // VP = Virtual League Baseball/Virtual Pro Yakyuu '95
        cartSize = 0x100000;  // 1MB
        break;

      case 0x5042:            // PB = 3-D Tetris
      case 0x4750:            // GP = Galactic Pinball
      case 0x5344:            // SD = SD Gundam Dimension War
      case 0x5442:            // TB = Teleroboxer
      case 0x5646:            // VF = Virtual Fishing
        cartSize = 0x100000;  // 1MB
        sramSize = 0x2000;    // 8KB
        break;

      case 0x5647:            // VG = Golf/T&E Virtual Golf
      case 0x4E46:            // NF = Nester's Funky Bowling
      case 0x5745:            // WE = Waterworld
        cartSize = 0x200000;  // 2MB
        break;

      case 0x5743:            // WC = Virtual Boy Wario Land
        cartSize = 0x200000;  // 2MB
        sramSize = 0x2000;    // 8KB
        break;

      case 0x4644:            // FD = Hyper Fighting
      case 0x575A:            // WZ = Virtual WarZone
        cartSize = 0x400000;  // 4MB
        sramSize = 0x2000;    // 8KB
        break;
      }

      if (cartSize)
        break;
    }

    uint32_t const nameAddress = (cartSize - 0x220) / 2;

    // Get name
    for (uint8_t c = 0; c < 20; c += 2)
    {
      uint16_t myWord = readWord(nameAddress + (c / 2));
      OSCR::Storage::Shared::buffer[c] = myWord & 0xFF;
      OSCR::Storage::Shared::buffer[c + 1] = myWord >> 8;
    }

    cartOff();

    setOutName(((char *)OSCR::Storage::Shared::buffer), 20);

    return (!!cartSize);
  }

  void printDetails()
  {
    printHeader();

    OSCR::UI::printValue(OSCR::Strings::Common::Name, fileName);

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, cartSize * 8);

    if (sramSize)
    {
      OSCR::UI::printSize(OSCR::Strings::Common::Save, sramSize * 8);
    }

    OSCR::UI::waitButton();
  }

  //******************************************
  // READ CODE
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::VirtualBoy), FS(OSCR::Strings::Directory::ROM), fileName);

    cartOn();

    OSCR::UI::ProgressBar::init((uint32_t)(cartSize), 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    // HYPER FIGHTING FIX
    // VIRTUAL BOY ADDRESSING IS TOP DOWN
    // ONLY FOR HYPER FIGHTING PLUGIN WITH ALL ADDRESS LINES CONNECTED
    // NORMAL PLUGIN DOES NOT HAVE THE UPPER ADDRESS LINES CONNECTED

    romSize = cartSize / 2;

    uint8_t const startAddress = (cartSize == 0x400000) ? 0x1000000 - romSize : 0;
    uint8_t const endAddress = (cartSize == 0x400000) ? 0x1000000 : romSize;

    for (uint32_t blockAddress = startAddress; blockAddress < endAddress; blockAddress += 256)
    {
      for (uint16_t addrOffset = 0, d = 0; addrOffset < 256; addrOffset++, d+=2)
      {
        uint16_t myWord = readWord(blockAddress + addrOffset);
        OSCR::Storage::Shared::buffer[d] = ((myWord >> 8) & 0xFF);
        OSCR::Storage::Shared::buffer[d + 1] = (myWord & 0xFF);
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::ProgressBar::finish();

    OSCR::Databases::Basic::matchCRC();
  }

  //******************************************
  // SRAM
  //******************************************

  void writeSRAM()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    cartOn();

    printHeader();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    for (uint32_t addr = 0; addr < sramSize; addr += 512)
    {
      uint16_t const bufferSize = OSCR::Storage::Shared::fill();

      for (uint16_t i = 0; i < bufferSize; i++)
      {
        writeByte(addr + i, OSCR::Storage::Shared::buffer[i]);
      }
    }

    OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));

    // Power cycle before verifying.
    cartOff();
    delay(500); // Wait
    cartOn();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    OSCR::Storage::Shared::rewind();

    writeErrors = 0;

    for (uint32_t addr = 0; addr < sramSize; addr += 512)
    {
      uint16_t const bufferSize = OSCR::Storage::Shared::fill();

      for (uint16_t i = 0; i < bufferSize; i++)
      {
        if (readByte(addr + i) != OSCR::Storage::Shared::buffer[i])
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
      return;
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
  }

  void readSRAM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SuperAcan), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveRAM));

    cartOn();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    for (uint32_t currBuffer = 0; currBuffer < sramSize; currBuffer += 512)
    {
      for (int currByte = 0; currByte < 512; currByte++)
      {
        uint8_t myByte = readByte(currBuffer + currByte);
        OSCR::Storage::Shared::buffer[currByte] = myByte;
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
  }

  //******************************************
  // SET ROM SIZE MANUALLY
  //******************************************

  void setRomSize()
  {
    switch (OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetROMSize), romSizeOptions, sizeofarray(romSizeOptions)))
    {
    case 0:
      cartSize = 0x10000;    // 64KB
      break;

    case 1:
      cartSize = 0x80000;    // 512KB
      break;

    case 2:
      cartSize = 0x100000;   // 1MB
      break;

    case 3:
      cartSize = 0x200000;   // 2MB
      break;

    case 4:
      cartSize = 0x400000;   // 4MB
      break;
    }
  }
} /* namespace OSCR::Cores::VirtualBoy */

#endif /* HAS_VBOY */
