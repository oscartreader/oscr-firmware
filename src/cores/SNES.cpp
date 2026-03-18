//******************************************
// SUPER NINTENDO MODULE
//******************************************
#include "config.h"

#if HAS_SNES
# include "cores/include.h"
# include "cores/SNES.h"
# include "cores/Flash.h"

namespace OSCR::Strings::SNES
{
  constexpr char const PROGMEM SlowROM[]  = "SlowROM";
  constexpr char const PROGMEM FastROM[]  = "FastROM";
  constexpr char const PROGMEM HiROM[]    = "HiROM";
  constexpr char const PROGMEM LoROM[]    = "LoROM";
  constexpr char const PROGMEM ExHiRom[]  = "ExHiRom";

  constexpr char const PROGMEM DSP1[]     = "DSP1";
  constexpr char const PROGMEM DSP2[]     = "DSP2";
  constexpr char const PROGMEM SDD1[]     = "SDD1";
  constexpr char const PROGMEM SRTC[]     = "SRTC";
  constexpr char const PROGMEM BATT[]     = "BATT";
  constexpr char const PROGMEM SPC[]      = "SPC";
  constexpr char const PROGMEM RTClock[]  = "RTC";
  constexpr char const PROGMEM SA1[]      = "SA1";

  constexpr char const PROGMEM ICTemplate3[] = "%S %S %S";
  constexpr char const * const PROGMEM ICTemplate2 = ICTemplate3 + 3;
  constexpr char const * const PROGMEM ICTemplate1 = ICTemplate2 + 3;
  //constexpr char const PROGMEM ICTemplate2[] = "%S %S";
  //constexpr char const PROGMEM ICTemplate1[] = "%S";
}

namespace OSCR::Cores::SNES
{
  /******************************************
    Defines
   *****************************************/
  // SNES Hi and LoRom, SA is HI with different Sram dumping
# define EX 4
# define SA 3
# define HI 1
# define LO 0

  /******************************************
     Variables
  *****************************************/
  // Define SNES Cart Reader Variables
  uint16_t romSpeed = 0;  // 0 = SlowROM, 3 = FastROM
  uint16_t romChips = 0;  // 0 = ROM only, 1 = ROM & RAM, 2 = ROM & Save RAM,  3 = ROM & DSP1, 4 = ROM & RAM & DSP1, 5 = ROM & Save RAM & DSP1, 19 = ROM & SFX
  // 227 = ROM & RAM & GameBoy data, 243 = CX4, 246 = ROM & DSP2
  uint8_t romSizeExp = 0;  // ROM-Size Exponent
  bool isNintendoPower = false;
  uint8_t cx4Type = 0;
  uint8_t cx4Map = 0;

  crdbSNESRecord * romDetail;
  OSCR::Databases::SNESRecord * romRecord = nullptr;

  using OSCR::Cores::Flash::mapping;

  /******************************************
    Menu
  *****************************************/
  // SNES/Nintendo Power SF Memory start menu
  constexpr char const PROGMEM menuOption01[] = "SNES/SFC cartridge";
  constexpr char const PROGMEM menuOption02[] = "SF Memory Cassette";
  constexpr char const PROGMEM menuOption03[] = "Satellaview BS-X";
  constexpr char const PROGMEM menuOption04[] = "Sufami Turbo";
  constexpr char const PROGMEM menuOption05[] = "Game Processor RAM";
  constexpr char const PROGMEM menuOption06[] = "Flash repro";
  constexpr char const PROGMEM menuOption07[] = "Calibrate Clock";

  enum class MenuOption : uint8_t
  {
    Cart,
# if defined(ENABLE_SFM)
    SFM,
# endif /* ENABLE_SFM */
# if defined(ENABLE_SV)
    SV,
# endif /* ENABLE_SV */
# if defined(ENABLE_ST)
    ST,
# endif /* ENABLE_ST */
# if defined(ENABLE_GPC)
    GPC,
# endif /* ENABLE_GPC */
# if HAS_FLASH8
    Flash,
# endif /* HAS_FLASH8 */
# if defined(OPTION_CLOCKGEN_CALIBRATION)
    Calibration,
# endif /* OPTION_CLOCKGEN_CALIBRATION */
    Back,
  };

  constexpr char const * const PROGMEM menuOptions[] = {
    menuOption01,
# if defined(ENABLE_SFM)
    menuOption02,
# endif /* ENABLE_SFM */
# if defined(ENABLE_SV)
    menuOption03,
# endif /* ENABLE_SV */
# if defined(ENABLE_ST)
    menuOption04,
# endif /* ENABLE_ST */
# if defined(ENABLE_GPC)
    menuOption05,
# endif /* ENABLE_GPC */
# if HAS_FLASH8
    menuOption06,
# endif /* HAS_FLASH8 */
# if defined(OPTION_CLOCKGEN_CALIBRATION)
    menuOption07,
# endif /* OPTION_CLOCKGEN_CALIBRATION */
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr uint8_t const kMenuOptionCount = sizeofarray(menuOptions);

  // SNES menu items
  constexpr char const PROGMEM snesMenuOption04[] = "Test RAM";

  enum class SNESMenuOption : uint8_t
  {
    ReadROM,
    ReadSave,
    WriteSave,
    TestRAM,
    Refresh,
    Browser,
    Back,
  };

  constexpr char const * const PROGMEM menuOptionsSNES[] = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    snesMenuOption04,
    OSCR::Strings::MenuOptions::RefreshCart,
    OSCR::Strings::MenuOptions::SetCartType,
    OSCR::Strings::MenuOptions::Back,
  };

  // Manual config menu items
  constexpr char const PROGMEM confMenuItem1[] = "Use Header Info";
  constexpr char const PROGMEM confMenuItem2[] = "4MB LoROM 256K SRAM";
  constexpr char const PROGMEM confMenuItem3[] = "4MB HiROM 64K SRAM";
  constexpr char const PROGMEM confMenuItem4[] = "6MB ExROM 256K SRAM";

  constexpr char const * const PROGMEM menuOptionsConfManual[] = {
    confMenuItem1,
    confMenuItem2,
    confMenuItem3,
    confMenuItem4,
    OSCR::Strings::MenuOptions::Back,
  };

# if HAS_FLASH8
  // Repro menu items
  constexpr char const PROGMEM reproMenuItem1[] = "CFI LoROM";
  constexpr char const PROGMEM reproMenuItem2[] = "CFI HiROM";
  constexpr char const PROGMEM reproMenuItem3[] = "LoROM (P0)";
  constexpr char const PROGMEM reproMenuItem4[] = "HiROM (P0)";
  constexpr char const PROGMEM reproMenuItem5[] = "ExLoROM (P1)";
  constexpr char const PROGMEM reproMenuItem6[] = "ExHiROM (P1)";

  constexpr char const * const PROGMEM menuOptionsRepro[] = {
    reproMenuItem1,
    reproMenuItem2,
    reproMenuItem3,
    reproMenuItem4,
    reproMenuItem5,
    reproMenuItem6,
    OSCR::Strings::MenuOptions::Back,
  };

  // CFI ROM config
  constexpr char const PROGMEM reproCFIItem1[] = "1x 2MB";
  constexpr char const PROGMEM reproCFIItem2[] = "2x 2MB";
  constexpr char const PROGMEM reproCFIItem3[] = "1x 4MB";
  constexpr char const PROGMEM reproCFIItem4[] = "2x 4MB";
  constexpr char const PROGMEM reproCFIItem5[] = "4x 2MB";
  constexpr char const PROGMEM reproCFIItem6[] = "1x 8MB";

  constexpr char const * const PROGMEM menuOptionsReproCFI[] = {
    reproCFIItem1,
    reproCFIItem2,
    reproCFIItem3,
    reproCFIItem4,
    reproCFIItem5,
    reproCFIItem6,
    OSCR::Strings::MenuOptions::Back,
  };

