//******************************************
// SNES Satellaview 8M Memory pack code by tamanegi_taro
// Revision 1.0.0 October 22nd 2018
// Added BSX Sram, copied from skamans enhanced sketch //sanni
//******************************************
#include "config.h"

#if HAS_SV
# include "cores/include.h"
# include "cores/Satellaview.h"

/******************************************
   Satellaview 8M Memory Pack
******************************************/
namespace OSCR::Cores::Satellaview
{
  /******************************************
     Variables
  *****************************************/
  //No global variables

  /******************************************
    Menu
  *****************************************/
  // SV flash menu items
  constexpr char const PROGMEM menuOption1[] = "Read Memory Pack";
  constexpr char const PROGMEM menuOption2[] = "Write Memory Pack";
  constexpr char const PROGMEM menuOption3[] = "Read BS-X Sram";
  constexpr char const PROGMEM menuOption4[] = "Write BS-X Sram";

  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::WriteFlash,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::Satellaview), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Read memory pack
        readROM();
        break;

      case 1: // Write memory pack
        writeFlash();
        break;

      case 2: // Read BS-X Sram
        readSRAM();
        break;

      case 3: // Write BS-X Sram
        writeSRAM();
        break;

      case 4: // Back
        resetGlobals();
        return;
      }

      OSCR::UI::waitButton();
    }
    while(true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::Satellaview));
  }

  /******************************************
     Setup
  *****************************************/
  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set cicrstPin(PG1) to Output
    DDRG |= (1 << 1);
    // Output a high signal until we're ready to start
    PORTG |= (1 << 1);
    // Set cichstPin(PG0) to Input
    DDRG &= ~(1 << 0);

