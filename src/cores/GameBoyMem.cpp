//******************************************
// GB MEMORY MODULE
//******************************************
#include "config.h"

#if HAS_GBX
# include "cores/include.h"
# include "cores/GameBoyMem.h"

namespace OSCR::Cores::GameBoyMem
{
# if HAS_FLASH
  using OSCR::Cores::Flash::flashid;
# endif /* HAS_FLASH */

  /******************************************
    Menu
  *****************************************/
  // GBM menu items
  constexpr char const PROGMEM gbmMenuItem1[] = "Read ID";
  constexpr char const PROGMEM gbmMenuItem6[] = "Read Mapping";
  constexpr char const PROGMEM gbmMenuItem7[] = "Write Mapping";

  constexpr char const * const PROGMEM menuOptionsGBM[] = {
    gbmMenuItem1,
    OSCR::Strings::MenuOptions::Read,
    OSCR::Strings::MenuOptions::Erase,
    OSCR::Strings::Headings::BlankCheck,
    OSCR::Strings::MenuOptions::Write,
    gbmMenuItem6,
    gbmMenuItem7,
  };

  void menu()
  {
    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::GBMemoryModule), menuOptionsGBM, sizeofarray(menuOptionsGBM)))
      {
#if HAS_FLASH
      case 0: // Read Flash ID
        readFlashID();
        break;
#endif

      case 1: // Read Flash
        readROM();
        break;

#if HAS_FLASH
      case 2: // Erase Flash
        if (!OSCR::Prompts::confirmErase()) continue;
        eraseFlash();
        break;

      // Blankcheck Flash
      case 3:
        blankcheckFlash();
        break;

      case 4: // Write Flash
        writeFlash();
        break;

      case 5: // Read mapping
        readMapping();
        break;

      case 6: // Write mapping
        if (!OSCR::Prompts::confirmErase()) continue;
        writeMapping();
        break;
#endif
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

    // Set RST(PH0) to Input
    DDRH &= ~(1 << 0);
    // Activate Internal Pullup Resistors
    PORTH |= (1 << 0);

    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;

    // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
    DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
    // Output a high signal on all pins, pins are active low therefore everything is disabled now
    PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;

    delay(400);

    // Check for Nintendo Power GB Memory cart
    uint8_t timeout = 0;

    // First byte of NP register is always 0x21
    while (readByte(0x120) != 0x21)
    {
      // Enable ports 0x120h (F2)
      send(0x09);

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

      timeout++;

      if (timeout > 10)
      {
        OSCR::UI::error(FS(OSCR::Strings::Errors::TimedOut));
      }
    }
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::GBMemoryModule));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  /**********************
    LOW LEVEL
  **********************/
  // Read one word out of the cartridge
  uint8_t readByte(uint16_t myAddress)
  {
    // Set data pins to Input
    DDRC = 0x0;
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Switch CS(PH3) and RD(PH6) to LOW
    PORTH &= ~(1 << 3);
    PORTH &= ~(1 << 6);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Read
    uint8_t tempByte = PINC;

    // Switch CS(PH3) and RD(PH6) to HIGH
    PORTH |= (1 << 6);
    PORTH |= (1 << 3);

    return tempByte;
  }

  // Write one word to data pins of the cartridge
  void writeByte(uint16_t myAddress, uint8_t myData)
  {
    // Set data pins to Output
    DDRC = 0xFF;
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTC = myData;

    // Pull CS(PH3) and write(PH5) low
    PORTH &= ~(1 << 3);
    PORTH &= ~(1 << 5);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Pull CS(PH3) and write(PH5) high
    PORTH |= (1 << 5);
    PORTH |= (1 << 3);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Set data pins to Input (or read errors??!)
    DDRC = 0x0;
  }

  /**********************
    HELPER FUNCTIONS
  **********************/
  void printSdBuffer(uint16_t startByte, uint16_t numBytes)
  {
    for (uint16_t currByte = 0; currByte < numBytes; currByte += 10)
    {
      for (uint8_t c = 0; c < 10; c++)
      {
        OSCR::UI::printHex<false>(OSCR::Storage::Shared::buffer[startByte + currByte + c]);
      }

      // Add a new line every 10 bytes
      OSCR::UI::printLine();
    }

    OSCR::UI::update();
  }

  void readROM()
  {
    constexpr uint16_t const numBanks = 64;

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GBMemoryModule), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    // Enable access to ports 0120h
    send(0x09);

    // Map entire flashrom
    send(0x04);

    // Disable ports 0x0120...
    send(0x08);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

    for (uint16_t currBank = 1; currBank < numBanks; currBank++)
    {
      // Set rom bank
      writeByte(0x2100, currBank);

      for (uint16_t currAddress = (currBank > 1) ? 0x4000 : 0; currAddress < 0x7FFF; currAddress += 512)
      {
        for (int currByte = 0; currByte < 512; currByte++)
        {
          OSCR::Storage::Shared::buffer[currByte] = readByte(currAddress + currByte);
        }

        OSCR::Storage::Shared::writeBuffer();
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();
  }

  /**********************
    GB Memory Functions
  **********************/
  void send(uint8_t myCommand)
  {
    switch (myCommand)
    {
    case 0x01:
      //CMD_01h -> ???
      writeByte(0x0120, 0x01);
      writeByte(0x013F, 0xA5);
      break;

    case 0x02:
      //CMD_02h -> Write enable Step 2
      writeByte(0x0120, 0x02);
      writeByte(0x013F, 0xA5);
      break;

    case 0x03:
      //CMD_03h -> Undo write Step 2
      writeByte(0x0120, 0x03);
      writeByte(0x013F, 0xA5);
      break;

    case 0x04:
      //CMD_04h -> Disable mapping; makes the entire flash and SRAM accessible
      writeByte(0x0120, 0x04);
      writeByte(0x013F, 0xA5);
      break;

    case 0x05:
      //CMD_05h -> Enable mapping; re-enables the mapping that was previously selected by 0xCn
      writeByte(0x0120, 0x05);
      writeByte(0x013F, 0xA5);
      break;

    case 0x08:
      //CMD_08h -> disable writes/reads to/from special Nintendo Power registers (those at 0120h..013Fh)
      writeByte(0x0120, 0x08);
      writeByte(0x013F, 0xA5);
      break;

    case 0x09:
      //CMD_09h Wakeup -> re-enable access to ports 0120h..013Fh
      writeByte(0x0120, 0x09);
      writeByte(0x0121, 0xAA);
      writeByte(0x0122, 0x55);
      writeByte(0x013F, 0xA5);
      break;

    case 0x0A:
      //CMD_0Ah -> Write enable Step 1
      writeByte(0x0120, 0x0A);
      writeByte(0x0125, 0x62);
      writeByte(0x0126, 0x04);
      writeByte(0x013F, 0xA5);
      break;

    case 0x10:
      //CMD_10h -> disable writes to normal MBC registers (such like 2100h)
      writeByte(0x0120, 0x10);
      writeByte(0x013F, 0xA5);
      break;

    case 0x11:
      //CMD_11h -> re-enable access to MBC registers like 0x2100
      writeByte(0x0120, 0x11);
      writeByte(0x013F, 0xA5);
      break;

    default:
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::UnknownType));
      break;
    }
  }