  // Setup number of flashroms
  void reproCFIMenu()
  {
    do
    {
      constexpr bool const reversed = true;

      switch (OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectType), menuOptionsReproCFI, sizeofarray(menuOptionsReproCFI)))
      {
      case 0:
        OSCR::Cores::Flash::flashSize = 2097152;

        printHeader();
        OSCR::Cores::Flash::writeCFI(1, 1, 0);
        OSCR::Cores::Flash::verifyFlash(1, 1, 0);
        break;

      case 1:
        OSCR::Cores::Flash::flashSize = 4194304;

        // Write first rom chip
        printHeader();
        OSCR::Cores::Flash::writeCFI(1, 2, 0);
        OSCR::Cores::Flash::verifyFlash(1, 2, 0);

        // Write second rom chip
        nextCFI();

        printHeader();
        OSCR::Cores::Flash::writeCFI(2, 2, 0);
        OSCR::Cores::Flash::verifyFlash(2, 2, 0);
        break;

      case 2:
        OSCR::Cores::Flash::flashSize = 4194304;
        OSCR::Cores::Flash::writeCFI(1, 1, 0);
        OSCR::Cores::Flash::verifyFlash(1, 1, 0);
        break;

      case 3:
        OSCR::Cores::Flash::flashSize = 8388608;

        // Write first rom chip
        printHeader();
        OSCR::Cores::Flash::writeCFI(1, 2, reversed);
        OSCR::Cores::Flash::verifyFlash(1, 2, reversed);

        // Write second rom chip
        nextCFI();

        printHeader();
        OSCR::Cores::Flash::writeCFI(2, 2, reversed);
        OSCR::Cores::Flash::verifyFlash(2, 2, reversed);

        break;

      case 4:
        OSCR::Cores::Flash::flashSize = 8388608;

        for (uint8_t chip = 1; chip < 5; chip++)
        {
          printHeader();
          OSCR::Cores::Flash::writeCFI(chip, 4, reversed);
          OSCR::Cores::Flash::verifyFlash(chip, 4, reversed);
          nextCFI();
        }
        break;

      case 5:
        OSCR::Cores::Flash::flashSize = 8388608;

        printHeader();
        OSCR::Cores::Flash::writeCFI(1, 1, reversed);
        OSCR::Cores::Flash::verifyFlash(1, 1, reversed);
        break;

      case 6:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void nextCFI()
  {
    delay(300);

    // Switch to next ROM chip
    switch (mapping)
    {
    // Chip 1 -> 2
    case   1: mapping = 122; return; // LoROM
    case   2: mapping = 222; return; // HiROM

    // Chip 2 -> 3
    case 122: mapping = 124; return; // LoROM
    case 222: mapping = 224; return; // HiROM

    // Chip 3 -> 4
    case 124: mapping = 142; return; // LoROM
    case 224: mapping = 242; return; // HiROM

    // Chip 4
    case 142:
    case 242:
      // Done
      return;
    }
  }

  // SNES repro menu
  void reproMenu()
  {
    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectType), menuOptionsRepro, sizeofarray(menuOptionsRepro)))
      {
      case 0: // CFI LoROM
        mapping = 1;
        reproCFIMenu();
        break;

      case 1: // CFI HiROM
        mapping = 2;
        reproCFIMenu();
        break;

      case 2: // LoRom
        mapping = 1;
        OSCR::Cores::Flash::setup8();
        OSCR::Cores::Flash::id8();
        OSCR::Cores::Flash::menuFlash8();
        break;

      case 3: // HiRom
        mapping = 2;
        OSCR::Cores::Flash::setup8();
        OSCR::Cores::Flash::id8();
        OSCR::Cores::Flash::menuFlash8();
        break;

      case 4: // ExLoRom
        mapping = 124;
        OSCR::Cores::Flash::setup8();
        OSCR::Cores::Flash::id8();
        OSCR::Cores::Flash::menuFlash8();
        break;

      case 5: // ExHiRom
        mapping = 224;
        OSCR::Cores::Flash::setup8();
        OSCR::Cores::Flash::id8();
        OSCR::Cores::Flash::menuFlash8();
        break;

      case 6:
        return;
      }
    }
    while (true);
  }
# endif /* HAS_FLASH8 */

  // SNES start menu
  void menu()
  {
    do
    {
      if __if_constexpr (kMenuOptionCount == 2)
      {
        snesMenu();
        return;
      }

      switch (static_cast<MenuOption>(OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetCartType), menuOptions, kMenuOptionCount)))
      {
      case MenuOption::Cart:
        snesMenu();
        continue;

# if defined(ENABLE_SFM)
      case MenuOption::SFM:
        OSCR::Cores::SFM::menu();
        continue;
# endif /* ENABLE_SFM */

# if defined(ENABLE_SV)
      case MenuOption::SV:
        OSCR::Cores::Satellaview::menu();
        continue;
# endif /* ENABLE_SV */

# if defined(ENABLE_ST)
      case MenuOption::ST:
        OSCR::Cores::ST::menu();
        continue;
# endif /* ENABLE_ST */

# if defined(ENABLE_GPC)
      case MenuOption::GPC:
        OSCR::Cores::GPC::menu();
        continue;
# endif /* ENABLE_GPC */

# if HAS_FLASH8
      case MenuOption::Flash:
        OSCR::Cores::Flash::setupVoltage();
        reproMenu();
        continue;
# endif /* HAS_FLASH8 */

# if defined(OPTION_CLOCKGEN_CALIBRATION)
      case MenuOption::Calibration:
        OSCR::ClockGen::clkcal();
        continue;
# endif /* OPTION_CLOCKGEN_CALIBRATION */

      case MenuOption::Back:
        return;
      }
    }
    while (true);
  }

  // SNES Menu
  void snesMenu()
  {
    openCRDB();

    getCartInfo();

    do
    {
      switch (static_cast<SNESMenuOption>(OSCR::UI::menu(FS(OSCR::Strings::Cores::SNES), menuOptionsSNES, sizeofarray(menuOptionsSNES))))
      {
      case SNESMenuOption::ReadROM:
        if (numBanks < 1)
        {
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        // start reading from cart
        readROM();

        // Internal Checksum
        compare_checksum();

        // CRC32
        matchCRC();
        break;

      case SNESMenuOption::ReadSave:
        if (sramSize < 1)
        {
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        readSRAM();

        break;

      case SNESMenuOption::WriteSave:
        if (sramSize < 1)
        {
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        writeSRAM(1);

        OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

        writeErrors = verifySRAM();

        if (writeErrors > 0)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
          OSCR::UI::printLine();
          OSCR::Lang::printErrorVerifyBytes(writeErrors);
          continue;
        }

        OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
        break;

      case SNESMenuOption::TestRAM:
        if (sramSize < 1)
        {
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }

        if (!OSCR::Prompts::confirmErase()) continue;

        readSRAM();

        eraseSRAM(0x0F);
        eraseSRAM(0xF0);

        writeSRAM(0);

        OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

        writeErrors = verifySRAM();

        if (writeErrors > 0)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
          OSCR::UI::printLine();
          OSCR::Lang::printErrorVerifyBytes(writeErrors);
          continue;
        }

        OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
        break;

      case SNESMenuOption::Refresh:
#if defined(ENABLE_ONBOARD_ATMEGA)
        getCartInfo();
        continue;
#else /* !ENABLE_ONBOARD_ATMEGA */
        // For arcademaster1 (Markfrizb) multi-game carts
        // Set reset pin to output (PH0)
        DDRH |= (1 << 0);
        // Switch RST(PH0) to LOW
        PORTH &= ~(1 << 0);

        resetArduino();
#endif /* ENABLE_ONBOARD_ATMEGA */

      case SNESMenuOption::Browser:
        confMenuManual(false);
        continue;

      case SNESMenuOption::Back:
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  // Menu for manual configuration
  void confMenuManual(bool disallowBack)
  {
    switch (OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectMapper), menuOptionsConfManual, sizeofarray(menuOptionsConfManual) - disallowBack))
    {
    case 0:
      return;

    case 1:
      romType = LO;
      numBanks = 128;
      sramSize = 256;
      strcpy_P(fileName, OSCR::Strings::SNES::LoROM);
      return;

    case 2:
      romType = HI;
      numBanks = 64;
      sramSize = 64;
      strcpy_P(fileName, OSCR::Strings::SNES::HiROM);
      return;

    case 3:
      romType = EX;
      numBanks = 96;
      sramSize = 256;
      strcpy_P(fileName, OSCR::Strings::SNES::ExHiRom);
      return;

    case 4: // Back
      return;
    }
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::SNES));
  }

  /******************************************
     Setup
  *****************************************/
  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

# if defined(ENABLE_CLOCKGEN)
    // Clock Generator
    //if (!OSCR::ClockGen::initialize())
    //{
    //  OSCR::UI::fatalError(FS(OSCR::Strings::Errors::ClockGenMissing));
    //}

    // Set clocks to 4Mhz/1Mhz for better SA-1 unlocking
    OSCR::ClockGen::clockgen.set_freq(100000000ULL, SI5351_CLK1);  // CPU
    OSCR::ClockGen::clockgen.set_freq(100000000ULL, SI5351_CLK2);  // CIC
    OSCR::ClockGen::clockgen.set_freq(400000000ULL, SI5351_CLK0);  // EXT

    // Start outputting master clock, CIC clock
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 0);  // no CPU clock yet; seems to affect SA-1 success a lot
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK2, 1);  // CIC clock (should go before master clock)
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK0, 1);  // master clock

    // Wait for clock generator
    OSCR::ClockGen::clockgen.update_status();

    delay(500);
# endif /* ENABLE_CLOCKGEN */

    // Set cicrstPin(PG1) to Output
    DDRG |= (1 << 1);
    // Output a high signal until we're ready to start
    PORTG |= (1 << 1);
    // Set cichstPin(PG0) to Input
    DDRG &= ~(1 << 0);

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

    // Set expand(PG5) to Input
    DDRG &= ~(1 << 5);
    // Activate Internal Pullup Resistors
    //PORTG |= (1 << 5);

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

    // ClockGen was here

    // Start CIC by outputting a low signal to cicrstPin(PG1)
    PORTG &= ~(1 << 1);

    // Wait for CIC reset
    delay(500);