#if defined(ENABLE_CLOCKGEN)
    if (!OSCR::ClockGen::initialize())
    {
      OSCR::UI::clear();
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::ClockGenMissing));
    }

    OSCR::ClockGen::clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
    OSCR::ClockGen::clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
    OSCR::ClockGen::clockgen.set_freq(2147727200ULL, SI5351_CLK0);
    OSCR::ClockGen::clockgen.set_freq(307200000ULL, SI5351_CLK2);
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK0, 1);
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 0);
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK2, 1);
#endif /* ENABLE_CLOCKGEN */

    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;
    //BA0-BA7
    DDRL = 0xFF;
    //PA0-PA7
    DDRA = 0xFF;

    // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
    DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
    // Switch RST(PH0) and WR(PH5) to HIGH
    PORTH |= (1 << 0) | (1 << 5);
    // Switch CS(PH3) and RD(PH6) to LOW
    PORTH &= ~((1 << 3) | (1 << 6));

    // Set Refresh(PE5) to Output
    DDRE |= (1 << 5);
    // Switch Refresh(PE5) to LOW (needed for SA-1)
    PORTE &= ~(1 << 5);

    // Set CPU Clock(PH1) to Output
    DDRH |= (1 << 1);
    //PORTH &= ~(1 << 1);

    // Set IRQ(PH4) to Input
    DDRH &= ~(1 << 4);
    // Activate Internal Pullup Resistors
    //PORTH |= (1 << 4);

    // Set expand(PG5) to output
    DDRG |= (1 << 5);
    // Output High
    PORTG |= (1 << 5);

    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
    // Enable Internal Pullups
    //PORTC = 0xFF;

    // Unused pins
    // Set wram(PE4) to Output
    DDRE |= (1 << 4);
    //PORTE &= ~(1 << 4);
    // Set pawr(PJ1) to Output
    DDRJ |= (1 << 1);
    //PORTJ &= ~(1 << 1);
    // Set pard(PJ0) to Output
    DDRJ |= (1 << 0);
    //PORTJ &= ~(1 << 0);

    // Start CIC by outputting a low signal to cicrstPin(PG1)
    PORTG &= ~(1 << 1);

    // Wait for CIC reset
    delay(1000);
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  /******************************************
     Low level functions
  *****************************************/
  // Write one byte of data to a location specified by bank and address, 00:0000
  void writeBank(uint8_t myBank, uint16_t myAddress, uint8_t myData)
  {
    // Set pins to output
    OSCR::Cores::SNES::dataOut();

    // Set RST RD WR to High and CS to Low
    OSCR::Cores::SNES::controlOut();

    PORTL = myBank;
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    PORTC = myData;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    // Wait till output is stable
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Switch WR(PH5) to LOW
    PORTH &= ~(1 << 5);

    // Leave WR low for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Switch WR(PH5) to HIGH
    PORTH |= (1 << 5);

    // Leave WR high for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  }

  // Read one byte of data from a location specified by bank and address, 00:0000
  uint8_t readBank(uint8_t myBank, uint16_t myAddress)
  {
    OSCR::Cores::SNES::dataIn();
    OSCR::Cores::SNES::controlIn();

    PORTL = myBank;
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Read
    uint8_t tempByte = PINC;
    return tempByte;
  }

  /******************************************
     SatellaView BS-X Sram functions
  *****************************************/
  void readSRAM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SNES), FS(OSCR::Strings::Directory::Save), "BSX", FS(OSCR::Strings::FileType::SaveRAM));

    cartOn();

    readBank(0x10, 0);  // Preconfigure to fix corrupt 1st byte

    //startBank = 0x10; endBank = 0x17; CS low
    for (uint8_t BSBank = 0x10; BSBank < 0x18; BSBank++)
    {
      //startAddr = 0x5000
      for (uint16_t currByte = 0x5000; currByte < 0x6000; currByte += 512)
      {
        for (uint16_t c = 0; c < 512; c++)
        {
          OSCR::Storage::Shared::buffer[c] = readBank(BSBank, currByte + c);
        }

        OSCR::Storage::Shared::writeBuffer();
      }
    }

    OSCR::Storage::Shared::close();
  }

  void writeSRAM()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    OSCR::UI::ProgressBar::init((uint32_t)(32768));

    // Write to sram bank
    for (uint8_t currBank = 0x10; currBank < 0x18; currBank++)
    {
      //startAddr = 0x5000
      for (long currByte = 0x5000; currByte < 0x6000; currByte += 512)
      {
        OSCR::Storage::Shared::fill();

        for (uint32_t c = 0; c < 512; c++)
        {
          writeBank(currBank, currByte + c, OSCR::Storage::Shared::buffer[c]);
        }

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    // Finish progressbar
    OSCR::UI::ProgressBar::finish();

    delay(100);

    // Close the file:
    OSCR::Storage::Shared::close();

    writeErrors = 0;

    //startBank = 0x10; endBank = 0x17; CS low
    for (uint8_t BSBank = 0x10; BSBank < 0x18; BSBank++)
    {
      for (long currByte = 0x5000; currByte < 0x6000; currByte += 512)
      {
        OSCR::Storage::Shared::fill();

        for (uint32_t c = 0; c < 512; c++)
        {
          if ((readBank(BSBank, currByte + c)) != OSCR::Storage::Shared::buffer[c])
          {
            writeErrors++;
          }
        }
      }
    }

    // Close the file:
    OSCR::Storage::Shared::close();

    if (writeErrors != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }
  }

  /******************************************
     SatellaView 8M Memory Pack functions
  *****************************************/
  // Read memory pack to SD card
  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SNES), FS(OSCR::Strings::Directory::ROM), "MEMPACK", FS(OSCR::Strings::FileType::Satellaview));

    cartOn();

    OSCR::UI::ProgressBar::init(0x100000);

    // Read Banks
    for (int currBank = 0x40; currBank < 0x50; currBank++)
    {
      for (long currByte = 0; currByte < 65536; currByte += 512)
      {
        for (int c = 0; c < 512; c++)
        {
          OSCR::Storage::Shared::buffer[c] = readBank(currBank, currByte + c);
        }

        OSCR::Storage::Shared::writeBuffer();

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::ProgressBar::finish();

    OSCR::UI::printLine(F("Read pack completed"));
  }

  void writeFlash(void)
  {
    //if CRC is not 8B86, BS-X cart is not inserted. Display error and reset
    if (readBank(0, 65503) != 0x8B || readBank(0, 65502) != 0x86)
    {
      OSCR::UI::fatalError(F("Error: Must use BS-X cart"));
    }

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();

    if (OSCR::Storage::Shared::getSize() != 0x100000)
    {
      OSCR::Storage::Shared::close();

      OSCR::UI::error(FS(OSCR::Strings::Errors::IncorrectFileSize));
      //OSCR::UI::printLine(FS(OSCR::Strings::Units::Size1MB));

      return;
    }

    cartOn();

    // Jump ahead
    OSCR::UI::setLineRel(4);
    // Create the progress bar...
    OSCR::UI::ProgressBar::init((uint32_t)(0x300000));
    // Go back
    OSCR::UI::setLineRel(-4);

    //Disable 8M memory pack write protection
    writeBank(0x0C, 0x5000, 0x80);  //Modify write enable register
    writeBank(0x0E, 0x5000, 0x80);  //Commit register modification

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    eraseAll();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    // Blank check

    OSCR::UI::print(FS(OSCR::Strings::Status::Checking));

    for (int currBank = 0xC0; currBank < 0xD0; currBank++)
    {
      for (long currByte = 0; currByte < 0x10000; currByte++)
      {
        if (0xFF != readBank(currBank, currByte))
        {
          OSCR::Storage::Shared::close();
          OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
          return;
        }
      }

      OSCR::UI::ProgressBar::advance(0x10000);
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    for (int currBank = 0xC0; currBank < 0xD0; currBank++)
    {
      for (long currByte = 0; currByte < 0x10000; currByte++)
      {
        writeBank(0xC0, 0x0000, 0x10);  //Program Byte
        writeBank(currBank, currByte, OSCR::Storage::Shared::sharedFile.read());
        writeCheck();
      }

      OSCR::UI::ProgressBar::advance(0x10000);
    }

    writeCheck();

    writeBank(0xC0, 0x0000, 0xFF);  //Terminate write

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::Storage::Shared::rewind();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    for (int currBank = 0xC0; currBank < 0xD0; currBank++)
    {
      for (long currByte = 0; currByte < 0x10000; currByte++)
      {
        if (OSCR::Storage::Shared::sharedFile.read() != readBank(currBank, currByte))
        {
          OSCR::Storage::Shared::close();

          OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
          return;
        }
      }

      OSCR::UI::ProgressBar::advance(0x10000);
    }

    // Close the file:
    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

    OSCR::UI::ProgressBar::finish();
  }

  void busyWait(uint16_t address, uint8_t flag)
  {
    do
    {
      if ((readBank(0xC0, address) & flag)) return;

      // CE or OE must be toggled with each subsequent status read or the
      // completion of a program or erase operation will not be evident.

      OSCR::Cores::SNES::controlOut();

      // Switch CS(PH3) High
      PORTH |= (1 << 3);

      // Leave CE high for at least 60ns
      __asm__("nop\n\t"
              "nop\n\t"
              "nop\n\t"
              "nop\n\t"
              "nop\n\t"
              "nop\n\t");

      OSCR::Cores::SNES::controlIn();

      // Leave CE low for at least 50ns
      __asm__("nop\n\t"
              "nop\n\t"
              "nop\n\t"
              "nop\n\t"
              "nop\n\t"
              "nop\n\t");
    }
    while (true);
  }

  void writeCheck(void)
  {
    writeBank(0xC0, 0x0000, 0x70); // Status Mode
    busyWait(0x0000, 0x80);
  }

  void detectCheck(void)
  {
    busyWait(0x0002, 0x80);
  }

  void eraseAll(void)
  {
    writeBank(0xC0, 0x0000, 0x50); //Clear Status Registers
    writeBank(0xC0, 0x0000, 0x71); //Status Mode
    busyWait(0x0004, 0x08); //supplyCheck();
    writeBank(0xC0, 0x0000, 0xA7); //Chip Erase
    writeBank(0xC0, 0x0000, 0xD0); //Confirm
    writeBank(0xC0, 0x0000, 0x71); //Status Mode
    busyWait(0x0004, 0x80); //eraseCheck();
    writeBank(0xC0, 0x0000, 0xFF);  //Teriminate
  }
} /* namespace OSCR::Cores::Satellaview */

#endif /* HAS_SV */