# if HAS_FLASH
  void send(uint8_t myCommand, uint16_t myAddress, uint8_t myData)
  {
    uint8_t myAddrLow = myAddress & 0xFF;
    uint8_t myAddrHigh = (myAddress >> 8) & 0xFF;

    switch (myCommand)
    {
    case 0x0F:
      // CMD_0Fh -> Write address/byte to flash
      writeByte(0x0120, 0x0F);
      writeByte(0x0125, myAddrHigh);
      writeByte(0x0126, myAddrLow);
      writeByte(0x0127, myData);
      writeByte(0x013F, 0xA5);
      break;

    default:
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::UnknownType));
      break;
    }
  }

  void sendSequence(uint8_t myData)
  {
    send(0x0F, 0x5555, 0xAA);
    send(0x0F, 0x2AAA, 0x55);
    send(0x0F, 0x5555, myData);
  }

  void switchGame(uint8_t myData)
  {
    // Enable ports 0x0120 (F2)
    send(0x09);

    // Enable mapping. Mapping must be enabled before the C0 cmd, otherwise switching has no effect.
    send(0x05);

    //CMD_C0h -> map selected game without reset
    //           C0 is the menu or a single 1MB sized game
    //           C1 is the first game entry
    //           C2 is the second game entry...
    writeByte(0x0120, 0xC0 | myData);
    writeByte(0x013F, 0xA5);
  }

  void resetFlash()
  {
    // Enable ports 0x0120 (F2)
    send(0x09);

    // Send reset command
    writeByte(0x2100, 0x01);
    sendSequence(0xF0);
    delay(100);
  }

  bool readFlashID()
  {
    cartOn();

    // Enable ports 0x0120 (F2)
    send(0x09);

    writeByte(0x2100, 0x01);

    // Read ID command
    sendSequence(0x90);

    // Read the two id bytes into a string
    flashid = readByte(0) << 8;
    flashid |= readByte(1);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ID));
    OSCR::UI::printHexLine(flashid);

    resetFlash();

    cartOff();

    return (flashid == 0xC289);
  }

  void eraseFlash()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    cartOn();

    //enable access to ports 0120h
    send(0x09);

    // Enable write
    send(0x0A);
    send(0x2);

    // Unprotect sector 0
    sendSequence(0x60);
    sendSequence(0x40);

    // Wait for unprotect to complete
    while ((readByte(0) & 0x80) != 0x80);

    // Send erase command
    sendSequence(0x80);
    sendSequence(0x10);

    // Wait for erase to complete
    while ((readByte(0) & 0x80) != 0x80);

    // Reset flashrom
    resetFlash();

    cartOff();
  }

  bool blankcheckFlash()
  {
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    cartOn();

    //enable access to ports 0120h (F2)
    send(0x09);

    // Map entire flashrom
    send(0x04);
    // Disable ports 0x0120...
    send(0x08);

    // Read rom
    uint16_t currAddress = 0;

    for (uint8_t currBank = 1; currBank < 64; currBank++)
    {
      // Set rom bank
      writeByte(0x2100, currBank);

      // Switch bank start address
      if (currBank > 1)
      {
        currAddress = 0x4000;
      }

      for (; currAddress < 0x7FFF; currAddress++)
      {
        if (readByte(currAddress) != 0xFF)
        {
          cartOff();
          OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
          return 0;
        }
      }
    }

    cartOff();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    return 1;
  }

  void writeFlash()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    // Get rom size from file
    fileSize = OSCR::Storage::Shared::getSize();

    if ((fileSize / 0x4000) > 64)
    {
      OSCR::UI::error(FS(OSCR::Strings::Errors::NotLargeEnough));
      return;
    }

    cartOn();

    // Enable access to ports 0120h
    send(0x09);
    // Enable write
    send(0x0A);
    send(0x2);

    // Map entire flash rom
    send(0x4);

    // Set bank for unprotect command, writes to 0x5555 need odd bank number
    writeByte(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send(0x10);
    send(0x08);

    // Unprotect sector 0
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0x60);
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0x40);

    // Check if flashrom is ready for writing or busy
    while ((readByte(0) & 0x80) != 0x80);

    // first bank: 0x0000-0x7FFF,
    uint16_t currAddress = 0x0;

    // Write 63 banks
    for (uint8_t currBank = 0x1; currBank < (fileSize / 0x4000); currBank++)
    {
      // all following banks: 0x4000-0x7FFF
      if (currBank > 1)
      {
        currAddress = 0x4000;
      }

      // Write single bank in 128 byte steps
      for (; currAddress < 0x7FFF; currAddress += 128)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::readBuffer(128);

        // Enable access to ports 0x120 and 0x2100
        send(0x09);
        send(0x11);

        // Set bank
        writeByte(0x2100, 0x1);

        // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
        send(0x10);
        send(0x08);

        // Write flash buffer command
        writeByte(0x5555, 0xAA);
        writeByte(0x2AAA, 0x55);
        writeByte(0x5555, 0xA0);

        // Wait until flashrom is ready again
        while ((readByte(0) & 0x80) != 0x80);

        // Enable access to ports 0x120 and 0x2100
        send(0x09);
        send(0x11);

        // Set bank
        writeByte(0x2100, currBank);

        // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
        send(0x10);
        send(0x08);

        // Fill flash buffer
        for (uint16_t currByte = 0; currByte < 128; currByte++)
        {
          writeByte(currAddress + currByte, OSCR::Storage::Shared::buffer[currByte]);
        }

        // Execute write
        writeByte(currAddress + 127, 0xFF);

        // Wait for write to complete
        while ((readByte(currAddress) & 0x80) != 0x80);
      }
    }

    cartOff();

    // Close the file:
    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void readMapping()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::GBMemoryModule), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Map));

    cartOn();

    // Enable ports 0x0120
    send(0x09);

    // Set WE and WP
    send(0x0A);
    send(0x2);

    // Enable hidden mapping area
    writeByte(0x2100, 0x01);
    sendSequence(0x77);
    sendSequence(0x77);

    // Read mapping
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

    for (uint8_t currByte = 0; currByte < 128; currByte++)
    {
      OSCR::Storage::Shared::buffer[currByte] = readByte(currByte);
    }

    OSCR::Storage::Shared::writeBuffer(128);

    // Reset flash to leave hidden mapping area
    resetFlash();

    cartOff();

    // Close the file:
    OSCR::Storage::Shared::close();

    // Signal end of process
    printSdBuffer(0, 20);
    printSdBuffer(102, 20);
  }

  bool eraseMapping()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    //enable access to ports 0120h
    send(0x09);
    // Enable write
    send(0x0A);
    send(0x2);

    // Unprotect sector 0
    sendSequence(0x60);
    sendSequence(0x40);

    // Wait for unprotect to complete
    while ((readByte(0) & 0x80) != 0x80);

    // Send erase command
    sendSequence(0x60);
    sendSequence(0x04);

    // Wait for erase to complete
    while ((readByte(0) & 0x80) != 0x80);

    // Reset flashrom
    resetFlash();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    // Enable ports 0x0120
    send(0x09);

    // Set WE and WP
    send(0x0A);
    send(0x2);

    // Enable hidden mapping area
    writeByte(0x2100, 0x01);
    sendSequence(0x77);
    sendSequence(0x77);

    // Disable ports 0x0120...
    send(0x08);

    // Read rom
    for (uint8_t currByte = 0; currByte < 128; currByte++)
    {
      if (readByte(currByte) != 0xFF)
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
        return false;
      }
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    return true;
  }

  void writeMapping()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    cartOn();

    // Erase mapping
    if (!eraseMapping()) return;

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    // Get map file size and check if it exceeds 128KByte
    if (OSCR::Storage::Shared::getSize() > 0x80)
    {
      OSCR::UI::error(FS(OSCR::Strings::Errors::NotLargeEnough));
      return;
    }

    // Enable access to ports 0120h
    send(0x09);

    // Enable write
    send(0x0A);
    send(0x2);

    // Map entire flash rom
    send(0x4);

    // Set bank, writes to 0x5555 need odd bank number
    writeByte(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send(0x10);
    send(0x08);

    // Unlock write to map area
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0x60);
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0xE0);

    // Check if flashrom is ready for writing or busy
    while ((readByte(0) & 0x80) != 0x80);

    // Fill SD buffer
    OSCR::Storage::Shared::readBuffer(128);

    // Enable access to ports 0x120 and 0x2100
    send(0x09);
    send(0x11);

    // Set bank
    writeByte(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send(0x10);
    send(0x08);

    // Write flash buffer command
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0xA0);

    // Wait until flashrom is ready again
    while ((readByte(0) & 0x80) != 0x80);

    // Enable access to ports 0x120 and 0x2100
    send(0x09);
    send(0x11);

    // Set bank
    writeByte(0x2100, 0);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send(0x10);
    send(0x08);

    // Fill flash buffer
    for (uint16_t currByte = 0; currByte < 128; currByte++)
    {
      writeByte(currByte, OSCR::Storage::Shared::buffer[currByte]);
    }

    // Execute write
    writeByte(127, 0xFF);

    cartOff();

    // Close the file:
    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }
  #endif
} /* namespace OSCR::Cores::GameBoyMem */

#endif /* HAS_GBX */