# if defined(ENABLE_CLOCKGEN)
    //Set clocks to standard or else SA-1 sram writing will fail
    OSCR::ClockGen::clockgen.set_freq(2147727200ULL, SI5351_CLK0);
    OSCR::ClockGen::clockgen.set_freq(357954500ULL, SI5351_CLK1);
    OSCR::ClockGen::clockgen.set_freq(307200000ULL, SI5351_CLK2);
# endif /* ENABLE_CLOCKGEN */
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    cartCRDB = new OSCR::Databases::SNES(FS(OSCR::Strings::FileType::SNES));
  }

  void closeCRDB()
  {
    if (cartCRDB == nullptr) return;
    delete snesCRDB;
    resetGlobals();
  }

  /******************************************
     I/O Functions
  *****************************************/

  // Switch data pins to read
  void dataIn()
  {
    // Set to Input and activate pull-up resistors
    DDRC = 0x00;
    // Pullups
    PORTC = 0xFF;
  }

  // Switch data pins to write
  void dataOut()
  {
    DDRC = 0xFF;
  }

  // Switch control pins to write
  void controlOut()
  {
    // Switch RD(PH6) and WR(PH5) to HIGH
    PORTH |= (1 << 6) | (1 << 5);
    // Switch CS(PH3) to LOW
    PORTH &= ~(1 << 3);
  }

  // Switch control pins to read
  void controlIn()
  {
    // Switch WR(PH5) to HIGH
    PORTH |= (1 << 5);
    // Switch CS(PH3) and RD(PH6) to LOW
    PORTH &= ~((1 << 3) | (1 << 6));
  }

  /******************************************
     Low level functions
  *****************************************/
  // Write one byte of data to a location specified by bank and address, 00:0000
  void writeBank(uint8_t myBank, uint16_t myAddress, uint8_t myData)
  {
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
    PORTL = myBank;
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;

    // Wait for the Byte to appear on the data bus
    // Arduino running at 16Mhz -> one nop = 62.5ns
    // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
    // let's be conservative and use 6 x 62.5 = 375ns
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    // Read
    return (uint8_t)PINC;
  }

  void readLoRomBanks(uint16_t start, uint16_t total, OSCR::Storage::File * file)
  {
    uint8_t buffer[1024] = { 0 };
    uint16_t currByte = 32768;

    // Initialize progress bar
    OSCR::UI::ProgressBar::init((total - start) * 1024);

    for (uint16_t currBank = start; currBank < total; currBank++)
    {
      PORTL = currBank;

      currByte = 32768;

      do
      {
        for (uint16_t c = 0; c < 1024; c++, currByte++)
        {
          PORTF = (currByte & 0xFF);
          PORTK = ((currByte >> 8) & 0xFF);

          // Wait for the Byte to appear on the data bus
          // Arduino running at 16Mhz -> one nop = 62.5ns
          // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
          // let's be conservative and use 6 x 62.5 = 375ns
          NOP;
          NOP;
          NOP;
          NOP;
          NOP;
          NOP;

          buffer[c] = PINC;
        }

        file->write(buffer, 1024);
      }
      while (currByte != 0);

      // update progress bar
      OSCR::UI::ProgressBar::advance(1024);
    }

    OSCR::UI::ProgressBar::finish();
  }

  void readHiRomBanks(uint16_t start, uint16_t total, OSCR::Storage::File * file)
  {
    uint8_t buffer[1024] = { 0 };
    uint16_t currByte = 0;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((total - start) * 1024);

    for (uint16_t currBank = start; currBank < total; currBank++)
    {
      PORTL = currBank;

      currByte = 0;

      do
      {
        for (uint16_t c = 0; c < 1024; c++, currByte++)
        {
          PORTF = (currByte & 0xFF);
          PORTK = ((currByte >> 8) & 0xFF);

          // Wait for the Byte to appear on the data bus
          // Arduino running at 16Mhz -> one nop = 62.5ns
          // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
          // let's be conservative and use 6 x 62.5 = 375ns
          NOP;
          NOP;
          NOP;
          NOP;
          NOP;
          NOP;

          buffer[c] = PINC;
        }

        file->write(buffer, 1024);
      }
      while (currByte != 0);

      // update progress bar
      OSCR::UI::ProgressBar::advance(1024);
    }

    OSCR::UI::ProgressBar::finish();
  }

  /******************************************
    SNES ROM Functions
  ******************************************/
  void getCartInfo()
  {
    bool manualConfig = 0;

    cartOn();

    //Prime SA1 cartridge
    PORTL = 192;

    for (uint16_t currByte = 0; currByte < 1024; currByte++)
    {
      PORTF = currByte & 0xFF;
      PORTK = currByte >> 8;

      // Wait for the Byte to appear on the data bus
      // Arduino running at 16Mhz -> one nop = 62.5ns
      // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
      // let's be conservative and use 6 x 62.5 = 375ns
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
    }

    // Print start page
    while (checkcart() == 0)
    {
      // Checksum either corrupt or 0000

      cartOff();

      switch (OSCR::Prompts::abortRetryContinue())
      {
      case OSCR::Prompts::AbortRetryContinue::Abort: return;
      case OSCR::Prompts::AbortRetryContinue::Continue: break;
      case OSCR::Prompts::AbortRetryContinue::Retry:
        cartOn();
        continue;
      }

      // Continuing... (breaking out)
      manualConfig = 1;
      break;
    }

    cartOff();

    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::NAME));
    OSCR::UI::printLine(fileName);

    OSCR::UI::print(FS(OSCR::Strings::Labels::REVISION));
    OSCR::UI::printLine(romVersion);

    OSCR::UI::print(FS(OSCR::Strings::Labels::TYPE));

    if (romType == HI)
      OSCR::UI::print(FS(OSCR::Strings::SNES::HiROM));
    else if (romType == LO)
      OSCR::UI::print(FS(OSCR::Strings::SNES::LoROM));
    else if (romType == EX)
      OSCR::UI::print(FS(OSCR::Strings::SNES::ExHiRom));
    else
      OSCR::UI::print(romType);

    OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));

    if (romSpeed == 0)
      OSCR::UI::printLine(FS(OSCR::Strings::SNES::SlowROM));
    else if (romSpeed == 2)
      OSCR::UI::printLine(FS(OSCR::Strings::SNES::SlowROM));
    else if (romSpeed == 3)
      OSCR::UI::printLine(FS(OSCR::Strings::SNES::FastROM));
    else
      OSCR::UI::printLine(romSpeed);

    OSCR::UI::print(F("ICs: ROM "));

    char icType[20];

    switch (romChips)
    {
    case   0:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, PSTR("ONLY"));
      break;

    case   1:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, OSCR::Strings::Common::RAM);
      break;

    case   2:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, OSCR::Strings::Common::Save);
      break;

    case   3:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, OSCR::Strings::SNES::DSP1);
      break;

    case   4:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate2, OSCR::Strings::SNES::DSP1, OSCR::Strings::Common::RAM);
      break;

    case   5:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate2, OSCR::Strings::SNES::DSP1, OSCR::Strings::Common::Save);
      break;

    case  67:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, OSCR::Strings::SNES::SDD1);
      break;

    case  69:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate2, OSCR::Strings::SNES::SDD1, OSCR::Strings::SNES::BATT);
      break;

    case  85:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate3, OSCR::Strings::SNES::SDD1, OSCR::Strings::Common::RAM, OSCR::Strings::SNES::BATT);
      break;

    case 227:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate2, OSCR::Strings::Common::RAM, PSTR("GBoy"));
      break;

    case 243:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, PSTR("CX4"));
      break;

    case 246:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, OSCR::Strings::SNES::DSP2);
      break;

    case 245:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate3, OSCR::Strings::SNES::SPC, OSCR::Strings::Common::RAM, OSCR::Strings::SNES::BATT);
      break;

    case 249:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate3, OSCR::Strings::SNES::SPC, OSCR::Strings::Common::RAM, OSCR::Strings::SNES::RTClock);
      break;

    case  19:
    case  20:
    case  21:
    case  26:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, PSTR("SuperFX"));
      break;

    case  52:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate2, OSCR::Strings::SNES::SA1, OSCR::Strings::Common::RAM);
      romType = SA;
      break;

    case  53:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate3, OSCR::Strings::SNES::SA1, OSCR::Strings::Common::RAM, OSCR::Strings::SNES::BATT);
      romType = SA;
      break;

    default:
      snprintf_P(icType, sizeof(icType), OSCR::Strings::SNES::ICTemplate1, OSCR::Strings::Symbol::Space);
      break;
    }

    OSCR::UI::printLine(icType);

    OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    if ((romSize >> 3) < 1)
    {
      OSCR::Lang::printBytes((romSize * 1024) >> 3);
    }
    else
    {
      OSCR::Lang::printBytes((romSize >> 3) * 1024 * 1024);
    }

    OSCR::UI::print(FS(OSCR::Strings::Symbol::MenuSpaces));

    OSCR::UI::print(FS(OSCR::Strings::Labels::BANKS));
    OSCR::UI::printLine(numBanks);

    //OSCR::UI::print(F("Chips: "));
    //OSCR::UI::printLine(romChips);

    OSCR::UI::print(FS(OSCR::Strings::Labels::SAVE));
    OSCR::Lang::printBytes((sramSize >> 3) * 1024);

    OSCR::UI::print(FS(OSCR::Strings::Symbol::MenuSpaces));

    OSCR::UI::print(FS(OSCR::Strings::Labels::CHK));
    OSCR::UI::printHex(checksum);

    OSCR::UI::waitButton();

    // Start manual config
    if (manualConfig == 1)
    {
      confMenuManual(true);
    }
  }

  void checkAltConf(uint16_t chksumSearch, crc32_t & crc32search)
  {
    printHeader();
    OSCR::UI::print(FS(OSCR::Strings::Labels::CHECKSUM));
    OSCR::UI::printHexLine(checksum);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::SearchingDatabase));

    // Iterate over results
    while (snesCRDB->searchRecordNext(chksumSearch))
    {
      romRecord = snesCRDB->record();
      romDetail = romRecord->data();

      OSCR::UI::print(FS(OSCR::Strings::Labels::CRCSum));
      OSCR::UI::printLineSync(crc32search);

      // Read file size
      uint8_t romSize2 = romDetail->size / 1024 / 1024;

      // Read number of banks
      uint8_t numBanks2 = romDetail->banks;

      // Some games have the same checksum, so compare CRC32 of header area with database too
      if (crc32search == romDetail->id32)
      {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));

        // Game found, check if ROM sizes differ but only change ROM size if non- standard size found in database, else trust the header to be right and the database to be wrong
        if (((romSize != romSize2) || (numBanks != numBanks2)) && ((romSize2 == 10) || (romSize2 == 12) || (romSize2 == 20) || (romSize2 == 24) || (romSize2 == 40) || (romSize2 == 48)))
        {
          // Correct size
          OSCR::UI::print(FS(OSCR::Strings::Labels::SIZE));
          OSCR::UI::print(romSize);
          OSCR::UI::print(FS(OSCR::Strings::Symbol::Arrow));
          OSCR::Lang::printBytesLine(romDetail->size);

          OSCR::UI::print(FS(OSCR::Strings::Labels::BANKS));
          OSCR::UI::print(numBanks);
          OSCR::UI::print(FS(OSCR::Strings::Symbol::Arrow));
          OSCR::UI::printLineSync(numBanks2);

          delay(1000);

          romSize = romSize2;
          numBanks = numBanks2;
        }

        return;
      }

      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::UI::print(crc32search);
      OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
      OSCR::UI::printLineSync(romDetail->id32);

      delay(1000);
    }
  }

  // Read header
  bool checkcart()
  {
    uint16_t headerStart = 0xFFB0;
    uint8_t snesHeader[80];

    // set control to read
    dataIn();

    PORTL = 0;

    for (uint16_t c = 0, currByte = headerStart; c < 80; c++, currByte++)
    {
      PORTF = (currByte & 0xFF);
      PORTK = ((currByte >> 8) & 0xFF);

      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;

      snesHeader[c] = PINC;
    }

    // Calculate CRC32 of header
    crc32_t crc = crc32_t(OSCR::CRC32::calculateCRC(BUFFN(snesHeader)));

    checksum = (snesHeader[0xFFDF - headerStart] << 8) | (snesHeader[0xFFDE - headerStart]);

    romType = snesHeader[0xFFD5 - headerStart];

    if ((romType >> 5) != 1) // Detect invalid romType byte due to too long ROM name (22 chars)
    {
      romType = LO; // LoROM   // Krusty's Super Fun House (U) 1.0 & Contra 3 (U)
    }
    else if (romType == 0x35)
    {
      romType = EX;  // Check if ExHiROM
    }
    else if (romType == 0x3A)
    {
      romType = HI;                                 // Check if SPC7110
    }
    else if (0x3BB0 == checksum) // invalid romType due to too long ROM name (Yuyu no Quiz de GO!GO!)
    {
      romType = LO;
    }
    else
    {
      romType &= 1;  // Must be LoROM or HiROM
    }

    // Check RomSpeed
    romSpeed = (snesHeader[0xFFD5 - headerStart] >> 4);

    // Check RomChips
    romChips = snesHeader[0xFFD6 - headerStart];

    if (romChips == 69)
    {
      romSize = 48;
      numBanks = 96;
      romType = HI;
    }
    else if (romChips == 67)
    {
      romSize = 32;
      numBanks = 64;
      romType = HI;
    }
    else if (romChips == 243)
    {
      cx4Type = snesHeader[0xFFC9 - headerStart] & 0xF;
      if (cx4Type == 2) // X2
      {
        romSize = 12;
        numBanks = 48;
      }
      else if (cx4Type == 3) // X3
      {
        romSize = 16;
        numBanks = 64;
      }
    }
    else if ((romChips == 245) && (romType == HI))
    {
      romSize = 24;
      numBanks = 48;
    }
    else if ((romChips == 249) && (romType == HI))
    {
      romSize = 40;
      numBanks = 80;
    }
    else
    {
      // Check RomSize
      uint8_t romSizeExp = snesHeader[0xFFD7 - headerStart] - 7;
      romSize = 1;
      while (romSizeExp--)
        romSize *= 2;

      if ((romType == EX) || (romType == SA))
      {
        numBanks = long(romSize) * 2;
      }
      else
      {
        numBanks = (long(romSize) * 1024 * 1024 / 8) / (32768 + (long(romType) * 32768));
      }
    }

    //Check SD card for alt config, pass CRC32 of snesHeader but filter out 0000 and FFFF checksums
    if ((0x0000 != checksum) && (0xFFFF != checksum))
    {
      checkAltConf(checksum, crc);
    }

    // Get name
    uint8_t nameLen = setOutName((char *)&snesHeader[0xFFC0 - headerStart], 21);

    // If name consists out of all japanese characters use game code
    if (nameLen == 0)
    {
      char code[5];

      // Get rom code
      for (uint16_t i = 0; i < 4; i++)
      {
        uint8_t data = snesHeader[0xFFB2 + i - headerStart];

        if (OSCR::Util::isAlphaNumeric(data))
        {
          code[nameLen++] = data;
        }
      }

      if (nameLen == 0)
      {
        useDefaultName();
      }
      else
      {
        OSCR::Util::applyTemplate(fileName, (size_t)kFileNameMax, F("SHVC-%s"), code);
      }
    }

    // Read sramSizeExp
    uint8_t sramSizeExp;

    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26))
    {
      // SuperFX
      if (snesHeader[0xFFDA - headerStart] == 0x33)
      {
        sramSizeExp = snesHeader[0xFFBD - headerStart];
      }
      else
      {
        sramSizeExp = (strncmp_P(fileName, PSTR("STARFOX2"), 8) == 0) ? 6 : 5;
      }
    }
    else // No SuperFX
    {
      sramSizeExp = snesHeader[0xFFD8 - headerStart];
    }

    // Calculate sramSize
    // Fail states usually have sramSizeExp at 255 (no cart inserted, SA-1 failure, etc)
    if (sramSizeExp != 0 && sramSizeExp != 255)
    {
      sramSizeExp = sramSizeExp + 3;
      sramSize = 1;
      while (sramSizeExp--)
        sramSize *= 2;
    }
    else
    {
      sramSize = 0;
    }

    // Check Cart Country
    //uint16_t cartCountry = snesHeader[0xFFD9 - headerStart];

    // ROM Version
    romVersion = snesHeader[0xFFDB - headerStart];

    // Test if checksum is equal to reverse checksum
    if (((word(snesHeader[0xFFDC - headerStart]) + (word(snesHeader[0xFFDD - headerStart]) * 256)) + (word(snesHeader[0xFFDE - headerStart]) + (word(snesHeader[0xFFDF - headerStart]) * 256))) == 65535)
    {
      return (0x0000 != checksum);
    }
    else // Either rom checksum is wrong or no cart is inserted
    {
      return false;
    }
  }

  uint16_t calc_checksum()
  {
    uint16_t calcChecksum = 0;
    uint16_t calcChecksumChunk = 0;
    uint16_t calcFilesize = 0;
    uint16_t c = 0;
    uint32_t i = 0;
    uint32_t j = 0;

    OSCR::Storage::Shared::openRO();

    calcFilesize = OSCR::Storage::Shared::getSize() * 8 / 1024 / 1024;

    // Nintendo Power (SF Memory Cassette)
    // Read up to 0x60000 then add FFs to 0x80000
    if (isNintendoPower == true)
    {
      for (i = 0; i < (0x60000 / 512); i++)
      {
        OSCR::Storage::Shared::fill();

        for (c = 0; c < 512; c++)
        {
          calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
        }

        calcChecksum = calcChecksumChunk;
      }

      calcChecksum += 0xF47C;  // FFs from 0x60000-0x80000
    }
    else if ((calcFilesize == 10) || (calcFilesize == 12) || (calcFilesize == 20) || (calcFilesize == 24))
    {
      uint32_t calcBase = 0;
      uint32_t calcMirror = 0;
      uint8_t calcMirrorCount = 0;

      calcBase = (calcFilesize > 16) ? 2097152 : 1048576;
      calcMirror = OSCR::Storage::Shared::getSize() - calcBase;
      calcMirrorCount = calcBase / calcMirror;

      // Momotarou Dentetsu Happy Fix 3MB (24Mbit)
      if ((calcFilesize == 24) && (romChips == 245))
      {
        for (i = 0; i < (OSCR::Storage::Shared::getSize() / 512); i++)
        {
          OSCR::Storage::Shared::fill();

          for (c = 0; c < 512; c++)
          {
            calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
          }
        }

        calcChecksum = 2 * calcChecksumChunk;
      }
      else
      {
        // Base 8/16 Mbit chunk

        for (j = 0; j < (calcBase / 512); j++)
        {
          OSCR::Storage::Shared::fill();

          for (c = 0; c < 512; c++)
          {
            calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
          }
        }

        calcChecksum = calcChecksumChunk;
        calcChecksumChunk = 0;

        // Add the mirrored chunk
        for (j = 0; j < (calcMirror / 512); j++)
        {
          OSCR::Storage::Shared::fill();

          for (c = 0; c < 512; c++)
          {
            calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
          }
        }

        calcChecksum += calcMirrorCount * calcChecksumChunk;
      }
    }
    else if ((calcFilesize == 40) && (romChips == 85))
    {
      // Daikaijuu Monogatari 2 Fix 5MB (40Mbit)
      // Add the 4MB (32Mbit) start
      for (j = 0; j < (4194304 / 512); j++)
      {
        OSCR::Storage::Shared::fill();

        for (c = 0; c < 512; c++)
        {
          calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
        }

        calcChecksum = calcChecksumChunk;
      }

      calcChecksumChunk = 0;

      // Add the 1MB (8Mbit) end
      for (j = 0; j < (1048576 / 512); j++)
      {
        OSCR::Storage::Shared::fill();

        for (c = 0; c < 512; c++)
        {
          calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
        }
      }

      calcChecksum += 4 * calcChecksumChunk;
    }
    else if (calcFilesize == 48)
    {
      // Star Ocean/Tales of Phantasia Fix 6MB (48Mbit)
      // Add the 4MB (32Mbit) start
      for (j = 0; j < (4194304 / 512); j++)
      {
        OSCR::Storage::Shared::fill();

        for (c = 0; c < 512; c++)
        {
          calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
        }

        calcChecksum = calcChecksumChunk;
      }

      calcChecksumChunk = 0;

      // Add the 2MB (16Mbit) end
      for (j = 0; j < (2097152 / 512); j++)
      {
        OSCR::Storage::Shared::fill();

        for (c = 0; c < 512; c++)
        {
          calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
        }
      }

      calcChecksum += 2 * calcChecksumChunk;
    }
    else
    {
      //calcFilesize == 2 || 4 || 8 || 16 || 32 || 40 || etc
      for (i = 0; i < (OSCR::Storage::Shared::getSize() / 512); i++)
      {
        OSCR::Storage::Shared::fill();

        for (c = 0; c < 512; c++)
        {
          calcChecksumChunk += OSCR::Storage::Shared::buffer[c];
        }

        calcChecksum = calcChecksumChunk;
      }
    }

    OSCR::Storage::Shared::close();

    return calcChecksum;
  }

  bool compare_checksum()
  {
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checksum));

    uint16_t calcsum = calc_checksum();
    OSCR::UI::printHex(calcsum);

    if (calcsum == checksum)
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::Arrow));
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
      return 1;
    }
    else
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
      OSCR::UI::printHexLine(checksum);
      OSCR::UI::error(FS(OSCR::Strings::Common::Invalid));
      return 0;
    }
  }

  bool matchCRC()
  {
    return snesCRDB->matchCRC();
  }

  // Read rom to SD card
  void readROM()
  {
    cartOn();

    dataIn();
    controlIn();

    printHeader();

    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SNESD), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::SNES));

    //Dump Derby Stallion '96 (Japan) and Sound Novel Tsukuru (Japan) - Actual Size is 24Mb
    if ((romType == LO) && (numBanks == 96) && ((0xCC86 == checksum) || (0xA77B == checksum)))
    {
      // Read Banks 0x00-0x3F for the 1st/2nd MB
      for (uint16_t currBank = 0; currBank < 64; currBank++)
      {
        // Dump the bytes to SD 512B at a time
        for (uint32_t currByte = 32768; currByte < 65536; currByte += 512)
        {
          for (uint16_t c = 0; c < 512; c++)
          {
            OSCR::Storage::Shared::buffer[c] = readBank(currBank, currByte + c);
          }

          OSCR::Storage::Shared::writeBuffer();
        }
      }
      //Read Bank 0x80-9F for the 3rd MB
      for (uint16_t currBank = 128; currBank < 160; currBank++)
      {
        // Dump the bytes to SD 512B at a time
        for (uint32_t currByte = 32768; currByte < 65536; currByte += 512)
        {
          for (uint16_t c = 0; c < 512; c++)
          {
            OSCR::Storage::Shared::buffer[c] = readBank(currBank, currByte + c);
          }

          OSCR::Storage::Shared::writeBuffer();
        }
      }
    }
    else if (romType == LO) //Dump Low-type ROM
    {
      if (romChips == 243) // 0xF3
      {
        cx4Map = readBank(0, 32594); // 0x7F52
        if ((cx4Type == 2) && (cx4Map != 0)) //X2
        {
          dataOut();
          controlOut();
          writeBank(0, 32594, 0); // Set 0x7F52 to 0
          dataIn();
          controlIn();
        }
        else if ((cx4Type == 3) && (cx4Map == 0)) // X3
        {
          dataOut();
          controlOut();
          writeBank(0, 32594, 1); // Set 0x7F52 to 1
          dataIn();
          controlIn();
        }
      }

      if (romSize > 24)
      {
        // ROM > 96 banks (up to 128 banks)
        readLoRomBanks(0x80, numBanks + 0x80, &OSCR::Storage::Shared::sharedFile);
      }
      else
      {
        // Read up to 96 banks starting at bank 0×00.
        readLoRomBanks(0, numBanks, &OSCR::Storage::Shared::sharedFile);
      }

      if (romChips == 243) //0xF3
      {
        // Restore CX4 Mapping Register
        dataOut();
        controlOut();
        writeBank(0, 32594, cx4Map);  // 0x7F52
        dataIn();
        controlIn();
      }
    }
    else if ((romType == HI) && (romChips == 69 || romChips == 67)) // Dump SDD1 High-type ROM
    {
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

      controlIn();
      uint8_t initialSOMap = readBank(0, 18439);

      for (uint16_t currMemmap = 0; currMemmap < (numBanks / 16); currMemmap++)
      {

        dataOut();
        controlOut();

        writeBank(0, 18439, currMemmap);

        dataIn();
        controlIn();

        readHiRomBanks(240, 256, &OSCR::Storage::Shared::sharedFile);

        if (currMemmap == 2) printHeader();  // need more space for the progress bars
      }

      dataOut();
      controlOut();

      writeBank(0, 18439, initialSOMap);

      dataIn();
      controlIn();
    }
    else if ((romType == HI) && ((romChips == 245) || (romChips == 249))) // Dump SPC7110 High-type ROM
    {
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

      // 0xC00000-0xDFFFFF
      //OSCR::UI::printSync(F("Part 1"));

      readHiRomBanks(192, 224, &OSCR::Storage::Shared::sharedFile);

      if (numBanks > 32)
      {
        dataOut();
        controlOut();
        // Set 0x4834 to 0xFF
        writeBank(0, 0x4834, 0xFF);

        dataIn();
        controlIn();

        // 0xE00000-0xEFFFFF
        //OSCR::UI::printSync(F(" 2"));

        readHiRomBanks(224, 240, &OSCR::Storage::Shared::sharedFile);

        if (numBanks > 48)
        {
          // 0xF00000-0xFFFFFF
          //OSCR::UI::printSync(F(" 3"));

          readHiRomBanks(240, 256, &OSCR::Storage::Shared::sharedFile);

          dataOut();
          controlOut();

          // Set 0x4833 to 3
          writeBank(0, 0x4833, 3);

          dataIn();
          controlIn();

          // 0xF00000-0xFFFFFF
          //OSCR::UI::printSync(F(" 4"));

          readHiRomBanks(240, 256, &OSCR::Storage::Shared::sharedFile);
        }
        //OSCR::UI::printLine();

        printHeader();  // need more space due to the 4 progress bars

        // Return mapping registers to initial settings...
        dataOut();
        controlOut();

        writeBank(0, 0x4833, 2);
        writeBank(0, 0x4834, 0);

        dataIn();
        controlIn();
      }
    }
    else if ((romType == HI) || (romType == SA) || (romType == EX)) // Dump standard High-type ROM
    {
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

      if (romChips == 85)
      {
        // Daikaijuu Monogatari 2, keeps out S-RTC register area
        readHiRomBanks(192, 192 + 64, &OSCR::Storage::Shared::sharedFile);
        readHiRomBanks(64, numBanks, &OSCR::Storage::Shared::sharedFile);  // (64 + (numBanks - 64))
      }
      else
      {
        readHiRomBanks(192, numBanks + 192, &OSCR::Storage::Shared::sharedFile);
      }
    }

    cartOff();

    // Close the file:
    OSCR::Storage::Shared::close();
  }

  /******************************************
    SNES SRAM Functions
  *****************************************/
  // Write file to SRAM
  void writeSRAM(bool browseFile)
  {
    uint16_t sramBanks = 0;
    uint32_t lastByte = 0;

    if (browseFile) OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    cartOn();

    dataOut(); // Set pins to output
    controlOut(); // Set RST RD WR to High and CS to Low

    // LoRom
    if (romType == LO)
    {
      // Sram size
      lastByte = sramSize * 128;

      if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) // SuperFX
      {
        if (lastByte > 0x10000) // Large SuperFX SRAM (no known carts)
        {
          sramBanks = lastByte / 0x10000;

          for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
          {
            for (uint32_t currByte = 0x0000; currByte < 0x10000; currByte++)
            {
              writeBank(currBank, currByte, OSCR::Storage::Shared::sharedFile.read());
            }
          }
        }
        else // SuperFX SRAM
        {
          for (uint32_t currByte = 0; currByte < lastByte; currByte++)
          {
            writeBank(0x70, currByte, OSCR::Storage::Shared::sharedFile.read());
          }
        }
      }
      else if (lastByte > 0x8000) // Large SRAM Fix
      {
        sramBanks = lastByte / 0x8000;

        for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
        {
          for (uint32_t currByte = 0x0000; currByte < 0x8000; currByte++)
          {
            writeBank(currBank, currByte, OSCR::Storage::Shared::sharedFile.read());
          }
        }
      }
      else
      {
        for (uint32_t currByte = 0; currByte < lastByte; currByte++)
        {
          writeBank(0x70, currByte, OSCR::Storage::Shared::sharedFile.read());
        }
      }
    }
    else if (romType == HI) // HiRom
    {
      if ((romChips == 245) || (romChips == 249)) // SPC7110 SRAM
      {
        // Configure SPC7110 SRAM Register
        // Set 0x4830 to 0x80
        writeBank(0, 0x4830, 0x80);

        // Sram size
        lastByte = (long(sramSize) * 128) + 0x6000;

        // Write to sram bank
        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
        {
          writeBank(0x30, currByte, OSCR::Storage::Shared::sharedFile.read());
        }

        // Reset SPC7110 SRAM Register
        dataOut();

        // Reset 0x4830 to 0x0
        writeBank(0, 0x4830, 0);
        dataIn();
      }
      else
      {
        // Writing SRAM on HiRom needs CS(PH3) to be high
        PORTH |= (1 << 3);

        // Sram size
        lastByte = (long(sramSize) * 128);

        if (lastByte > 0x2000) // Large SRAM Fix
        {
          sramBanks = lastByte / 0x2000;

          for (uint16_t currBank = 0x30; currBank < sramBanks + 0x30; currBank++)
          {
            for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte++)
            {
              writeBank(currBank, currByte, OSCR::Storage::Shared::sharedFile.read());
            }
          }
        }
        else
        {
          lastByte += 0x6000;

          // Write to sram bank
          for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
          {
            writeBank(0x30, currByte, OSCR::Storage::Shared::sharedFile.read());
          }
        }
      }
    }
    else if (romType == EX) // ExHiRom
    {
      // Writing SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);

      // Sram size
      lastByte = sramSize * 128;

      if (lastByte > 0x2000) // Large EX SRAM Fix
      {
        sramBanks = lastByte / 0x2000;

        for (uint16_t currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++)
        {
          for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte++)
          {
            writeBank(currBank, currByte, OSCR::Storage::Shared::sharedFile.read());
          }
        }
      }
      else
      {
        lastByte += 0x6000;

        // Write to sram bank
        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
        {
          writeBank(0xB0, currByte, OSCR::Storage::Shared::sharedFile.read());
        }
      }
    }
    else if (romType == SA) // SA1
    {
      lastByte = (long(sramSize) * 128);

# if defined(ENABLE_CLOCKGEN)
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 1);
# endif /* ENABLE_CLOCKGEN */

      // Direct writes to BW-RAM (SRAM) in banks 0x40-0x43 don't work
      // Break BW-RAM (SRAM) into 0x2000 blocks
      uint8_t lastBlock = 0;
      lastBlock = lastByte / 0x2000;

      // Writing SRAM on SA1 needs CS(PH3) to be high
      // PORTH |=  (1 << 3);

      // Setup BW-RAM
      // Set 0x2224 (SNES BMAPS) to map SRAM Block 0 to 0x6000-0x7FFF
      writeBank(0, 0x2224, 0);
      // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
      writeBank(0, 0x2226, 0x80);
      // Set 0x2228 (SNES BWPA) to 0x00 BW-RAM Write-Protected Area
      writeBank(0, 0x2228, 0);
      delay(1000);

      // Use $2224 (SNES) to map BW-RAM block to 0x6000-0x7FFF
      // Use $2226 (SNES) to write enable the BW-RAM
      uint8_t firstByte = 0;

      for (uint8_t currBlock = 0; currBlock < lastBlock; currBlock++)
      {
        // Set 0x2224 (SNES BMAPS) to map SRAM Block to 0x6000-0x7FFF
        writeBank(0, 0x2224, currBlock);

        // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
        writeBank(0, 0x2226, 0x80);

        for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte += 512)
        {
          OSCR::Storage::Shared::fill();

          if ((currBlock == 0) && (currByte == 0x6000))
          {
            firstByte = OSCR::Storage::Shared::buffer[0];
          }

          for (uint16_t c = 0; c < 512; c++)
          {
            writeBank(0, currByte + c, OSCR::Storage::Shared::buffer[c]);
          }
        }
      }

      // Rewrite First Byte
      writeBank(0, 0x2224, 0);
      writeBank(0, 0x2226, 0x80);
      writeBank(0, 0x6000, firstByte);

