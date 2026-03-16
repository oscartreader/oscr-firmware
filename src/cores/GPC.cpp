//******************************************
// SNES Game Processor RAM Cassette code by LuigiBlood
// Revision 1.0.0 February 2024
//******************************************
#include "config.h"

#if HAS_GPC
# include "cores/include.h"
# include "cores/GPC.h"

/******************************************
   Game Processor RAM Cassette
******************************************/
namespace OSCR::Cores::GPC
{
  /******************************************
    Menu
  *****************************************/
  // GPC flash menu items
  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::Read,
    OSCR::Strings::MenuOptions::Write,
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::GPC), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Read ram
        readRAM();
        break;

      case 1: // Write ram
        if (!writeRAM()) continue;
        break;

      case 2: // Back
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::GPC));
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
#endif

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
    DDRC = 0xFF;

    OSCR::Cores::SNES::controlOut();

    PORTL = myBank;
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
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Switch WR(PH5) to LOW
    PORTH &= ~(1 << 5);

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
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Switch WR(PH5) to HIGH
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

  // Read one byte of data from a location specified by bank and address, 00:0000
  uint8_t readBank(uint8_t myBank, uint16_t myAddress)
  {
    DDRC = 0x00;
    PORTC = 0xFF;

    OSCR::Cores::SNES::controlIn();

    PORTL = myBank;
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;

    // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
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
    );

    // Read
    uint8_t tempByte = PINC;
    return tempByte;
  }

  /******************************************
     Game Processor RAM Cassette functions
  *****************************************/
  // Read RAM cassette to SD card
  void readRAM()
  {
    printHeader();

    setOutName_P(PSTR("GPC4M"));

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SNESD), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SNES));

    cartOn();

    // Draw progress bar
    OSCR::UI::setLineRel(1); // place it below the status line
    OSCR::UI::ProgressBar::init(0x80000);
    OSCR::UI::setLineRel(-1); // go back to status line

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    // Read Banks
    for (uint8_t currBank = 0xC0; currBank < 0xC8; currBank++)
    {
      for (uint32_t currByte = 0; currByte < 65536; currByte += 512)
      {
        for (uint16_t c = 0; c < 512; c++)
        {
          OSCR::Storage::Shared::buffer[c] = readBank(currBank, currByte + c);
        }

        OSCR::Storage::Shared::writeBuffer();

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::ProgressBar::finish();
  }

  // false = don't waitButton();
  bool writeRAM()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();

    if (OSCR::Storage::Shared::getSize() != 0x80000)
    {
      OSCR::Storage::Shared::close();

      OSCR::UI::error(FS(OSCR::Strings::Errors::IncorrectFileSize));
      //OSCR::Lang::printBytesLine(0x80000);

      return false;
    }

    cartOn();

    // Draw progress bar
    OSCR::UI::setLineRel(2); // place it 2 lines down (for Writing and Verifying)
    OSCR::UI::ProgressBar::init(0x80000 * 2); // * 2 (writing and then verifying)
    OSCR::UI::setLineRel(-2); // go back

    // Disable ram cassette write protection
    for (int countProtect = 0; countProtect < 15; countProtect++)
    {
      writeBank(0x20, 0x6000, 0x00);
    }

    // Write ram
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    for (uint8_t currBank = 0xC0; currBank < 0xC8; currBank++)
    {
      for (uint32_t currByte = 0x0000; currByte < 0x10000; currByte += 512)
      {
        OSCR::Storage::Shared::fill();

        for (uint16_t c = 0; c < 512; c++)
        {
          writeBank(currBank, currByte + c, OSCR::Storage::Shared::buffer[c]);
        }

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    // Reenable write protection
    uint8_t keepByte = readBank(0x20, 0x6000);

    delay(100);

    writeBank(0x20, 0x6000, keepByte);

    delay(100);

    OSCR::Storage::Shared::rewind();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    //startBank = 0xC0; endBank = 0xC7; CS low
    for (uint8_t currBank = 0xC0; currBank < 0xC8; currBank++)
    {
      //startAddr = 0x0000
      for (uint32_t currByte = 0x0000; currByte < 0x10000; currByte += 512)
      {
        OSCR::Storage::Shared::fill();

        for (uint16_t c = 0; c < 512; c++)
        {
          if ((readBank(currBank, currByte + c)) != OSCR::Storage::Shared::buffer[c])
          {
            writeErrors++;
          }
        }

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS((writeErrors) ? OSCR::Strings::Common::FAIL : OSCR::Strings::Common::OK));

    OSCR::UI::ProgressBar::finish();

    if (writeErrors)
    {
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
      return false;
    }

    return true;
  }
} /* namespace OSCR::Cores::GPC */

#endif /* HAS_GPC */