# if defined(ENABLE_CLOCKGEN)
      // Disable CPU clock
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 0);
# endif /* ENABLE_CLOCKGEN */
    }

    cartOff();

    OSCR::Storage::Shared::close();
  }

  void readSRAM()
  {
    printHeader();

    cartOn();

    controlIn(); // set control

    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::SNESD), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::SaveRAM));

    uint16_t sramBanks = 0;
    uint32_t lastByte = 0;

    if (romType == LO)
    {
      // Sram size
      lastByte = (long(sramSize) * 128);

      if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) // SuperFX
      {
        if (lastByte > 0x10000) // Large SuperFX SRAM (no known carts)
        {
          sramBanks = lastByte / 0x10000;

          for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
          {
            for (uint32_t currByte = 0x0000; currByte < 0x10000; currByte++)
            {
              OSCR::Storage::Shared::sharedFile.write(readBank(currBank, currByte));
            }
          }
        }
        else // SuperFX SRAM
        {
          for (uint32_t currByte = 0; currByte < lastByte; currByte++)
          {
            OSCR::Storage::Shared::sharedFile.write(readBank(0x70, currByte));
          }
        }
      }
      else if (lastByte > 0x8000) // Large SRAM Fix
      {
        sramBanks = lastByte / 0x8000;

        for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
        {
          for (uint32_t currByte = 0x0000; currByte < 0x8000; currByte++)
          {
            OSCR::Storage::Shared::sharedFile.write(readBank(currBank, currByte));
          }
        }
      }
      else
      {
        for (uint32_t currByte = 0; currByte < lastByte; currByte++)
        {
          OSCR::Storage::Shared::sharedFile.write(readBank(0x70, currByte));
        }
      }
    }
    else if (romType == HI)
    {
      if ((romChips == 245) || (romChips == 249)) // SPC7110 SRAM
      {
        // Configure SPC7110 SRAM Register
        dataOut();

        // Set 0x4830 to 0x80
        writeBank(0, 0x4830, 0x80);
        dataIn();

        // Sram size
        lastByte = (sramSize * 128) + 0x6000;

        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
        {
          OSCR::Storage::Shared::sharedFile.write(readBank(0x30, currByte));
        }

        dataOut();

        // Reset 0x4830 to 0x0
        writeBank(0, 0x4830, 0);
        dataIn();
      }
      else
      {
        // Dumping SRAM on HiRom needs CS(PH3) to be high
        PORTH |= (1 << 3);

        // Sram size
        lastByte = sramSize * 128;

        if (lastByte > 0x2000) // Large SRAM Fix
        {
          sramBanks = lastByte / 0x2000;

          for (uint16_t currBank = 0x30; currBank < sramBanks + 0x30; currBank++)
          {
            for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte++)
            {
              OSCR::Storage::Shared::sharedFile.write(readBank(currBank, currByte));
            }
          }
        }
        else
        {
          lastByte += 0x6000;

          for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
          {
            OSCR::Storage::Shared::sharedFile.write(readBank(0x30, currByte));
          }
        }
      }
    }
    else if (romType == EX)
    {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);

      // Sram size
      lastByte = sramSize * 128;

      if (lastByte > 0x2000) // Large EX SRAM Fix
      {
        sramBanks = lastByte / 0x2000;

        for (uint16_t currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++)
        {
          for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte++)
          {
            OSCR::Storage::Shared::sharedFile.write(readBank(currBank, currByte));
          }
        }
      }
      else
      {
        lastByte += 0x6000;

        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
        {
          OSCR::Storage::Shared::sharedFile.write(readBank(0xB0, currByte));
        }
      }
    }
    else if (romType == SA)
    {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);

      // Sram size
      uint32_t lastByte = sramSize * 128;

      if (lastByte > 0x10000)
      {
        sramBanks = lastByte / 0x10000;

        for (uint16_t currBank = 0x40; currBank < sramBanks + 0x40; currBank++)
        {
          for (uint32_t currByte = 0; currByte < 0x10000; currByte++)
          {
            OSCR::Storage::Shared::sharedFile.write(readBank(currBank, currByte));
          }
        }
      }
      else
      {
        for (uint32_t currByte = 0x0; currByte < lastByte; currByte++)
        {
          OSCR::Storage::Shared::sharedFile.write(readBank(0x40, currByte));
        }
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();
  }

  // Check if the SRAM was written without any error
  uint32_t verifySRAM()
  {
    uint32_t errors = 0;
    uint32_t sramBanks = 0;
    uint32_t lastByte = 0;

    OSCR::Storage::Shared::openRO();

    cartOn();

    // Set control
    controlIn();

    switch (romType)
    {
    case LO:
      // Sram size
      lastByte = sramSize * 128;

      if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) // SuperFX
      {
        if (lastByte > 0x10000) // Large SuperFX SRAM (no known carts)
        {
          sramBanks = lastByte / 0x10000;

          for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
          {
            for (uint32_t currByte = 0; currByte < 0x10000; currByte += 512)
            {
              OSCR::Storage::Shared::fill();

              for (uint16_t c = 0; c < 512; c++)
              {
                if ((readBank(currBank, currByte + c)) != OSCR::Storage::Shared::buffer[c])
                {
                  errors++;
                }
              }
            }
          }
        }
        else // SuperFX SRAM
        {
          for (uint32_t currByte = 0; currByte < lastByte; currByte += 512)
          {
            OSCR::Storage::Shared::fill();

            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(0x70, currByte + c)) != OSCR::Storage::Shared::buffer[c])
              {
                errors++;
              }
            }
          }
        }
      }
      else if (lastByte > 0x8000) // Large SRAM Fix
      {
        sramBanks = lastByte / 0x8000;

        for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
        {
          for (uint32_t currByte = 0; currByte < 0x8000; currByte += 512)
          {
            OSCR::Storage::Shared::fill();

            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(currBank, currByte + c)) != OSCR::Storage::Shared::buffer[c])
              {
                errors++;
              }
            }
          }
        }
      }
      else
      {
        for (uint32_t currByte = 0; currByte < lastByte; currByte += 512)
        {
          OSCR::Storage::Shared::fill();

          for (uint16_t c = 0; c < 512; c++)
          {
            if ((readBank(0x70, currByte + c)) != OSCR::Storage::Shared::buffer[c])
            {
              errors++;
            }
          }
        }
      }
      break;

    case HI:
      if ((romChips == 245) || (romChips == 249)) // SPC7110 SRAM
      {
        // Configure SPC7110 SRAM Register
        dataOut();

        // Set 0x4830 to 0x80
        writeBank(0, 0x4830, 0x80);

        dataIn();

        // Sram size
        lastByte = (sramSize * 128) + 0x6000;

        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte += 512)
        {
          OSCR::Storage::Shared::fill();

          for (uint16_t c = 0; c < 512; c++)
          {
            if ((readBank(0x30, currByte + c)) != OSCR::Storage::Shared::buffer[c])
            {
              errors++;
            }
          }
        }
        dataOut();

        // Reset 0x4830 to 0x0
        writeBank(0, 0x4830, 0);

        dataIn();
      }
      else
      {
        // Dumping SRAM on HiRom needs CS(PH3) to be high
        PORTH |= (1 << 3);

        // Sram size
        lastByte = sramSize * 128;

        if (lastByte > 0x2000) // Large SRAM Fix
        {
          sramBanks = lastByte / 0x2000;

          for (uint16_t currBank = 0x30; currBank < sramBanks + 0x30; currBank++)
          {
            for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte += 512)
            {
              OSCR::Storage::Shared::fill();

              for (uint16_t c = 0; c < 512; c++)
              {
                if ((readBank(currBank, currByte + c)) != OSCR::Storage::Shared::buffer[c])
                {
                  errors++;
                }
              }
            }
          }
        }
        else
        {
          lastByte += 0x6000;

          for (uint32_t currByte = 0x6000; currByte < lastByte; currByte += 512)
          {
            OSCR::Storage::Shared::fill();

            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(0x30, currByte + c)) != OSCR::Storage::Shared::buffer[c])
              {
                errors++;
              }
            }
          }
        }
      }
      break;

    case EX:
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);

      // Sram size
      lastByte = sramSize * 128;

      if (lastByte > 0x2000) // Large EX SRAM Fix
      {
        sramBanks = lastByte / 0x2000;

        for (uint16_t currBank = 0xB0; currBank < (sramBanks + 0xB0); currBank++)
        {
          for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte += 512)
          {
            OSCR::Storage::Shared::fill();

            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(currBank, currByte + c)) != OSCR::Storage::Shared::buffer[c])
              {
                errors++;
              }
            }
          }
        }
      }
      else
      {
        lastByte += 0x6000;

        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte += 512)
        {
          OSCR::Storage::Shared::fill();

          for (uint16_t c = 0; c < 512; c++)
          {
            if ((readBank(0xB0, currByte + c)) != OSCR::Storage::Shared::buffer[c])
            {
              errors++;
            }
          }
        }
      }
      break;

    case SA:
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);

      // Sram size
      lastByte = sramSize * 128;

      if (lastByte > 0x10000)
      {
        sramBanks = lastByte / 0x10000;

        for (uint16_t currBank = 0x40; currBank < sramBanks + 0x40; currBank++)
        {
          for (uint32_t currByte = 0x0; currByte < 0x10000; currByte += 512)
          {
            OSCR::Storage::Shared::fill();

            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(currBank, currByte + c)) != OSCR::Storage::Shared::buffer[c])
              {
                errors++;
              }
            }
          }
        }
      }
      else
      {
        for (uint32_t currByte = 0x0; currByte < lastByte; currByte += 512)
        {
          OSCR::Storage::Shared::fill();

          for (uint16_t c = 0; c < 512; c++)
          {
            if ((readBank(0x40, currByte + c)) != OSCR::Storage::Shared::buffer[c])
            {
              errors++;
            }
          }
        }
      }
      break;
    }

    // Close the file:
    OSCR::Storage::Shared::close();

    cartOff();

    return errors;
  }

  // Overwrite the entire SRAM
  bool eraseSRAM(uint8_t b)
  {
    OSCR::UI::printHex(b);
    OSCR::UI::printSync(FS(OSCR::Strings::Symbol::LabelEnd));

    cartOn();

    dataOut(); // Set pins to output
    controlOut(); // Set control pins

    uint16_t sramBanks = 0;
    uint32_t lastByte = 0;
    uint8_t lastBlock = 0;

    switch (romType)
    {
    case LO:
      // Sram size
      lastByte = sramSize * 128;

      // SuperFX
      if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26))
      {
        if (lastByte > 0x10000) // Large SuperFX SRAM (no known carts)
        {
          sramBanks = lastByte / 0x10000;

          for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
          {
            for (uint32_t currByte = 0x0000; currByte < 0x10000; currByte++)
            {
              writeBank(currBank, currByte, b);
            }
          }
        }
        else // SuperFX SRAM
        {
          for (uint32_t currByte = 0; currByte < lastByte; currByte++)
          {
            writeBank(0x70, currByte, b);
          }
        }
      }
      else if (lastByte > 0x8000) // Large SRAM Fix
      {
        sramBanks = lastByte / 0x8000;

        for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
        {
          for (uint32_t currByte = 0x0000; currByte < 0x8000; currByte++)
          {
            writeBank(currBank, currByte, b);
          }
        }
      }
      else
      {
        for (uint32_t currByte = 0; currByte < lastByte; currByte++)
        {
          writeBank(0x70, currByte, b);
        }
      }
      break;

    case HI:
      if ((romChips == 245) || (romChips == 249)) // SPC7110 SRAM
      {
        // Configure SPC7110 SRAM Register
        // Set 0x4830 to 0x80
        writeBank(0, 0x4830, 0x80);

        // Sram size
        lastByte = (sramSize * 128) + 0x6000;

        // Write to sram bank
        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
        {
          writeBank(0x30, currByte, b);
        }

        // Reset SPC7110 SRAM Register
        dataOut();

        // Reset 0x4830 to 0x0
        writeBank(0, 0x4830, 0);

        dataIn();
      }
      else
      {
        // Writing SRAM on HiRom needs CS(PH3) to be high
        PORTH |= (1 << 3);

        // Sram size
        lastByte = (sramSize * 128);

        if (lastByte > 0x2000) // Large SRAM Fix
        {
          sramBanks = lastByte / 0x2000;

          for (uint16_t currBank = 0x30; currBank < sramBanks + 0x30; currBank++)
          {
            for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte++)
            {
              writeBank(currBank, currByte, b);
            }
          }
        }
        else
        {
          lastByte += 0x6000;

          // Write to sram bank
          for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
          {
            writeBank(0x30, currByte, b);
          }
        }
      }
      break;

    case EX: // ExHiRom
      // Writing SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);

      /// Sram size
      lastByte = sramSize * 128;

      if (lastByte > 0x2000) // Large EX SRAM Fix
      {
        sramBanks = lastByte / 0x2000;

        for (uint16_t currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++)
        {
          for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte++)
          {
            writeBank(currBank, currByte, b);
          }
        }
      }
      else
      {
        lastByte += 0x6000;

        // Write to sram bank
        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte++)
        {
          writeBank(0xB0, currByte, b);
        }
      }

      break;

    case SA: // SA1
      lastByte = sramSize * 128;

# if defined(ENABLE_CLOCKGEN)
      // Enable CPU Clock
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 1);
# endif /* ENABLE_CLOCKGEN */

      // Direct writes to BW-RAM (SRAM) in banks 0x40-0x43 don't work
      // Break BW-RAM (SRAM) into 0x2000 blocks
      // Use $2224 to map BW-RAM block to 0x6000-0x7FFF
      lastBlock = lastByte / 0x2000;

      // Writing SRAM on SA1 needs CS(PH3) to be high
      // PORTH |=  (1 << 3);

      // Setup BW-RAM
      // Set 0x2224 (SNES BMAPS) to map SRAM Block 0 to 0x6000-0x7FFF
      writeBank(0, 0x2224, 0);
      // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
      writeBank(0, 0x2226, 0x80);
      // Set 0x2228 (SNES BWPA) to 0x00 BW-RAM Write-Protected Area
      writeBank(0, 0x2228, 0);
      delay(1000);

      // Use $2224 (SNES) to map BW-RAM block to 0x6000-0x7FFF
      // Use $2226 (SNES) to write enable the BW-RAM
      for (uint8_t currBlock = 0; currBlock < lastBlock; currBlock++)
      {
        // Set 0x2224 (SNES BMAPS) to map SRAM Block to 0x6000-0x7FFF
        writeBank(0, 0x2224, currBlock);

        // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
        writeBank(0, 0x2226, 0x80);

        for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte += 512)
        {
          for (uint16_t c = 0; c < 512; c++)
          {
            writeBank(0, currByte + c, b);
          }
        }
      }

      // Rewrite First Byte
      writeBank(0, 0x2224, 0);
      writeBank(0, 0x2226, 0x80);
      writeBank(0, 0x6000, b);

# if defined(ENABLE_CLOCKGEN)
      // Disable CPU clock
      OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 0);
# endif /* ENABLE_CLOCKGEN */

      break;
    }

    dataIn();

    // Variable for errors
    writeErrors = 0;

    // Set control
    controlIn();

    sramBanks = 0;

    switch (romType)
    {
    case LO: // Sram size
      lastByte = sramSize * 128;

      if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) // SuperFX
      {
        if (lastByte > 0x10000) // Large SuperFX SRAM (no known carts)
        {
          sramBanks = lastByte / 0x10000;

          for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
          {
            for (uint32_t currByte = 0; currByte < 0x10000; currByte += 512)
            {
              for (uint16_t c = 0; c < 512; c++)
              {
                if ((readBank(currBank, currByte + c)) != b)
                {
                  writeErrors++;
                }
              }
            }
          }
        }
        else // SuperFX SRAM
        {
          for (uint32_t currByte = 0; currByte < lastByte; currByte += 512)
          {
            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(0x70, currByte + c)) != b)
              {
                writeErrors++;
              }
            }
          }
        }
      }
      else if (lastByte > 0x8000) // Large SRAM Fix
      {
        sramBanks = lastByte / 0x8000;

        for (uint16_t currBank = 0x70; currBank < sramBanks + 0x70; currBank++)
        {
          for (uint32_t currByte = 0; currByte < 0x8000; currByte += 512)
          {
            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(currBank, currByte + c)) != b)
              {
                writeErrors++;
              }
            }
          }
        }
      }
      else
      {
        for (uint32_t currByte = 0; currByte < lastByte; currByte += 512)
        {
          for (uint16_t c = 0; c < 512; c++)
          {
            if ((readBank(0x70, currByte + c)) != b)
            {
              writeErrors++;
            }
          }
        }
      }
      break;

    case HI:
      if ((romChips == 245) || (romChips == 249)) // SPC7110 SRAM
      {
        // Configure SPC7110 SRAM Register
        dataOut();

        // Set 0x4830 to 0x80
        writeBank(0, 0x4830, 0x80);

        dataIn();

        // Sram size
        lastByte = (sramSize * 128) + 0x6000;

        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte += 512)
        {
          for (uint16_t c = 0; c < 512; c++)
          {
            if ((readBank(0x30, currByte + c)) != b)
            {
              writeErrors++;
            }
          }
        }

        dataOut();

        // Reset 0x4830 to 0x0
        writeBank(0, 0x4830, 0);

        dataIn();
      }
      else
      {
        // Dumping SRAM on HiRom needs CS(PH3) to be high
        PORTH |= (1 << 3);

        // Sram size
        lastByte = (sramSize * 128);

        if (lastByte > 0x2000) // Large SRAM Fix
        {
          sramBanks = lastByte / 0x2000;

          for (uint16_t currBank = 0x30; currBank < sramBanks + 0x30; currBank++)
          {
            for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte += 512)
            {
              for (uint16_t c = 0; c < 512; c++)
              {
                if ((readBank(currBank, currByte + c)) != b)
                {
                  writeErrors++;
                }
              }
            }
          }
        }
        else
        {
          lastByte += 0x6000;

          for (uint32_t currByte = 0x6000; currByte < lastByte; currByte += 512)
          {
            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(0x30, currByte + c)) != b)
              {
                writeErrors++;
              }
            }
          }
        }
      }
      break;

    case EX:
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);

      // Sram size
      lastByte = (sramSize * 128);

      if (lastByte > 0x2000) // Large EX SRAM Fix
      {
        sramBanks = lastByte / 0x2000;

        for (uint16_t currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++)
        {
          for (uint32_t currByte = 0x6000; currByte < 0x8000; currByte += 512)
          {
            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(currBank, currByte + c)) != b)
              {
                writeErrors++;
              }
            }
          }
        }
      }
      else
      {
        lastByte += 0x6000;

        for (uint32_t currByte = 0x6000; currByte < lastByte; currByte += 512)
        {
          for (uint16_t c = 0; c < 512; c++)
          {
            if ((readBank(0xB0, currByte + c)) != b)
            {
              writeErrors++;
            }
          }
        }
      }

      break;

    case SA:
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);

      // Sram size
      lastByte = (sramSize * 128);

      if (lastByte > 0x10000)
      {
        sramBanks = lastByte / 0x10000;
        for (uint16_t currBank = 0x40; currBank < sramBanks + 0x40; currBank++)
        {
          for (uint32_t currByte = 0x0; currByte < 0x10000; currByte += 512)
          {
            for (uint16_t c = 0; c < 512; c++)
            {
              if ((readBank(currBank, currByte + c)) != b)
              {
                writeErrors++;
              }
            }
          }
        }
      }
      else
      {
        for (uint32_t currByte = 0x0; currByte < lastByte; currByte += 512)
        {
          for (uint16_t c = 0; c < 512; c++)
          {
            if ((readBank(0x40, currByte + c)) != b)
            {
              writeErrors++;
            }
          }
        }
      }

      break;
    }

    cartOff();

    if (writeErrors == 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
      return true;
    }

    OSCR::Lang::printErrorVerifyBytes(writeErrors);
    return false;
  }
}

#endif /* HAS_SNES */
