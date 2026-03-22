//******************************************
// NES MODULE
//******************************************
// mostly copy&pasted from "Famicom Dumper" 2019-08-31 by skaman
// also based on "CoolArduino" by HardWareMan
// Pinout changes: LED and CIRAM_A10
#include "config.h"

#if HAS_NES
# include "cores/include.h"
# include "cores/NES.h"
# include "common/crdb/nes.h"
# include "cores/Flash.h"

namespace OSCR::Cores::NES
{
# if HAS_FLASH
  using OSCR::Cores::Flash::flashid;
# endif /* HAS_FLASH */

  void printNESSettings(void)
  {
    printHeader();

    OSCR::UI::printValue(OSCR::Strings::Common::Mapper, NES_MAPPER);
    OSCR::UI::printSize(OSCR::Strings::Common::PRG, ((uint32_t)NES_PRGSIZE) * 1024);
    OSCR::UI::printSize(OSCR::Strings::Common::CHR, ((uint32_t)NES_CHRSIZE) * 1024);

    uint32_t _ramSize = 0;

    if (NES_MAPPER == 0)
    {
      _ramSize = NES_RAMSIZE * 256;
    }
    else if ((NES_MAPPER == 16) || (NES_MAPPER == 80) || (NES_MAPPER == 159))
    {
      if (NES_MAPPER == 16)
        _ramSize = NES_RAMSIZE * 32;
      else
        _ramSize = NES_RAMSIZE * 16;
    }
    else if ((NES_MAPPER == 19) && (NES_RAM == 2))
    {
      _ramSize = 128;
    }
    else
    {
      _ramSize = NES_RAMSIZE * 1024;
    }

    OSCR::UI::printSize(OSCR::Strings::Common::RAM, _ramSize);
  }

  /******************************************
     Low Level Functions
  *****************************************/
  static void set_address(uint16_t address)
  {
    uint8_t l = address & 0xFF;
    uint8_t h = address >> 8;

    PORTL = l;
    PORTA = h;

    // PPU /A13
    if ((address >> 13) & 1)
      PORTF &= ~(1 << 4);
    else
      PORTF |= 1 << 4;
  }

  static void set_romsel(uint16_t address)
  {
    if (address & 0x8000) {
      NES_ROMSEL_LOW;
    } else {
      NES_ROMSEL_HI;
    }
  }

  char const _file_name_no_number_fmt[] PROGMEM = "%s.%s";
  char const _file_name_with_number_fmt[] PROGMEM = "%s.%02d.%s";

  /******************************************
    Variables
  *****************************************/

  constexpr uint16_t const prgSizes[] = {
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    2048,
    4096,
    8192,
    16384,
    32768,
  };
  __constinit uint8_t prglo = 0;   // Lowest Entry
  __constinit uint8_t prghi = sizeofarray(prgSizes) - 1;  // Highest Entry

  constexpr uint16_t const chrSizes[] = {
    0,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    2048,
    4096,
  };
  __constinit uint8_t chrlo = 0;   // Lowest Entry
  __constinit uint8_t chrhi = sizeofarray(chrSizes) - 1;  // Highest Entry

  constexpr uint8_t const ramSizes[] = {
    0,
    8,
    16,
    32,
  };
  __constinit uint8_t ramlo = 0;  // Lowest Entry
  __constinit uint8_t ramhi = sizeofarray(ramSizes) - 1;  // Highest Entry

  bool flashfound = false;  // NESmaker 39SF040 Flash Cart
  bool mmc6 = false;

  /******************************************
    Menus
  *****************************************/
  // NES start menu
  constexpr char const PROGMEM menuOption01[] = "Read iNES ROM";
  constexpr char const PROGMEM menuOption02[] = "Read PRG/CHR";
  constexpr char const PROGMEM menuOption05[] = "Change Mapper";
  constexpr char const PROGMEM menuOption06[] = "Flash Repro";

  enum class MenuOption : uint8_t
  {
    ReadROM,
    ReadPRGCHR,
    ReadSave,
    WriteSave,
    SetMapper,
# if HAS_FLASH
    WriteFlash,
# endif
    BrowseDatabase,
    RefreshCart,
    Back,
  };

  constexpr char const * const PROGMEM menuOptions[] = {
    menuOption01,
    menuOption02,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    menuOption05,
# if HAS_FLASH
    menuOption06,
# endif
    OSCR::Strings::MenuOptions::SelectCart,
    OSCR::Strings::MenuOptions::RefreshCart,
    OSCR::Strings::MenuOptions::Back,
  };

  // NES chips menu
  constexpr char const PROGMEM nesChipsMenuItem1[]  = "Combined PRG+CHR";
  constexpr char const PROGMEM nesChipsMenuItem2[]  = "Read PRG";
  constexpr char const PROGMEM nesChipsMenuItem3[]  = "Read CHR";

  constexpr char const * const PROGMEM menuOptionsNESChips[] = {
    nesChipsMenuItem1,
    nesChipsMenuItem2,
    nesChipsMenuItem3,
    OSCR::Strings::MenuOptions::Back,
  };

# if HAS_FLASH
  // Repro Writer Menu
  constexpr char const PROGMEM nesFlashMenuItem1[]  = "Flash NesMaker";
  constexpr char const PROGMEM nesFlashMenuItem2[]  = "Flash A29040B-MAPPER0";

  constexpr char const * const PROGMEM menuOptionsNESFlash[] = {
    nesFlashMenuItem1,
    nesFlashMenuItem2,
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr char const PROGMEM headerA29040B[]      = "FLASH A29040B MAPPER 0";
# endif

  // NES start menu
  void menu()
  {
    MenuOption selection;

    openCRDB();

    getMapping();
    checkStatus();

    do
    {
      selection = static_cast<MenuOption>(OSCR::UI::menu(FS(OSCR::Strings::Cores::NES), menuOptions, sizeofarray(menuOptions)));

      switch (selection)
      {
      case MenuOption::ReadROM: // Read Rom
        readRom();
        break;

      case MenuOption::ReadPRGCHR: // Read single chip
        chipMenu();
        break;

      case MenuOption::ReadSave: // Read RAM
        readRAM();
        break;

      case MenuOption::WriteSave: // Write RAM
        writeRAM();
        break;

      case MenuOption::SetMapper: // Change Mapper
        fromCRDB = false;
        useDefaultName();
        while (!setMapper());
        checkMapperSize();
        setPRGSize();
        setCHRSize();
        setRAMSize();
        checkStatus();
        break;

# if HAS_FLASH
      case MenuOption::WriteFlash: // Write FLASH
        flashMenu();
        continue;
# endif

      case MenuOption::BrowseDatabase: // Browse Database
        dbBrowseMenu();
        continue;

      case MenuOption::RefreshCart: // RefreshCart cart
        getMapping();
        checkStatus();
        continue;

      case MenuOption::Back: // Back to main menu
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void chipMenu()
  {
    uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectType), menuOptionsNESChips, sizeofarray(menuOptionsNESChips));
    switch (menuSelection)
    {
      case 0: // Read combined PRG/CHR
        readRaw();
        break;

      case 1: // Read PRG
        readPRG(false);
        break;

      // Read CHR
      case 2:
        readCHR(false);
        break;

      // Return to Main Menu
      case 3:
        return;
    }

    OSCR::UI::waitButton();
  }

# if defined(ENABLE_PINCONTROL)
  OSCR::Hardware::PinControl pinControl = OSCR::Hardware::PinControl();
  using OSCR::Hardware::PinBank;
  using OSCR::Hardware::DataMode;
# endif /* ENABLE_PINCONTROL */

  crdbNESRecord * romDetail;
  crdbNESMapperRecord * mapperDetail;

  OSCR::Databases::NESRecord * romRecord = nullptr;
  OSCR::Databases::NESMapper * nesMapperCRDB = nullptr;
  //OSCR::Databases::NES * nesCRDB = nullptr;

  void dbBrowseMenu()
  {
    OSCR::crdbBrowser(FS(OSCR::Strings::Headings::SelectCRDBEntry), nesCRDB);
    romRecord = nesCRDB->record();
    romDetail = romRecord->data();

    setOutName(BUFFN(romDetail->name));

#   if CRDB_DEBUGGING
    OSCR::Serial::printValue(OSCR::Strings::Common::Selected, romDetail->name);

    romRecord->debug();
#   endif /* CRDB_DEBUGGING */
  }

# if HAS_FLASH
  void flashMenu()
  {
    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::NES), menuOptionsNESFlash, sizeofarray(menuOptionsNESFlash)))
      {
      case 0:
        if (NES_MAPPER == 30)
        {
          writeFlash();
        }
        else
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }
        break;

      case 1:
        if (NES_MAPPER == 0)
        {
          A29040B_writeFlash();
        }
        else
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
          OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
          continue;
        }
        break;

      case 2: // Return to Main Menu
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }
# endif

  /******************************************
     Setup
  *****************************************/

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::NES));
  }

  void openCRDB()
  {
    if (nesMapperCRDB == nullptr)
    {
      nesMapperCRDB = new OSCR::Databases::NESMapper(F("nes-mappers"));
    }

    if (nesCRDB == nullptr)
    {
      cartCRDB = new OSCR::Databases::NES(FS(OSCR::Strings::FileType::NES));
    }
  }

  void closeCRDB()
  {
    delete nesMapperCRDB;
    delete nesCRDB;

    nesMapperCRDB = nullptr;

    resetGlobals();
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    delay(100);
    OSCR::Power::enableCartridge();
    delay(400);

    // CPU R/W, IRQ, PPU /RD, PPU /A13, CIRAM /CE, PPU /WR, /ROMSEL, PHI2
    DDRF = 0b10110111;
    // CPU R/W, IRQ, PPU /RD, PPU /A13, CIRAM /CE, PPU /WR, /ROMSEL, PHI2
    PORTF = 0b11111111;

    // Set CIRAM A10 to Input
    DDRC &= ~(1 << 2);
    // Activate Internal Pullup Resistors
    PORTC |= (1 << 2);

# if defined(ENABLE_PINCONTROL)
    pinControl.usePullup(true);
    pinControl.addAddressBank(PinBank::kPinBankL, PinBank::kPinBankA);
    pinControl.addDataBank(PinBank::kPinBankK);
    pinControl.setAddress(0);
# else /* !ENABLE_PINCONTROL */
    // A0-A7 to Output
    DDRL = 0xFF;
    // A8-A14 to Output
    DDRA = 0xFF;

    // Set D0-D7 to Input
    PORTK = 0xFF;
    DDRK = 0;

    set_address(0);
# endif /* ENABLE_PINCONTROL */

    delay(100);
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  crc32_t oldcrc32, oldcrc32MMC3;

  void getMapping()
  {
    OSCR::Databases::NESRecord * record;

    printHeader();

    fromCRDB = false;
    oldcrc32.reset();
    oldcrc32MMC3.reset();

    cartOn();

    // Read first 512 bytes of first and last block of PRG ROM and compute CRC32
    // MMC3 maps the last 8KB block of PRG ROM to 0xE000 while 0x8000 can contain random data after bootup
    for (size_t c = 0; c < 512; c++)
    {
      oldcrc32 += read_prg_byte(0x8000 + c);
      oldcrc32MMC3 += read_prg_byte(0xE000 + c);
    }

    cartOff();

    oldcrc32.done();
    oldcrc32MMC3.done();

    // Filter out all 0xFF checksums at 0x8000 and 0xE000
    if (oldcrc32 == 0xBD7BC39F && oldcrc32MMC3 == 0xBD7BC39F)
    {
      useDefaultName();
      OSCR::UI::printHeader(FS(OSCR::Strings::Headings::CRDB));
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotFoundDB));
    }
    else
    {
      OSCR::UI::printLabel(OSCR::Strings::Common::CRCSum);
      OSCR::UI::print(oldcrc32);

      if (oldcrc32 != oldcrc32MMC3)
      {
        OSCR::UI::print(FS(OSCR::Strings::Symbol::Slash));
        OSCR::UI::print(oldcrc32MMC3);
      }

      OSCR::UI::printLine();
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::SearchingDatabase));

#if OPTION_CRDB_STRICT_MATCHING
      if (nesCRDB->findEitherRecord(oldcrc32, oldcrc32MMC3, 4))
#else
      if (nesCRDB->findEitherRecord(oldcrc32, oldcrc32MMC3, 8))
#endif /* OPTION_CRDB_STRICT_MATCHING */
      {
        record = nesCRDB->record();

        if (!nesCRDB->hasError())
        {
          romRecord = record;
          romDetail = record->data();
          fromCRDB = true;

          setOutName(BUFFN(romDetail->name));

          OSCR::UI::printLine(romDetail->name);

#   if CRDB_DEBUGGING
          OSCR::Serial::print(F("Found ROM: "));
          OSCR::Serial::printLine(romDetail->name);

          romRecord->debug();
#   endif /* CRDB_DEBUGGING */
        }
        else
        {
          // File searched until end but nothing found
#   if CRDB_DEBUGGING
          OSCR::Serial::printLine(FS(OSCR::Strings::Errors::NotFoundDB));
#   endif /* CRDB_DEBUGGING */
          OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotFoundDB));

          useDefaultName();
        }
      }
      else
      {
#   if CRDB_DEBUGGING
        OSCR::Serial::printLine(FS(OSCR::Strings::Errors::NotFoundDB));
#   endif /* CRDB_DEBUGGING */
      }
    }

    OSCR::UI::update();

    delay(1000);
  }

  // void read(char const * fileSuffix, uint8_t const * header, uint8_t const headersize, bool const renamerom)
  void read(bool const headered)
  {
    uint8_t const headerSize = headered ? 16 : 0;

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::NES), FS(OSCR::Strings::Directory::ROM), fileName, FS(headered ? OSCR::Strings::FileType::NES : OSCR::Strings::FileType::Raw));

    if (fromCRDB)
    {
      OSCR::UI::printValue(OSCR::Strings::Common::Name, romDetail->name);
    }
    else
    {
      OSCR::UI::printValue_P(OSCR::Strings::Common::Name, OSCR::Strings::Common::Unknown);
    }

    OSCR::UI::ProgressBar::init(headerSize + (((uint32_t)NES_PRGSIZE) * 1024) + (((uint32_t)NES_CHRSIZE) * 1024), 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    //Write header
    if (headered)
    {
      OSCR::Storage::Shared::sharedFile.write(NES_INES, headerSize);
      OSCR::Storage::Shared::sharedFile.crcReset(); // Don't include header in CRC32

      OSCR::UI::ProgressBar::advance(headerSize);
    }

    cartOn();

    readPRG(true);
    readCHR(true);

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::ProgressBar::finish();

    // Compare CRC32 with database
    if (nesCRDB->matchCRC(OSCR::CRC32::current))
    {
      if (headered)
      {
        OSCR::Storage::Shared::rename_P(nesCRDB->record()->data()->name, FS(OSCR::Strings::FileType::NES));
      }
    }
  }

  void readRom()
  {
    //read("nes", NES_INES, 16, true);
    read(true);
  }

  void readRaw()
  {
    //read("bin", NULL, 0, false);
    read(false);
  }

# if defined(ENABLE_PINCONTROL)
  uint8_t read_prg_byte(uint16_t address)
  {
    pinControl.setMode(DataMode::kRead);
    NES_PRG_READ;
    pinControl.setAddress(address);
    NES_PHI2_HI;
    set_romsel(address);
    _delay_us(1);
    return pinControl.readData<uint8_t>();
  }

  uint8_t read_chr_byte(uint16_t address)
  {
    pinControl.setMode(DataMode::kRead);
    NES_PHI2_HI;
    NES_ROMSEL_HI;
    pinControl.setAddress(address);
    NES_CHR_READ_LOW;
    _delay_us(1);
    uint8_t result = pinControl.readData<uint8_t>();
    NES_CHR_READ_HI;
    return result;
  }

  void write_prg_byte(uint16_t address, uint8_t data)
  {
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    pinControl.setMode(DataMode::kWrite);
    NES_PRG_WRITE;

    pinControl.writeData(data);
    pinControl.setAddress(address);  // PHI2 low, ROMSEL always HIGH
    NES_PHI2_HI;
    set_romsel(address);  // ROMSEL is low if need, PHI2 high
    _delay_us(1);         // WRITING
    // PHI2 low, ROMSEL high
    NES_PHI2_LOW;
    _delay_us(1);
    NES_ROMSEL_HI;
    // Back to read mode
    NES_PRG_READ;
    pinControl.setMode(DataMode::kRead);
    pinControl.setAddress(0);
    // Set phi2 to high state to keep cartridge unreseted
    NES_PHI2_HI;
  }
# else /* !ENABLE_PINCONTROL */
  uint8_t read_prg_byte(uint16_t address)
  {
    NES_MODE_READ;
    NES_PRG_READ;
    NES_ROMSEL_HI;
    set_address(address);
    NES_PHI2_HI;
    set_romsel(address);
    _delay_us(1);
    return PINK;
  }

  uint8_t read_chr_byte(uint16_t address)
  {
    NES_MODE_READ;
    NES_PHI2_HI;
    NES_ROMSEL_HI;
    set_address(address);
    NES_CHR_READ_LOW;
    _delay_us(1);
    uint8_t result = PINK;
    NES_CHR_READ_HI;
    return result;
  }

  void write_prg_byte(uint16_t address, uint8_t data)
  {
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    NES_MODE_WRITE;
    NES_PRG_WRITE;
    PORTK = data;

    set_address(address);  // PHI2 low, ROMSEL always HIGH
    //  _delay_us(1);
    NES_PHI2_HI;
    //_delay_us(10);
    set_romsel(address);  // ROMSEL is low if need, PHI2 high
    _delay_us(1);         // WRITING
    //_delay_ms(1); // WRITING
    // PHI2 low, ROMSEL high
    NES_PHI2_LOW;
    _delay_us(1);
    NES_ROMSEL_HI;
    // Back to read mode
    //  _delay_us(1);
    NES_PRG_READ;
    NES_MODE_READ;
    set_address(0);
    // Set phi2 to high state to keep cartridge unreseted
    //  _delay_us(1);
    NES_PHI2_HI;
    //  _delay_us(1);
  }
# endif /* ENABLE_PINCONTROL */

# if HAS_FLASH
  static void write_chr_byte(uint16_t address, uint8_t data)
  {
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    NES_MODE_WRITE;
    PORTK = data;
    set_address(address);  // PHI2 low, ROMSEL always HIGH
    _delay_us(1);

    NES_CHR_WRITE_LOW;

    _delay_us(1);  // WRITING

    NES_CHR_WRITE_HI;

    _delay_us(1);

    NES_MODE_READ;
    set_address(0);
    NES_PHI2_HI;

    //_delay_us(1);
  }
# endif

  void write_mmc1_byte(uint16_t address, uint8_t data)
  {  // write loop for 5 bit register
    if (address >= 0xE000)
    {
      for (uint8_t i = 0; i < 5; i++)
      {
        write_reg_byte(address, data >> i);  // shift 1 bit into temp register [WRITE RAM SAFE]
      }
    }
    else
    {
      for (uint8_t j = 0; j < 5; j++)
      {
        write_prg_byte(address, data >> j);  // shift 1 bit into temp register
      }
    }
  }

  // REFERENCE FOR REGISTER WRITE TO 0xE000/0xF000
  // PORTF 7 = CPU R/W = 0
  // PORTF 6 = /IRQ = 1
  // PORTF 5 = PPU /RD = 1
  // PORTF 4 = PPU /A13 = 1
  // PORTF 3 = CIRAM /CE = 1
  // PORTF 2 = PPU /WR = 1
  // PORTF 1 = /ROMSEL
  // PORTF 0 = PHI2 (M2)

# if defined(ENABLE_PINCONTROL)
  // WRITE RAM SAFE TO REGISTERS 0xE000/0xF000
  static void write_reg_byte(uint16_t address, uint8_t data)
  {  // FIX FOR MMC1 RAM CORRUPTION
    NES_PHI2_LOW;
    NES_ROMSEL_HI;  // A15 HI = E000
    pinControl.setMode(DataMode::kWrite);
    NES_PRG_WRITE;  // CPU R/W LO
    pinControl.writeData(data);

    pinControl.setAddress(address);  // PHI2 low, ROMSEL always HIGH
    // DIRECT PIN TO PREVENT RAM CORRUPTION
    // DIFFERENCE BETWEEN M2 LO AND ROMSEL HI MUST BE AROUND 33ns
    // IF TIME IS GREATER THAN 33ns THEN WRITES TO 0xE000/0xF000 WILL CORRUPT RAM AT 0x6000/0x7000
    PORTF = 0b01111101;  // ROMSEL LO/M2 HI
    PORTF = 0b01111110;  // ROMSEL HI/M2 LO
    _delay_us(1);
    // Back to read mode
    NES_PRG_READ;
    pinControl.setMode(DataMode::kRead);
    pinControl.setAddress(0);
    // Set phi2 to high state to keep cartridge unreseted
    NES_PHI2_HI;
  }

  static void write_ram_byte(uint16_t address, uint8_t data)
  {  // Mapper 19 (Namco 106/163) WRITE RAM SAFE ($E000-$FFFF)
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    pinControl.setMode(DataMode::kWrite);
    NES_PRG_WRITE;
    pinControl.writeData(data);

    pinControl.setAddress(address);  // PHI2 low, ROMSEL always HIGH
    NES_PHI2_HI;
    NES_ROMSEL_LOW;    // SET /ROMSEL LOW OTHERWISE CORRUPTS RAM
    _delay_us(1);  // WRITING
    // PHI2 low, ROMSEL high
    NES_PHI2_LOW;
    _delay_us(1);
    NES_ROMSEL_HI;
    // Back to read mode
    NES_PRG_READ;
    pinControl.setMode(DataMode::kRead);
    pinControl.setAddress(0);
    // Set phi2 to high state to keep cartridge unreseted
    NES_PHI2_HI;
  }

  static void write_wram_byte(uint16_t address, uint8_t data)
  {  // Mapper 5 (MMC5) RAM
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    pinControl.setAddress(address);
    pinControl.writeData(data);

    _delay_us(1);
    pinControl.setMode(DataMode::kWrite);
    NES_PRG_WRITE;
    NES_PHI2_HI;
    _delay_us(1);  // WRITING
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    // Back to read mode
    NES_PRG_READ;
    pinControl.setMode(DataMode::kRead);
    pinControl.setAddress(0);
    // Set phi2 to high state to keep cartridge unreseted
    NES_PHI2_HI;
  }

# else /* !ENABLE_PINCONTROL */

  // WRITE RAM SAFE TO REGISTERS 0xE000/0xF000
  void write_reg_byte(uint16_t address, uint8_t data)
  {  // FIX FOR MMC1 RAM CORRUPTION
    NES_PHI2_LOW;
    NES_ROMSEL_HI;  // A15 HI = E000
    NES_MODE_WRITE;
    NES_PRG_WRITE;  // CPU R/W LO
    PORTK = data;

    set_address(address);  // PHI2 low, ROMSEL always HIGH
    // DIRECT PIN TO PREVENT RAM CORRUPTION
    // DIFFERENCE BETWEEN M2 LO AND ROMSEL HI MUST BE AROUND 33ns
    // IF TIME IS GREATER THAN 33ns THEN WRITES TO 0xE000/0xF000 WILL CORRUPT RAM AT 0x6000/0x7000
    PORTF = 0b01111101;  // ROMSEL LO/M2 HI
    PORTF = 0b01111110;  // ROMSEL HI/M2 LO
    _delay_us(1);
    // Back to read mode
    NES_PRG_READ;
    NES_MODE_READ;
    set_address(0);
    // Set phi2 to high state to keep cartridge unreseted
    NES_PHI2_HI;
  }

  void write_ram_byte(uint16_t address, uint8_t data)
  {  // Mapper 19 (Namco 106/163) WRITE RAM SAFE ($E000-$FFFF)
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    NES_MODE_WRITE;
    NES_PRG_WRITE;
    PORTK = data;

    set_address(address);  // PHI2 low, ROMSEL always HIGH
    NES_PHI2_HI;
    NES_ROMSEL_LOW;    // SET /ROMSEL LOW OTHERWISE CORRUPTS RAM
    _delay_us(1);  // WRITING
    // PHI2 low, ROMSEL high
    NES_PHI2_LOW;
    _delay_us(1);
    NES_ROMSEL_HI;
    // Back to read mode
    NES_PRG_READ;
    NES_MODE_READ;
    set_address(0);
    // Set phi2 to high state to keep cartridge unreseted
    NES_PHI2_HI;
  }

  void write_wram_byte(uint16_t address, uint8_t data)
  {  // Mapper 5 (MMC5) RAM
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    set_address(address);
    PORTK = data;

    _delay_us(1);
    NES_MODE_WRITE;
    NES_PRG_WRITE;
    NES_PHI2_HI;
    _delay_us(1);  // WRITING
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    // Back to read mode
    NES_PRG_READ;
    NES_MODE_READ;
    set_address(0);
    // Set phi2 to high state to keep cartridge unreseted
    NES_PHI2_HI;
  }

# endif /* ENABLE_PINCONTROL */

  // Pirate Mapper 59
  static void write_reg_m59(uint16_t address)
  {
    NES_ROMSEL_HI;
    NES_MODE_WRITE;
    NES_PRG_WRITE;
    set_address(address);
    set_romsel(address);
    _delay_us(1);  // WRITING
    NES_ROMSEL_HI;
    NES_PRG_READ;
    NES_MODE_READ;
    set_address(0);
  }

  // Multicart Mappers 226/289/332
  static void write_prg_pulsem2(uint16_t address, uint8_t data)
  {
    NES_PHI2_LOW;
    NES_ROMSEL_HI;
    NES_PHI2_HI;
    NES_MODE_WRITE;
    NES_PHI2_LOW;
    NES_PRG_WRITE;
    NES_PHI2_HI;
    PORTK = data;
    NES_PHI2_LOW;
    set_address(address);  // PHI2 low, ROMSEL always HIGH
    NES_PHI2_HI;
    set_romsel(address);  // ROMSEL is low if need, PHI2 high
    for (uint16_t i = 0; i < 8; i++) {
      NES_PHI2_LOW;
      NES_PHI2_HI;
    }
    NES_PHI2_LOW;
    for (uint16_t i = 0; i < 8; i++) {
      NES_PHI2_LOW;
      NES_PHI2_HI;
    }
    NES_ROMSEL_HI;
    NES_PHI2_LOW;
    NES_PRG_READ;
    NES_PHI2_HI;
    NES_MODE_READ;
    NES_PHI2_LOW;
    set_address(0);
    NES_PHI2_HI;
  }

  static uint8_t read_prg_pulsem2(uint16_t address)
  {
    NES_PHI2_LOW;
    NES_MODE_READ;
    NES_PHI2_HI;
    NES_PRG_READ;
    NES_PHI2_LOW;
    set_address(address);
    NES_PHI2_HI;
    set_romsel(address);
    NES_PHI2_LOW;
    for (uint16_t i = 0; i < 8; i++) {
      NES_PHI2_LOW;
      NES_PHI2_HI;
    }
    return PINK;
  }

  void dumpPRG_pulsem2(uint16_t base, uint16_t address)
  {
    for (size_t x = 0; x < 512; x++)
    {
      OSCR::Storage::Shared::buffer[x] = read_prg_pulsem2(base + address + x);
    }

    OSCR::Storage::Shared::writeBuffer();

    OSCR::UI::ProgressBar::advance(512);
  }

  // Multicart Mapper 332
  static uint8_t read_chr_pulsem2(uint16_t address)
  {
    NES_PHI2_LOW;
    NES_MODE_READ;
    NES_PHI2_HI;
    NES_ROMSEL_HI;
    NES_PHI2_LOW;
    set_address(address);
    NES_PHI2_HI;
    NES_CHR_READ_LOW;
    NES_PHI2_LOW;
    for (uint16_t i = 0; i < 8; i++) {
      NES_PHI2_LOW;
      NES_PHI2_HI;
    }
    uint8_t result = PINK;
    NES_PHI2_LOW;
    NES_CHR_READ_HI;
    NES_PHI2_HI;
    return result;
  }

  void dumpCHR_pulsem2(uint16_t address)
  {
    for (int x = 0; x < 512; x++)
    {
      OSCR::Storage::Shared::buffer[x] = read_chr_pulsem2(address + x);
    }

    OSCR::Storage::Shared::writeBuffer();

    OSCR::UI::ProgressBar::advance(512);
  }

  /******************************************
     File Functions
  *****************************************/

  //createNewFile fails to dump RAM if ROM isn't dumped first
  //void CreateRAMFileInSD() {
  //createNewFile("RAM", "bin");
  //}
  //Temporary fix
  void CreateRAMFileInSD()
  {
    char fileCount[7];

    snprintf_P(BUFFN(OSCR::Storage::Shared::sharedFileName), OSCR::Storage::FilenameTemplatePP, OSCR::Strings::Common::RAM, OSCR::Strings::FileType::Save);

    for (uint8_t i = 0; i < 100; i++)
    {
      if (!OSCR::Storage::sd.exists(OSCR::Storage::Shared::sharedFileName))
      {
        OSCR::Storage::Shared::sharedFile.open(OSCR::Storage::Shared::sharedFileName, O_RDWR | O_CREAT);
        return;
      }

      snprintf_P(BUFFN(fileCount), PSTR("%S.%02d.%S"), OSCR::Strings::Common::RAM, i, OSCR::Strings::FileType::Save);
    }

    OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Cores::NES));
    OSCR::UI::ncFatalErrorStorage();
  }

  /******************************************
     Config Functions
  *****************************************/
  bool setMapper()
  {
    uint16_t currentMapper = 0;

    if (mapperDetail != nullptr)
    {
      currentMapper = mapperDetail->mapper;
    }

    if (currentMapper > 220)
    {
      currentMapper = 0;
    }

    currentMapper = OSCR::UI::rangeSelect(FS(OSCR::Strings::Headings::SelectMapper), 0, 3);

    // Check if valid
    bool validMapper = false;

    for (uint8_t currMaplist = 0; currMaplist < nesMapperCRDB->numRecords(); currMaplist++)
    {
      if (mapperDetail->mapper == currentMapper)
      {
        validMapper = true;
      }
    }

    if (!validMapper) return false;

    NES_MAPPER = mapperDetail->mapper;
    NES_SUBMAPPER = mapperDetail->submapper;

    return true;
  }

  void checkMapperSize()
  {
    prglo = mapperDetail->prglo;
    prghi = mapperDetail->prghi;
    chrlo = mapperDetail->chrlo;
    chrhi = mapperDetail->chrhi;
    ramlo = mapperDetail->ramlo;
    ramhi = mapperDetail->ramhi;
  }

  void setPRGSize()
  {
    if (prglo == prghi)
    {
      NES_PRG = prglo;
    }
    else
    {
      NES_PRG = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeK), prgSizes, sizeofarray(prgSizes));
    }
  }

  void setCHRSize()
  {
    if (chrlo == chrhi)
    {
      NES_CHR = chrlo;
    }
    else
    {
      NES_CHR = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeK), chrSizes, sizeofarray(chrSizes));
    }
  }

  void setRAMSize()
  {
    if (ramlo == ramhi)
    {
      NES_RAM = ramlo;
    }
    else
    {
      NES_RAM = OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeK), ramSizes, sizeofarray(ramSizes));
    }
  }

  // MMC6 Detection
  // Mapper 4 includes both MMC3 AND MMC6
  // RAM is mapped differently between MMC3 and MMC6
  void checkMMC6()
  {               // Detect MMC6 Carts - read PRG 0x3E00A ("STARTROPICS")
    write_prg_byte(0x8000, 6);     // PRG Bank 0 ($8000-$9FFF)
    write_prg_byte(0x8001, 0x1F);  // 0x3E000

    uint8_t prgchk0 = read_prg_byte(0x800A);
    uint8_t prgchk1 = read_prg_byte(0x800B);
    uint8_t prgchk2 = read_prg_byte(0x800C);
    uint8_t prgchk3 = read_prg_byte(0x800D);

    if ((prgchk0 == 0x53) && (prgchk1 == 0x54) && (prgchk2 == 0x41) && (prgchk3 == 0x52))
    {
      mmc6 = true;  // MMC6 Cart
    }
  }

  void checkStatus()
  {
    NES_PRGSIZE = (OSCR::Util::power<2>(NES_PRG)) * 16;

    if (NES_CHR == 0)
    {
      NES_CHRSIZE = 0;  // 0K
    }
    else
    {
      NES_CHRSIZE = (OSCR::Util::power<2>(NES_CHR)) * 4;
    }

    if (NES_RAM == 0)
    {
      NES_RAMSIZE = 0;  // 0K
    }
    else if (NES_MAPPER == 82)
    {
      NES_RAMSIZE = 5;  // 5K
    }
    else
    {
      NES_RAMSIZE = (OSCR::Util::power<2>(NES_RAM)) * 4;
    }

    // Mapper Variants
    // Identify variant for use across multiple functions
    if (NES_MAPPER == 4) {  // Check for MMC6/MMC3
      checkMMC6();
      if (mmc6) {
        NES_RAMSIZE = 1;      // 1K
        NES_RAM = 1;  // Must be a non-zero value
      }
    }
# if HAS_FLASH
    else if (NES_MAPPER == 30)  // Check for Flashable/Non-Flashable
    {
      NESmaker_ID();  // Flash ID
    }
# endif

    printNESSettings();

    OSCR::UI::waitButton();
  }

  /******************************************
     ROM Functions
  *****************************************/
  void dumpPRG(uint16_t base, uint16_t address)
  {
    for (size_t x = 0; x < 512; x++)
    {
      OSCR::Storage::Shared::buffer[x] = read_prg_byte(base + address + x);
    }

    OSCR::Storage::Shared::writeBuffer();

    OSCR::UI::ProgressBar::advance(512);
  }

  void dumpCHR(uint16_t address)
  {
    for (size_t x = 0; x < 512; x++)
    {
      OSCR::Storage::Shared::buffer[x] = read_chr_byte(address + x);
    }

    OSCR::Storage::Shared::writeBuffer();

    OSCR::UI::ProgressBar::advance(512);
  }

  void dumpCHR_M2(uint16_t address)
  {  // MAPPER 45 - PULSE M2 LO/HI
    for (size_t x = 0; x < 512; x++)
    {
      NES_PHI2_LOW;
      OSCR::Storage::Shared::buffer[x] = read_chr_byte(address + x);
    }
    OSCR::Storage::Shared::writeBuffer();
    OSCR::UI::ProgressBar::advance(512);
  }

  void dumpMMC5RAM(uint16_t base, uint16_t address)
  {  // MMC5 SRAM DUMP - PULSE M2 LO/HI
    for (size_t x = 0; x < 512; x++)
    {
      NES_PHI2_LOW;
      OSCR::Storage::Shared::buffer[x] = read_prg_byte(base + address + x);
    }
    OSCR::Storage::Shared::writeBuffer();
    OSCR::UI::ProgressBar::advance(512);
  }

  void writeMMC5RAM(uint16_t base, uint16_t address)
  {  // MMC5 SRAM WRITE
    uint8_t bytecheck;

    OSCR::Storage::Shared::fill();

    for (size_t x = 0; x < 512; x++)
    {
      do
      {
        write_prg_byte(0x5102, 2);  // PRG RAM PROTECT1
        write_prg_byte(0x5103, 1);  // PRG RAM PROTECT2
        write_wram_byte(base + address + x, OSCR::Storage::Shared::buffer[x]);
        bytecheck = read_prg_byte(base + address + x);
      }
      while (bytecheck != OSCR::Storage::Shared::buffer[x]);  // CHECK WRITTEN BYTE
    }

    write_prg_byte(0x5102, 0);  // PRG RAM PROTECT1
    write_prg_byte(0x5103, 0);  // PRG RAM PROTECT2
  }

  void dumpBankPRG(size_t const from, size_t const to, size_t const base)
  {
    for (size_t address = from; address < to; address += 512)
    {
      dumpPRG(base, address);
    }
  }

  void dumpBankCHR(size_t const from, size_t const to)
  {
    for (size_t address = from; address < to; address += 512)
    {
      dumpCHR(address);
    }
  }

  void readPRG(bool readrom)
  {
    if (!readrom)
    {
      printHeader();

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::NES), FS(OSCR::Strings::Directory::ROM), "PRG", FS(OSCR::Strings::FileType::Raw));

      cartOn();
    }

    set_address(0);
    _delay_us(1);

    uint16_t base = 0x8000;
    bool busConflict = false;
    uint16_t banks;

    switch (NES_MAPPER)
    {
      case 0:
      case 3:
      case 13:
      case 87:  // 16K/32K
      case 122:
      case 184:  // 32K
      case 185:  // 16K/32K
        if (oldcrc32 == 0xE37A6AA8 || oldcrc32 == 0x90046A48 || oldcrc32 == 0xCBA2352F || oldcrc32 == 0x56DA99FA || oldcrc32MMC3 == 0xE37A6AA8 || oldcrc32MMC3 == 0x90046A48 || oldcrc32MMC3 == 0xCBA2352F || oldcrc32MMC3 == 0x56DA99FA)
        {
          //OSCR::UI::printLineSync(F("DUMPING 8KiB PRG"));
          dumpBankPRG(0x0, 0x2000, base);
        }
        else
        {
          banks = OSCR::Util::power<2>(NES_PRG);
          dumpBankPRG(0x0, 0x4000 * banks, base);
        }
        break;

      case 1:
      case 155:  // 32K/64K/128K/256K/512K
        if (NES_PRG == 1)
        {
          write_prg_byte(0x8000, 0x80);
          dumpBankPRG(0x0, 0x8000, base);
        }
        else
        {
          banks = OSCR::Util::power<2>(NES_PRG);

          for (size_t i = 0; i < banks; i++) // 16K Banks ($8000-$BFFF)
          {
            write_prg_byte(0x8000, 0x80); // Clear Register
            write_mmc1_byte(0x8000, 0x0C); // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)

            if (NES_PRG > 4) // 512K
            {
              // Reset 512K Flag for Lower 256K
              write_mmc1_byte(0xA000, 0);
            }

            if (i > 15) // Switch Upper 256K
            {
              // Set 512K Flag
              write_mmc1_byte(0xA000, 0x10);
            }

            write_mmc1_byte(0xE000, i);
            dumpBankPRG(0x0, 0x4000, base);
          }
        }
        break;

      case 2:   // bus conflicts - fixed last bank
      case 30:  // bus conflicts in non-flashable configuration
        banks = OSCR::Util::power<2>(NES_PRG);

        busConflict = true;

        for (size_t i = 0; i < banks; i++)
        {
          for (size_t x = 0; x < 0x8000; x++)
          {
            if (read_prg_byte(0xC000 + x) == i)
            {
              write_prg_byte(0xC000 + x, i);
              busConflict = false;
              break;
            }
          }

          if (busConflict)
          {
            write_prg_byte(0xC000 + i, i);
          }

          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 4:
      case 12:
      case 37:
      case 44:
      case 47:
      case 49:
      case 52:
      case 64:
      case 74:
      case 76:
      case 88:
      case 95:
      case 115:
      case 116:
      case 118:
      case 119:
      case 126:
      case 134:
      case 154:  // 128K
      case 158:
      case 165:  // 512K
      case 176:
      case 195:
      case 196:
      case 206:  // 32/64/128K
      case 224:
      case 245:  // 1024K
      case 248:
      case 268:  // submapper 0
      case 315:
      case 351:
      case 366:
      case 422:
      case 534:
      case 995:
      case 996:
      case 997:
      case 998:
      case 999:
        if ((NES_MAPPER == 206) && (NES_PRG == 1))
        {
          dumpBankPRG(0x0, 0x8000, base);
        }
        else
        {
          banks = OSCR::Util::power<2>(NES_PRG) * 2;

          write_prg_byte(0xA001, 0x80);  // Block Register - PRG RAM Chip Enable, Writable

          if ((NES_MAPPER == 126) || (NES_MAPPER == 422) || (NES_MAPPER == 534))
          {
            write_prg_byte(0x6803, 0);  // set MMC3 banking mode
          }

          if ((NES_MAPPER == 115) || (NES_MAPPER == 134) || (NES_MAPPER == 248))
          {
            write_prg_byte(0x6000, 0);  // set MMC3 banking mode
          }

          if (NES_MAPPER == 176)
          {
            write_prg_byte(0x5FF3, 0);  // extended MMC3 mode: disabled
            write_prg_byte(0x5FF0, 1);  // 256K outer bank mode
          }

          if (NES_MAPPER == 351)
          {
            write_prg_byte(0x5000, 0);
            write_prg_byte(0x5001, 0);
            write_prg_byte(0x5002, 0);
          }

          for (size_t i = 0; i < banks; i++)
          {
            switch(NES_MAPPER)
            {
            case 37:
              if (i == 0)
              {
                write_prg_byte(0x6000, 0);  // Switch to Lower Block ($0000-$FFFF)
              }
              else if (i == 8)
              {
                write_prg_byte(0x6000, 3);  // Switch to 2nd 64K Block ($10000-$1FFFF)
              }
              else if (i == 16)
              {
                write_prg_byte(0x6000, 4);  // Switch to 128K Block ($20000-$3FFFF)
              }
              break;

            case 44:
              write_prg_byte(0xA001, 0x80 | ((i >> 4) & 0x07));
              break;

            case 47:
              write_prg_byte(0x6000 + (i >> 4), 0);
              break;

            case 49:
              write_prg_byte(0x6000, ((i & 0x30) << 2) | 1);
              break;

            case 52:
              write_prg_byte(0x6000, (i & 0x70) >> 4);
              break;

            case 115:
            case 248:
              write_prg_byte(0x6000, (i & 0x20) << 1);  // A18
              break;

            case 116:
              write_prg_byte(0x4100, 0x01);  // MMC3 mode
              break;

            case 126:
            case 422:
            case 534:
              write_prg_byte(0x6800, (i & 0x300) >> 4 | (i & 0x70) >> 4);  // submapper 0
              // write_prg_byte(0x6800, (i & 0x80) >> 2 | (i & 0x70) >> 4); // submapper 1
              break;

            case 134:
              write_prg_byte(0x6000, (i & 0x40) >> 2);  // A19
              write_prg_byte(0x6001, (i & 0x30) >> 4);  // A18-17
              break;

            case 176:
              write_prg_byte(0x5FF1, (i & 0xE0) >> 1);
              break;

            case 245: // uses CHR A11 as PRG A19
              if (i == 0)
              {
                write_prg_byte(0x8000, 0);
                write_prg_byte(0x8001, 0);
              }

              if (i == 64)
              {
                write_prg_byte(0x8000, 0);
                write_prg_byte(0x8001, 0xFF);
              }
              break;

            case 224:
            case 268:
              write_prg_byte(0x5000, ((i & 0x70) >> 4) | ((i & 0xC00) >> 6));
              write_prg_byte(0x5001, ((i & 0x80) >> 3) | ((i & 0x300) >> 6) | 0x60);
              write_prg_byte(0x6000, ((i & 0x70) >> 4) | ((i & 0xC00) >> 6));
              write_prg_byte(0x6001, ((i & 0x80) >> 3) | ((i & 0x300) >> 6) | 0x60);
              break;

            case 315:
              write_prg_byte(0x6800, (i & 30) >> 3);
              break;

            case 351:
              write_prg_byte(0x5001, i << 1);
              break;

            case 366:
              write_prg_byte(0x6800 + (i & 0x70), i);
              break;

            case 995:
              write_prg_byte(0x5000, ((i & 0x70) >> 4) | ((i & 0xC00) >> 6));
              write_prg_byte(0x5001, ((i & 0x80) >> 4) | ((i & 0x100) >> 6) | ((i & 0x200) >> 8) | 0x60);
              write_prg_byte(0x6000, ((i & 0x70) >> 4) | ((i & 0xC00) >> 6));
              write_prg_byte(0x6001, ((i & 0x80) >> 4) | ((i & 0x100) >> 6) | ((i & 0x200) >> 8) | 0x60);
              break;

            case 996:
              write_prg_byte(0x5000, ((i & 0x70) >> 4) | ((i & 0x180) >> 3));
              write_prg_byte(0x5001, 0x60);
              write_prg_byte(0x6000, ((i & 0x70) >> 4) | ((i & 0x180) >> 3));
              write_prg_byte(0x6001, 0x60);
              break;

            case 997:
              if (i >= banks / 2)
              {
                write_prg_byte(0x5000, ((i & 0x70) >> 4) | ((i & 0xC00) >> 6) | 0x88);
                write_prg_byte(0x6000, ((i & 0x70) >> 4) | ((i & 0xC00) >> 6) | 0x88);
              }
              else
              {
                write_prg_byte(0x5000, ((i & 0x70) >> 4) | ((i & 0xC00) >> 6));
                write_prg_byte(0x6000, ((i & 0x70) >> 4) | ((i & 0xC00) >> 6));
              }

              write_prg_byte(0x6001, ((i & 0x80) >> 3) | ((i & 0x300) >> 6) | 0x60);
              write_prg_byte(0x6002, 0x00);
              write_prg_byte(0x6003, 0x00);
              break;

            case 998:
            case 999:
              write_prg_byte(0x5000, ((i & 0x70) >> 4));
              write_prg_byte(0x5001, 0x60);
              write_prg_byte(0x6000, ((i & 0x70) >> 4));
              write_prg_byte(0x6001, 0x60);
              break;
            }

            switch(NES_MAPPER)
            {
            case 224:
            case 268:
            case 995:
            case 996:
            case 997:
            case 998:
            case 999:
              write_prg_byte(0x5002, 0x00);
              write_prg_byte(0x5003, 0x00);
              write_prg_byte(0x6002, 0x00);
              write_prg_byte(0x6003, 0x00);
              break;
            }

            write_prg_byte(0x8000, 0x06);  // PRG Bank 0 ($8000-$9FFF)
            write_prg_byte(0x8001, i);
            dumpBankPRG(0x0, 0x2000, base);
          }
        }
        break;

      case 5:  // 128K/256K/512K
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        write_prg_byte(0x5100, 3);               // 8K PRG Banks

        for (size_t i = 0; i < banks; i += 2) // 128K/256K/512K
        {
          write_prg_byte(0x5114, i | 0x80);
          write_prg_byte(0x5115, (i + 1) | 0x80);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 7:   // 128K/256K
      case 34:  // BxROM/NINA
      case 39:
      case 77:
      case 96:   // 128K
      case 177:  // up to 1024K
      case 241:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++) // 32K Banks
        {
          if (NES_MAPPER == 34)
          {
            write_prg_byte(0x7FFD, i);  // NINA Bank select
            delay(200);                 // NINA seems slow to switch banks
          }

          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x8000, base);  // 32K Banks ($8000-$FFFF)
        }
        break;

      case 9:
        banks = OSCR::Util::power<2>(NES_PRG) * 2; // 8K banks

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xA000, i);       // Switch bank at $8000
          dumpBankPRG(0x0, 0x2000, base);  //
        }
        break;

      case 10:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xA000, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 11:
      case 144:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xFFB0 + i, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 15:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i += 2)
        {
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 16:
      case 159:  // 128K/256K
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x6008, i);       // Submapper 4
          write_prg_byte(0x8008, i);       // Submapper 5
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 18:  // 128K/256K
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i += 2)
        {
          write_prg_byte(0x8000, i & 0xF);
          write_prg_byte(0x8001, (i >> 4) & 0xF);
          write_prg_byte(0x8002, (i + 1) & 0xF);
          write_prg_byte(0x8003, ((i + 1) >> 4) & 0xF);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 19:  // 128K/256K
      case 532:
        for (size_t j = 0; j < 64; j++) // Init Register
        {
          write_ram_byte(0xE000, 0);       // PRG Bank 0 ($8000-$9FFF)
        }

        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_ram_byte(0xE000, i);  // PRG Bank 0 ($8000-$9FFF)
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 21:  // 256K
        banks = OSCR::Util::power<2>(NES_PRG) * 2;
        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xA000, i);
          dumpBankPRG(0x2000, 0x4000, base);
        }
        break;

      case 22:
      case 23:
      case 25:
      case 65:
      case 75:  // 128K/256K
      case 272:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;
        if (NES_MAPPER == 23)
        {
          write_prg_byte(0x9002, 0);
          write_prg_byte(0x9008, 0);
        }

        if (NES_MAPPER == 25)
        {
          write_prg_byte(0x9005, 0);  // set vrc4 swap setting for TMNT2
        }

        if (NES_MAPPER == 272)
        {
          write_prg_byte(0x9002, 0);
          write_prg_byte(0x9004, 0);
        }

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 24:
      case 26:  // 256K
      case 29:
      case 78:  // 128K
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 27:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xA000, i);
          dumpBankPRG(0x2000, 0x4000, base);
        }
        break;

      case 28:  // using 32k mode for inner and outer banks, switching only with outer
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        write_prg_byte(0x5000, 0x81);
        write_prg_byte(0x8000, 0);
        write_prg_byte(0x5000, 0x80);
        write_prg_byte(0x8000, 0);
        write_prg_byte(0x5000, 0x01);
        write_prg_byte(0x8000, 0);
        write_prg_byte(0x5000, 0);
        write_prg_byte(0x8000, 0);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x5000, 0x81);
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 31:
        banks = OSCR::Util::power<2>(NES_PRG) * 4;

        for (size_t i = 0; i < banks; i += 8)
        {
          write_prg_byte(0x5FF8, i);
          write_prg_byte(0x5FF9, i + 1);
          write_prg_byte(0x5FFA, i + 2);
          write_prg_byte(0x5FFB, i + 3);
          write_prg_byte(0x5FFC, i + 4);
          write_prg_byte(0x5FFD, i + 5);
          write_prg_byte(0x5FFE, i + 6);
          write_prg_byte(0x5FFF, i + 7);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 32:  // 128K/256K
        banks = OSCR::Util::power<2>(NES_PRG) * 2;
        for (size_t i = 0; i < banks; i++) {  // 128K/256K
          write_prg_byte(0x9000, 1);          // PRG Mode 0 - Read $A000-$BFFF to avoid difference between Modes 0 and 1
          write_prg_byte(0xA000, i);          // PRG Bank
          dumpBankPRG(0x2000, 0x4000, base);  // 8K Banks ($A000-$BFFF)
        }
        break;

      case 33:
      case 48:  // 128K/256K
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i += 2)
        {
          write_prg_byte(0x8000, i);       // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i + 1);   // PRG Bank 1 ($A000-$BFFF)
          dumpBankPRG(0x0, 0x4000, base);  // 8K Banks ($A000-$BFFF)
        }
        break;

      case 35:
      case 90:
      case 209:
      case 211:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        write_prg_byte(0xD000, 0x02);

        for (uint8_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xD003, ((i >> 5) & 0x06) | 0x20);
          write_prg_byte(0x8000, (i & 0x3F));
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 36:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xFFA0 + i, (i << 4));
          write_prg_byte(0x4101, 0);
          write_prg_byte(0x4102, (i << 4));
          write_prg_byte(0x4103, 0);
          write_prg_byte(0x4100, 0);
          write_prg_byte(0x4103, 0xFF);
          write_prg_byte(0xFFFF, 0xFF);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 38:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x7000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 40:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xFFFF, i);
          dumpBankPRG(0x4000, 0x6000, base);
        }

        if (NES_PRG > 2)
        {
          write_prg_byte(0xC018, 0);
          dumpBankPRG(0x0, 0x8000, base);
          write_prg_byte(0xC058, 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 41:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x6000 + i, 0);
          write_prg_byte(0x6000 + i, 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 42:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;
        base = 0x6000;  // 8k switchable PRG ROM bank at $6000-$7FFF

        for (size_t i = 0; i < banks - 4; i++)
        {
          write_prg_byte(0xE000, i & 0x0F);
          dumpBankPRG(0x0, 0x2000, base);
        }

        base = 0x8000;  // last 32k fixed to $8000-$FFFF

        dumpBankPRG(0x0, 0x8000, base);
        break;

      case 43:
        base = 0xC000;

        for (size_t i = 0; i < 8; i++)
        {
          write_prg_byte(0x4022, i);
          dumpBankPRG(0x0, 0x2000, base);
        }

        base = 0x5000;
        for (size_t i = 0; i < 4; i++)
        {
          dumpBankPRG(0x0, 0x1000, base);
        }

        base = 0xE000;
        dumpBankPRG(0x0, 0x1000, base);
        break;

      case 45:                                    // MMC3 Clone with Outer Registers
        banks = ((OSCR::Util::power<2>(NES_PRG) * 2)) - 2;  // Set Number of Banks
        for (size_t i = 0; i < banks; i += 2) // 128K/256K/512K/1024K
        {
          for (uint16_t address = 0x0; address < 0x2000; address += 512)
          {
            // set outer bank registers
            write_prg_byte(0x6000, 0x00);               // CHR-OR
            write_prg_byte(0x6000, (i & 0xC0));         // PRG-OR
            write_prg_byte(0x6000, ((i >> 2) & 0xC0));  // CHR-AND,CHR-OR/PRG-OR
            write_prg_byte(0x6000, 0x80);               // PRG-AND

            // set inner bank registers
            write_prg_byte(0x8000, 6);  // PRG Bank 0 ($8000-$9FFF)
            write_prg_byte(0x8001, i);
            dumpPRG(base, address);
          }

          for (uint16_t address = 0x2000; address < 0x4000; address += 512)
          {
            // set outer bank registers
            write_prg_byte(0x6000, 0x00);                     // CHR-OR
            write_prg_byte(0x6000, ((i + 1) & 0xC0));         // PRG-OR
            write_prg_byte(0x6000, (((i + 1) >> 2) & 0xC0));  // CHR-AND,CHR-OR/PRG-OR
            write_prg_byte(0x6000, 0x80);                     // PRG-AND

            // set inner bank registers
            write_prg_byte(0x8000, 7);  // PRG Bank 1 ($A000-$BFFF)
            write_prg_byte(0x8001, i + 1);
            dumpPRG(base, address);
          }
        }

        // Final 2 Banks ($C000-$FFFF)
        for (uint16_t address = 0x4000; address < 0x8000; address += 512)
        {
          // set outer bank registers
          write_prg_byte(0x6000, 0x00);  // CHR-OR
          write_prg_byte(0x6000, 0xC0);  // PRG-OR
          write_prg_byte(0x6000, 0xC0);  // CHR-AND,CHR-OR/PRG-OR
          write_prg_byte(0x6000, 0x80);  // PRG-AND
          dumpPRG(base, address);
        }
        break;

      case 46:
        banks = OSCR::Util::power<2>(NES_PRG) / 2; // 32k banks

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x6000, (i & 0x1E) >> 1); // high bits
          write_prg_byte(0x8000, i & 0x01); // low bit
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 50:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x4122, (i & 0x08) | ((i & 0x03) << 1) | ((i & 0x04) >> 2));
          dumpBankPRG(0xC000, 0xE000, base);
        }
        break;

      case 55:
        dumpBankPRG(0x0, 0x8000, base);

        base = 0x6000;

        for (size_t i = 0; i < 8; i++)
        {
          dumpBankPRG(0x0, 0x800, base);
        }
        break;

      case 56:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xE000, 1);
          write_prg_byte(0xF000, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 57:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          for (uint16_t address = 0; address < 0x4000; address += 512)
          {
            write_prg_pulsem2(0x8800, i << 5);
            write_prg_pulsem2(0x8000, 0x80);
            dumpPRG_pulsem2(base, address);
          }
        }
        break;

      case 58:
      case 213:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i += 2)
        {
          write_prg_byte(0x8000 + (i & 0x07), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 59:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i += 2)
        {
          write_reg_m59(0x8000 + ((i & 0x07) << 4));
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 60:
        dumpBankPRG(0x0, 0x4000, base);

        for (size_t i = 0; i < 3; i++)
        {
          write_prg_byte(0x8D8D, i);
          delay(500);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 61:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8010 | ((i & 0x01) << 5) | ((i >> 1) & 0x0F), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 62:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000 + (i * 512) + ((i & 32) << 1), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 63:  // 3072K total
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < 192; i++)
        {
          write_prg_byte(0x8000 + (i << 2), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 66:  // 64K/128K
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++) // 64K/128K
        {
          write_prg_byte(0x8000, i << 4); // bits 4-5
          dumpBankPRG(0x0, 0x8000, base); // 32K Banks ($8000-$FFFF)
        }
        break;

      case 67:  // 128K
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++) // 128K
        {
          write_reg_byte(0xF800, i);          // [WRITE RAM SAFE]
          dumpBankPRG(0x0, 0x4000, base);     // 16K Banks ($8000-$BFFF)
        }
        break;

      case 68:
      case 73:  // 128K
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++) // 128K
        {
          write_prg_byte(0xF000, i);
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 69:  // 128K/256K
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        write_prg_byte(0x8000, 8);            // Command Register - PRG Bank 0
        write_prg_byte(0xA000, 0);            // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF

        for (size_t i = 0; i < banks; i++) // 128K/256K
        {
          write_prg_byte(0x8000, 9);          // Command Register - PRG Bank 1
          write_prg_byte(0xA000, i);          // Parameter Register - ($8000-$9FFF)
          dumpBankPRG(0x0, 0x2000, base);     // 8K Banks ($8000-$9FFF)
        }
        break;

      case 70:
      case 89:
      case 152:  // 64K/128K
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++) // 128K
        {
          write_prg_byte(0x8000, i << 4);
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 71:  // 64K/128K/256K
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xC000, i);
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 72:  // 128K
        banks = OSCR::Util::power<2>(NES_PRG);

        write_prg_byte(0x8000, 0);            // Reset Register

        for (size_t i = 0; i < banks; i++) // 128K
        {
          write_prg_byte(0x8000, i | 0x80);   // PRG Command + Bank
          write_prg_byte(0x8000, i);          // PRG Bank
          dumpBankPRG(0x0, 0x4000, base);     // 16K Banks ($8000-$BFFF)
        }
        break;

      case 79:
      case 146:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x4100, i << 3);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 80:   // 128K
      case 207:  // 256K [CART SOMETIMES NEEDS POWERCYCLE]
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x7EFA, i);  // PRG Bank 0 ($8000-$9FFF)
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 82:  // 128K
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x7EFA, i << 2);  // PRG Bank 0 ($8000-$9FFF)
          dumpBankPRG(0x0, 0x2000, base);  // 8K Banks ($8000-$BFFF)
        }
        break;

      case 85:  // 128K/512K
      case 117:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000, i);       // PRG Bank 0 ($8000-$9FFF)
          dumpBankPRG(0x0, 0x2000, base);  // 8K Banks ($8000-$9FFF)
        }
        break;

      case 86:
      case 140:  // 128K
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++) // 128K
        {
          write_prg_byte(0x6000, i << 4);     // bits 4-5
          dumpBankPRG(0x0, 0x8000, base);     // 32K Banks ($8000-$FFFF)
        }
        break;

      case 91:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000 + ((i & 0x30) >> 3), i);  // PRG A18-A17 (submapper 0 only)
          write_prg_byte(0x7000, i);                      // PRG -A13
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 92:  // 256K
        banks = OSCR::Util::power<2>(NES_PRG);

        write_prg_byte(0x8000, 0); // Reset Register

        for (size_t i = 0; i < banks; i++) // 256K
        {
          write_prg_byte(0x8000, i | 0x80);   // PRG Command + Bank
          write_prg_byte(0x8000, i);          // PRG Bank
          dumpBankPRG(0x4000, 0x8000, base);  // 16K Banks ($C000-$FFFF)
        }
        break;

      case 93:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x6000, i);
          write_prg_byte(0x8000, i << 4 | 0x01);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 94:  // bus conflicts - fixed last bank
        banks = OSCR::Util::power<2>(NES_PRG);

        busConflict = true;

        for (size_t i = 0; i < banks; i++)
        {
          for (size_t x = 0; x < 0x4000; x++)
          {
            if (read_prg_byte(0xC000 + x) == (i << 2))
            {
              write_prg_byte(0xC000 + x, i << 2);
              busConflict = false;
              break;
            }
          }

          if (busConflict)
          {
            write_prg_byte(0x8000 + i, i << 2);
          }

          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 97:   // fixed first bank
      case 180:  // bus conflicts - fixed fist bank
        banks = OSCR::Util::power<2>(NES_PRG);

        dumpBankPRG(0x0, 0x4000, base);

        for (size_t i = 1; i < banks; i++)
        {
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x4000, 0x8000, base);
        }
        break;

      case 104:  // 1280K
        for (size_t i = 1; i < 80; i++)
        {
          write_prg_byte(0x8000, (i & 0x70) >> 4);  // outer bank
          write_prg_byte(0xC000, i & 0x0F);         // inner bank
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 105:                           // 256K
        write_mmc1_byte(0xA000, 0);       // Clear PRG Init/IRQ (Bit 4)
        write_mmc1_byte(0xA000, 0x10);    // Set PRG Init/IRQ (Bit 4) to enable bank swapping

        for (size_t i = 0; i < 4; i++) // PRG CHIP 1 128K
        {
          write_mmc1_byte(0xA000, i << 1);
          dumpBankPRG(0x0, 0x8000, base);  // 32K Banks ($8000-$FFFF)
        }

        write_mmc1_byte(0x8000, 0x0C);    // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
        write_mmc1_byte(0xA000, 0x08);    // Select PRG CHIP 2 (Bit 3)

        for (size_t j = 0; j < 8; j++) // PRG CHIP 2 128K
        {
          write_mmc1_byte(0xE000, j);
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 107:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xC000, i << 1);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 111:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x5000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 112:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000, 0);
          write_prg_byte(0xA000, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 113:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x4100, (i & 0x07) << 3);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 114:  // Submapper 0
      case 182:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        write_prg_byte(0x6000, 0);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xA000, 4);
          write_prg_byte(0xC000, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 120:
        base = 0x6000;
        for (size_t i = 0; i < 8; i++) {
          write_prg_byte(0x41FF, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        base = 0x8000;
        dumpBankPRG(0x0, 0x8000, base);
        break;

      case 125:
        base = 0x6000;
        for (size_t i = 0; i < 16; i++) {
          write_prg_byte(0x6000, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 142:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;
        base = 0x6000;  // 4x 8k switchable PRG ROM banks at $6000-$DFFF
        for (size_t i = 0; i < banks; i += 4) {
          write_prg_byte(0xE000, 4);  // Select 8 KB PRG bank at CPU $6000-$7FFF
          write_prg_byte(0xF000, i);
          write_prg_byte(0xE000, 1);  // Select 8 KB PRG bank at CPU $8000-$9FFF
          write_prg_byte(0xF000, i + 1);
          write_prg_byte(0xE000, 2);  // Select 8 KB PRG bank at CPU $A000-$BFFF
          write_prg_byte(0xF000, i + 2);
          write_prg_byte(0xE000, 3);  // Select 8 KB PRG bank at CPU $C000-$DFFF
          write_prg_byte(0xF000, i + 3);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 148:  // Sachen SA-008-A and Tengen 800008 -- Bus conflicts
        banks = OSCR::Util::power<2>(NES_PRG) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000, i << 3);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 153:  // 512K
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {  // 512K
          write_prg_byte(0x8000, i >> 4);     // PRG Outer Bank (Documentation says duplicate over $8000-$8003 registers)
          write_prg_byte(0x8001, i >> 4);     // PRG Outer Bank
          write_prg_byte(0x8002, i >> 4);     // PRG Outer Bank
          write_prg_byte(0x8003, i >> 4);     // PRG Outer Bank
          write_prg_byte(0x8008, i & 0xF);    // PRG Inner Bank
          dumpBankPRG(0x0, 0x4000, base);     // 16K Banks ($8000-$BFFF)
        }
        break;

      case 157:
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8008, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 162:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;
        write_prg_byte(0x5300, 0x07);  // A16-A15 controlled by $5000
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x5200, (i & 0x30) >> 4);  // A20-A19
          write_prg_byte(0x5000, i & 0x0F);         // A18-A15
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 163:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;
        write_prg_byte(0x5300, 0x04);  // disable bit swap on writes to $5000-$5200
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x5200, (i & 0x30) >> 4);  // A20-A19
          write_prg_byte(0x5000, i & 0x0F);         // A18-A15
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 168:
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000, i << 6);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 174:  // 128K
        for (size_t i = 0; i < 8; i++) {
          write_prg_byte(0xFF00 + (i << 4), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 175:  // 256K
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0xA000, i << 2);
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 178:
        banks = OSCR::Util::power<2>(NES_PRG);
        write_prg_byte(0x4800, 0);  // NROM-256 mode
        write_prg_byte(0x4803, 0);  // set PRG-RAM
        for (size_t i = 0; i < banks; i += 2) {
          for (uint16_t address = 0x0; address < 0x8000; address += 512) {
            write_prg_byte(0x4802, i >> 3);    // high PRG (up to 8 bits?!)
            write_prg_byte(0x4801, i & 0x07);  // low PRG (3 bits)
            dumpPRG(base, address);
          }
        }
        break;

      case 189:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x4132, (i << 4) & 0xF0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 200:
      case 212:
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 + (i & 0x07), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 201:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 + (i & 0xFF), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 202:
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 | (i << 1), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 203:
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000, (i & 0x1F) << 2);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 210:  // 128K/256K
        banks = OSCR::Util::power<2>(NES_PRG) * 2;
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0xE000, i);      // PRG Bank 0 ($8000-$9FFF) [WRITE NO RAM]
          write_prg_byte(0xE800, i + 1);  // PRG Bank 1 ($A000-$BFFF) [WRITE NO RAM]
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 214:
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 | (i << 2), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 221:
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 + ((i << 2) & 0xE0) + ((i << 3) & 0x200), 0);
          write_prg_byte(0xC000 + (i & 7), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 225:
      case 255:
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x9000 + ((i & 0x40) << 8) + ((i & 0x3F) << 6), i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      // MAPPER 226 - BMC SUPER 42-IN-1, BMC 76-IN-1 1024K/2048K
      // CART REQUIRES PULSING M2 TO ENABLE BANKSWITCH
      // WRITING DATA TO THE SD CARD BREAKS THE M2 PULSING SEQUENCE
      // SET THE BANK USING THE PULSING M2 CODE FOR EACH 512 BYTE BLOCK
      case 226:  // 1024K/2048K
        banks = OSCR::Util::power<2>(NES_PRG);
        for (size_t i = 0; i < banks; i += 2) {
          for (uint16_t address = 0x0; address < 0x8000; address += 512) {
            write_prg_pulsem2(0x8001, (i & 0x40) >> 6);
            write_prg_pulsem2(0x8000, ((i & 0x20) << 2) | (i & 0x1F));
            dumpPRG_pulsem2(base, address);
          }
        }
        break;

      case 227:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8083 + ((i & 0xF) << 3), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 228:
        banks = OSCR::Util::power<2>(NES_PRG);
        write_prg_byte(0x8000, 0);
        for (size_t i = 0; i < banks; i += 2) {  // up to 1024k PRG
          write_prg_byte(0x8000 + ((i & 0x3F) << 6), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        if (NES_PRG > 5) {  // reading the 3rd 512k PRG chip (Action 52)
          for (size_t i = 0; i < 32; i += 2) {
            write_prg_byte(0x9800 + ((i & 0x1F) << 6), 0);
            dumpBankPRG(0x0, 0x8000, base);
          }
        }
        break;

      case 229:
        write_prg_byte(0x8000, 0);
        dumpBankPRG(0x0, 0x8000, base);
        for (size_t i = 2; i < 32; i++) {
          write_prg_byte(0x8000 + i, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 232:
        if (oldcrc32 == 0x2B50A29C || oldcrc32 == 0x9DF7D376 || oldcrc32 == 0x596EC6E6 || oldcrc32 == 0xD003AFCA || oldcrc32 == 0x5004C6CF || oldcrc32 == 0x59D7D89D || oldcrc32 == 0x017D903E || oldcrc32 == 0xC1E6A786 || oldcrc32 == 0xD43AC4BE || oldcrc32 == 0x3E54E71D || oldcrc32 == 0x3D8497EA || oldcrc32MMC3 == 0x2B50A29C || oldcrc32MMC3 == 0x9DF7D376 || oldcrc32MMC3 == 0x596EC6E6 || oldcrc32MMC3 == 0xD003AFCA || oldcrc32MMC3 == 0x5004C6CF || oldcrc32MMC3 == 0x59D7D89D || oldcrc32MMC3 == 0x017D903E || oldcrc32MMC3 == 0xC1E6A786 || oldcrc32MMC3 == 0xD43AC4BE || oldcrc32MMC3 == 0x3E54E71D || oldcrc32MMC3 == 0x3D8497EA)
        {
          //OSCR::UI::printLineSync(F("DUMPING QUATTRO CART"));

          for (size_t i = 0; i < 16; i++)
          {
            write_prg_byte(0x8000, ((i & 0x04) << 2) | (i & 0x08));
            write_prg_byte(0xC000, i & 0x03);
            dumpBankPRG(0x0, 0x4000, base);
          }
        }
        else
        {
          banks = OSCR::Util::power<2>(NES_PRG) / 4;

          for (size_t outerbank = 0; outerbank < 4; outerbank++)
          {
            write_prg_byte(0x8000, outerbank << 3);

            for (size_t i = 0; i < banks; i++)
            {
              write_prg_byte(0xC000, i);
              dumpBankPRG(0x0, 0x4000, base);
            }
          }
        }
        break;

      // MAPPER 233 BMC 22-IN-1/20-IN-1 (42-IN-1) 1024K
      // ROM SPLIT INTO 512K + 512K CHIPS THAT COMBINE USING /CE + CE PINS
      // ENABLING PRG CHIP 1 DISABLES PRG CHIP 2 AND VICE VERSA
      // CART USES DIODE AND CAP TIED TO TC4520BP THAT CONTROLS THE ENABLE LINE TO THE PRG CHIPS
      // M2 IS NOT CONNECTED ON THIS CART UNLIKE OTHER RESET-BASED MULTICARTS
      // SWITCHING PRG CHIPS REQUIRES A POWER CYCLE OF THE CART READER
      // COMBINE THE LOWER AND UPPER DUMPS - BE SURE TO REMOVE ANY INES HEADER ON THE UPPER DUMP
      case 233:  // 1024K
        // BMC 22-in-1/20-in-1 (42-in-1)
        // POWER CYCLE TO SWITCH BETWEEN CHIPS
        banks = OSCR::Util::power<2>(NES_PRG) / 2;  // 512K/512K

        // Check PRG Chip
        write_prg_byte(0x8000, 0);

        if (read_prg_byte(0x8001) == 0x00) // PRG CHIP 1
        {
            OSCR::UI::printLineSync(F("READING LOWER CHIP"));
        }
        else // PRG CHIP 2 - Should be 0xEF
        {
            OSCR::UI::printLineSync(F("READING UPPER CHIP"));
        }

        for (size_t i = 0; i < banks; i += 2)
        {
          write_prg_byte(0x8000, i & 0x1F);
          dumpBankPRG(0x0, 0x8000, base);
        }

        OSCR::UI::printLineSync(F("POWER CYCLE TO SWITCH"));

        break;

      case 235:
        for (size_t i = 0; i < 32; i++)
        {
          write_prg_byte(0x8000 + i, 0);
          dumpBankPRG(0x0, 0x8000, base);
        }

        if (NES_PRG > 6)
        {
          for (size_t i = 32; i < 64; i++)
          {
            write_prg_byte(0x80E0 + i, 0);
            dumpBankPRG(0x0, 0x8000, base);
          }

          if (NES_PRG > 7)
          {
            for (size_t i = 64; i < 96; i++)
            {
              write_prg_byte(0x81E0 + i, 0);
              dumpBankPRG(0x0, 0x8000, base);
            }

            for (size_t i = 96; i < 128; i++)
            {
              write_prg_byte(0x82E0 + i, 0);
              dumpBankPRG(0x0, 0x8000, base);
            }
          }
        }
        break;

      case 236:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000 | ((i & 0x38) >> 3), 0);  // A19-A17
          write_prg_byte(0xC030 | (i & 0x0F), 0);         // A17-A14
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 237:  // 1024K
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_pulsem2(0x8000, i | 0xC0);  // 32K NROM Mode
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 240:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x5FFF, (i & 0xF) << 4);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 242: // total size is 640k THIS IS NORMAL
        for (size_t i = 0; i < 32; i++) // dump 1st chip of 512k
        {
          write_prg_byte(0x8400 + (i * 4), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }

        for (size_t i = 0; i < 8; i++) // dump 2nd chip of 128k
        {
          write_prg_byte(0x8000 + (i * 4), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 246:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i += 4)
        {
          write_prg_byte(0x6000, (i | 0));
          write_prg_byte(0x6001, (i | 1));
          write_prg_byte(0x6002, (i | 2));
          write_prg_byte(0x6003, (i | 3));
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 252:
      case 253:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xA010, i);
          dumpBankPRG(0x2000, 0x4000, base);
        }
        break;

      case 261:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xF000 + ((i & 0x0E) << 6) + ((i & 0x01) << 5), i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 286:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xA0F0 + i, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 288:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000 + ((i << 3) & 0x18), i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 289:  // 512K/1024K/2048K
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          for (uint16_t address = 0; address < 0x4000; address += 512) // 16K
          {
            write_prg_pulsem2(0x6000, 0); // NROM-128 Mode
            write_prg_pulsem2(0x6001, i); // Set Bank
            dumpPRG_pulsem2(base, address);
          }
        }
        break;

      case 290:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x8000 | ((i << 10) & 0x7800) | ((i << 6) & 0x40), i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 312:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x6000, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 314: // 1024K/2048K
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_pulsem2(0x5000, 0x80); // NROM
          write_prg_pulsem2(0x5001, ((i & 0x3F) | 0x80)); // prg 32k bank (M==1 (NROM-256))
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 319:  // 128K
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x6004, (i << 3) | 0x40);  // PRG A14 = CPU A14 (NROM-256)
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 331:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xE000, i >> 3);
          write_prg_byte(0xA000, i);
          write_prg_byte(0xC000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 332:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          for (uint16_t address = 0x0; address < 0x4000; address += 512)
          {
            write_prg_pulsem2(0x6000, 0x08 | (i & 0x07) | ((i & 0x08) << 3));
            dumpPRG_pulsem2(base, address);
          }
        }
        break;

      case 380:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xF201 + ((i & 0x1F) << 2), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 396:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xA000, (i >> 3) & 0x07);
          write_prg_byte(0x8000, i & 0x07);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 399:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xE001, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 446:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        write_prg_byte(0x5003, 0);
        write_prg_byte(0x5005, 0);

        for (uint8_t i = 0; i < banks; i++) // 8192 for 64MiB
        {
          write_prg_byte(0x5002, i >> 8); // outer bank LSB
          write_prg_byte(0x5001, i); // outer bank MSB
          write_prg_byte(0x8000, 0);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 470:
        banks = OSCR::Util::power<2>(NES_PRG) / 2;

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0x5000, i >> 3);
          write_prg_byte(0x8000, i & 0x07);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 519:
        banks = OSCR::Util::power<2>(NES_PRG);

        for (size_t i = 0; i < banks; i += 2)
        {
          write_prg_byte(0x8000 + (i & 0x3F), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 552:
        banks = OSCR::Util::power<2>(NES_PRG) * 2;

        for (size_t i = 0; i < banks; i++)
        {
          // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x7EFA, ((i & 0x01) << 5) | ((i & 0x02) << 3) | ((i & 0x04) << 1) | ((i & 0x08) >> 1) | ((i & 0x10) >> 3) | ((i & 0x20) >> 5));

          // 8K Banks ($8000-$BFFF)
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;
    }

    if (!readrom)
    {
      OSCR::Storage::Shared::close();

      cartOff();
    }

    set_address(0);
    NES_PHI2_HI;
    NES_ROMSEL_HI;
  }

  void readCHR(bool readrom)
  {
    uint16_t banks;
    bool busConflict = false;

    if (!readrom)
    {
      printHeader();

      OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::NES), FS(OSCR::Strings::Directory::ROM), "CHR", FS(OSCR::Strings::FileType::Raw));

      cartOn();
    }

    set_address(0);
    _delay_us(1);

    switch (NES_MAPPER)
    {
    case 0:  // 8K
    case 43:
    case 55:
      dumpBankCHR(0x0, 0x2000);
      break;

    case 1:
    case 155:
      banks = OSCR::Util::power<2>(NES_CHR);
      for (size_t i = 0; i < banks; i += 2) {  // 8K/16K/32K/64K/128K (Bank #s are based on 4K Banks)
        write_prg_byte(0x8000, 0x80);          // Clear Register
        write_mmc1_byte(0xA000, i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 3:  // 8K/16K/32K - bus conflicts
    case 29:
    case 66:  // 16K/32K
    case 70:
    case 148:  // Sachen SA-008-A and Tengen 800008 - Bus conflicts
    case 152:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      busConflict = true;
      for (size_t i = 0; i < banks; i++) {
        for (size_t x = 0; x < 0x8000; x++) {
          if (read_prg_byte(0x8000 + x) == i) {
            write_prg_byte(0x8000 + x, i);
            busConflict = false;
            break;
          }
        }
        if (busConflict) {
          write_prg_byte(0x8000 + i, i);
        }
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 4:
    case 12:
    case 37:
    case 44:
    case 47:
    case 49:
    case 52:
    case 64:
    case 74:
    case 95:  // 32K
    case 115:
    case 116:
    case 118:
    case 119:
    case 126:
    case 134:
    case 158:
    case 176:
    case 189:
    case 195:
    case 196:
    case 206:  // 16K/32K/64K
    case 224:
    case 248:
    case 268:
    case 315:
    case 351:
    case 366:
    case 422:
    case 534:
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      write_prg_byte(0xA001, 0x80);
      if ((NES_MAPPER == 126) || (NES_MAPPER == 422) || (NES_MAPPER == 534)) {
        write_prg_byte(0x6803, 0);  // set MMC3 banking mode
      }
      if ((NES_MAPPER == 115) || (NES_MAPPER == 134) || (NES_MAPPER == 248)) {
        write_prg_byte(0x6000, 0);  // set MMC3 banking mode
      }
      if (NES_MAPPER == 176) {
        write_prg_byte(0x5FF3, 0);  // extended MMC3 mode: disabled
        write_prg_byte(0x5FF0, 1);  // 256K outer bank mode
      }
      if (NES_MAPPER == 351) {
        write_prg_byte(0x5000, 0);
        write_prg_byte(0x5001, 0);
        write_prg_byte(0x5002, 0);
      }
      for (size_t i = 0; i < banks; i++) {
        if (NES_MAPPER == 12) {
          write_prg_byte(0x4132, (i & 0x100) >> 8 | (i & 0x100) >> 4);
        }
        if (NES_MAPPER == 37) {
          if (i == 0) {
            write_prg_byte(0x6000, 0);
          } else if (i == 64) {
            write_prg_byte(0x6000, 3);
          } else if (i == 128) {
            write_prg_byte(0x6000, 4);
          }
        }
        if (NES_MAPPER == 44) {
          write_prg_byte(0xA001, 0x80 | ((i >> 7) & 0x07));
        }
        if (NES_MAPPER == 47) {
          write_prg_byte(0x6800 + ((i & 0x180) >> 7), 0);
        }
        if (NES_MAPPER == 49) {
          write_prg_byte(0x6800, ((i & 0x180) >> 1) | 1);
        }
        if (NES_MAPPER == 52) {
          write_prg_byte(0x6000, (i & 0x180) >> 3 | (i & 0x200) >> 7);
        }
        if ((NES_MAPPER == 115) || (NES_MAPPER == 248)) {
          write_prg_byte(0x6001, (i & 0x100) >> 8);  // A18
        }
        if (NES_MAPPER == 116) {
          write_prg_byte(0x4100, 0x01 | ((i & 0x100) >> 6));  // A18
        }
        if (NES_MAPPER == 126) {
          write_prg_byte(0x6800, (i & 0x200) >> 5 | (i & 0x100) >> 3);  // select outer bank
        }
        if (NES_MAPPER == 134) {
          write_prg_byte(0x6000, (i & 0x200) >> 4);  // A19
          write_prg_byte(0x6001, (i & 0x180) >> 3);  // A18-17
        }
        if (NES_MAPPER == 176) {
          write_prg_byte(0x5FF2, (i & 0x700) >> 3);  // outer 256k bank
        }
        if ((NES_MAPPER == 224) || (NES_MAPPER == 268) || (NES_MAPPER == 995) || (NES_MAPPER == 996) || (NES_MAPPER == 997)) {
          write_prg_byte(0x5000, ((i & 0x380) >> 4) | ((i & 0xC00) >> 9));
          write_prg_byte(0x6000, ((i & 0x380) >> 4) | ((i & 0xC00) >> 9));
        }
        if (NES_MAPPER == 315) {
          write_prg_byte(0x6800, ((i & 0x100) >> 8) | ((i & 0x80) >> 6) | ((i & 0x40) >> 3));
        }
        if (NES_MAPPER == 351) {
          write_prg_byte(0x5000, (i >> 1) & 0xFC);
        }
        if (NES_MAPPER == 366) {
          write_prg_byte(0x6800 + ((i & 0x380) >> 3), i);
        }
        if ((NES_MAPPER == 422) || (NES_MAPPER == 534)) {
          write_prg_byte(0x6800, (i & 0x380) >> 4);
        }
        if ((NES_MAPPER == 998) || (NES_MAPPER == 999)) {
          write_prg_byte(0x5000, (i & 0x80) >> 4);
          write_prg_byte(0x6000, (i & 0x80) >> 4);
        }
        write_prg_byte(0x8000, 0x02);
        write_prg_byte(0x8001, i);
        dumpBankCHR(0x1000, 0x1400);
      }
      break;

    case 5:  // 128K/256K/512K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      write_prg_byte(0x5101, 0);  // 8K CHR Banks
      for (size_t i = 0; i < banks; i++) {
        if (i == 0)
          write_prg_byte(0x5130, 0);  // Set Upper 2 bits
        else if (i == 8)
          write_prg_byte(0x5130, 1);  // Set Upper 2 bits
        else if (i == 16)
          write_prg_byte(0x5130, 2);  // Set Upper 2 bits
        else if (i == 24)
          write_prg_byte(0x5130, 3);  // Set Upper 2 bits
        write_prg_byte(0x5127, i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 9:
    case 10:
      banks = OSCR::Util::power<2>(NES_CHR);
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xB000, i);
        write_prg_byte(0xC000, i);
        dumpBankCHR(0x0, 0x1000);
      }
      break;

    case 11:
    case 144:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xFFB0 + i, i << 4);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 16:
    case 159:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x6000, i);  // Submapper 4
        write_prg_byte(0x8000, i);  // Submapper 5
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 18:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xA000, i & 0xF);         // CHR Bank Lower 4 bits
        write_prg_byte(0xA001, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 19:  // 128K/256K
    case 532:
      for (size_t j = 0; j < 64; j++) {  // Init Register
        write_ram_byte(0xE800, 0xC0);    // CHR RAM High/Low Disable (ROM Enable)
      }
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      write_ram_byte(0xE800, 0xC0);  // CHR RAM High/Low Disable (ROM Enable)
      for (size_t i = 0; i < banks; i += 8) {
        write_prg_byte(0x8000, i);      // CHR Bank 0
        write_prg_byte(0x8800, i + 1);  // CHR Bank 1
        write_prg_byte(0x9000, i + 2);  // CHR Bank 2
        write_prg_byte(0x9800, i + 3);  // CHR Bank 3
        write_prg_byte(0xA000, i + 4);  // CHR Bank 4
        write_prg_byte(0xA800, i + 5);  // CHR Bank 5
        write_prg_byte(0xB000, i + 6);  // CHR Bank 6
        write_prg_byte(0xB800, i + 7);  // CHR Bank 7
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 21:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xB000, i & 0xF);           // CHR Bank Lower 4 bits
        if (NES_CHR == 5)                          // Check CHR Size to determine VRC4a (128K) or VRC4c (256K)
          write_prg_byte(0xB002, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4a (Wai Wai World 2)
        else                                       // banks == 256
          write_prg_byte(0xB040, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4c (Ganbare Goemon Gaiden 2)
        dumpBankCHR(0x0, 0x400);
      }
      break;


    case 22:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xB000, (i << 1) & 0xF);  // CHR Bank Lower 4 bits
        write_prg_byte(0xB002, (i >> 3) & 0xF);  // CHR Bank Upper 4 bits
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 23:
    case 272:
      {
        banks = OSCR::Util::power<2>(NES_CHR) * 4;

        // Detect VRC4e Carts - read PRG 0x1FFF6 (DATE)
        // Boku Dracula-kun = 890810, Tiny Toon = 910809, Crisis Force = 910701, Parodius Da! = 900916
        write_prg_byte(0x8000, 15);  // Load last bank

        uint16_t m23reg = 0xB001;

        if ((read_prg_byte(0x9FF6) == 0x30) && !(NES_MAPPER == 272))
        {
          OSCR::UI::printLineSync(F("VRC4e detected!"));

          m23reg = 0xB004;
        }

        for (size_t i = 0; i < banks; i++)
        {
          write_prg_byte(0xB000, i & 0xF);          // CHR Bank 0: Lower 4 bits
          write_prg_byte(m23reg, (i >> 4) & 0x1F);  // CHR Bank 0: Upper 5 bits
          dumpBankCHR(0x0, 0x400);
        }

        break;
      }

    case 24:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      write_prg_byte(0xB003, 0);  // PPU Banking Mode 0
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xD000, i);  // CHR Bank 0
        dumpBankCHR(0x0, 0x400);    // 1K Banks
      }
      break;

    case 25:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xB000, i & 0xF);         // CHR Bank Lower 4 bits
        write_prg_byte(0xB00A, (i >> 4) & 0xF);  // Combine VRC2c and VRC4b, VRC4d reg
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 26:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      write_prg_byte(0xB003, 0);
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xD000, i);
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 27:
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xB000, i & 0x0F);
        write_prg_byte(0xB001, i >> 4);
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 32:  // 128K
    case 65:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xB000, i);
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 33:  // 128K/256K
    case 48:  // 256K
      banks = OSCR::Util::power<2>(NES_CHR) * 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8002, i);
        dumpBankCHR(0x0, 0x800);
      }
      break;

    case 34:  // NINA
      banks = OSCR::Util::power<2>(NES_CHR);
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x7FFE, i);  // Select 4 KB CHR bank at $0000
        delay(200);                 // NINA seems slow to switch banks
        dumpBankCHR(0x0, 0x1000);
      }
      break;

    case 35:
    case 90:
    case 209:
    case 211:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      write_prg_byte(0xD000, 0x02);
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xD003, ((i >> 3) & 0x18) | 0x20);
        write_prg_byte(0x9000, (i & 0x3F));
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 36:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x4200, i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 38:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x7000, i << 2);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 41:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x6004 + ((i & 0x0C) << 1), 0);
        write_prg_byte(0x6004 + ((i & 0x0C) << 1), 0);
        write_prg_byte(0xFFF0 + (i & 0x03), i & 0x03);
        write_prg_byte(0xFFF0 + (i & 0x03), i & 0x03);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 42:
      banks = OSCR::Util::power<2>(NES_CHR);
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, i & 0x0F);
        dumpBankCHR(0x0, 0x1000);
      }
      break;

    case 45:  // 128K/256K/512K/1024K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      write_prg_byte(0xA001, 0x80);  // Unlock Write Protection - not used by some carts
      for (size_t i = 0; i < banks; i++) {
        // set outer bank registers
        write_prg_byte(0x6000, 0);                          // CHR-OR
        write_prg_byte(0x6000, 0);                          // PRG-OR
        write_prg_byte(0x6000, (((i / 256) << 4) | 0x0F));  // CHR-AND,CHR-OR/PRG-OR
        write_prg_byte(0x6000, 0x80);                       // PRG-AND
        // set inner bank registers
        write_prg_byte(0x8000, 0x2);  // CHR Bank 2 ($1000-$13FF)
        write_prg_byte(0x8001, i);
        for (size_t address = 0x1000; address < 0x1200; address += 512) {
          dumpCHR_M2(address);  // Read CHR with M2 Pulse
        }
        // set outer bank registers
        write_prg_byte(0x6000, 0);                          // CHR-OR
        write_prg_byte(0x6000, 0);                          // PRG-OR
        write_prg_byte(0x6000, (((i / 256) << 4) | 0x0F));  // CHR-AND,CHR-OR/PRG-OR
        write_prg_byte(0x6000, 0x80);                       // PRG-AND
        // set inner bank registers
        write_prg_byte(0x8000, 0x2);  // CHR Bank 2 ($1000-$13FF)
        write_prg_byte(0x8001, i);
        for (size_t address = 0x1200; address < 0x1400; address += 512) {
          dumpCHR_M2(address);  // Read CHR with M2 Pulse
        }
      }
      break;

    case 46:
      banks = OSCR::Util::power<2>(NES_CHR);  // 8k banks
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x6000, (i & 0x78) << 1);  // high bits
        write_prg_byte(0x8000, (i & 0x07) << 4);  // low bits
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 56:
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xFC00, i);
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 57:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        for (uint16_t address = 0x0; address < 0x2000; address += 512)
        {
          write_prg_pulsem2(0x8800, i & 0x07);                  // A15-A13
          write_prg_pulsem2(0x8000, 0x80 | ((i & 0x08) << 3));  // A16
          dumpCHR_pulsem2(address);
        }
      }
      break;

    case 58:
    case 213:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + ((i & 0x07) << 3), 0);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 59:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_reg_m59(0x8000 + (i & 0x07));
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 60:
      for (size_t i = 0; i < 4; i++) {
        write_prg_byte(0x8D8D, i);
        delay(500);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 61:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 | (i << 8), 0);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 62:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + (i / 4), i & 3);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 67:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) * 2;
      for (size_t i = 0; i < banks; i++) {  // 2K Banks
        write_prg_byte(0x8800, i);          // CHR Bank 0
        dumpBankCHR(0x0, 0x800);
      }
      break;

    case 68:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 2;
      for (size_t i = 0; i < banks; i += 4) {  // 2K Banks
        write_prg_byte(0x8000, i);             // CHR Bank 0
        write_prg_byte(0x9000, i + 1);         // CHR Bank 1
        write_prg_byte(0xA000, i + 2);         // CHR Bank 2
        write_prg_byte(0xB000, i + 3);         // CHR Bank 3
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 69:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, 0);  // Command Register - CHR Bank 0
        write_prg_byte(0xA000, i);  // Parameter Register - ($0000-$03FF)
        dumpBankCHR(0x0, 0x400);    // 1K Banks
      }
      break;

    case 72:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      write_prg_byte(0x8000, 0);            // Reset Register
      for (size_t i = 0; i < banks; i++) {  // 8K Banks
        write_prg_byte(0x8000, i | 0x40);   // CHR Command + Bank
        write_prg_byte(0x8000, i);          // CHR Bank
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 75:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR);
      for (size_t i = 0; i < banks; i++) {        // 4K Banks
        write_reg_byte(0xE000, i);                // CHR Bank Low Bits [WRITE RAM SAFE]
        write_prg_byte(0x9000, (i & 0x10) >> 3);  // High Bit
        dumpBankCHR(0x0, 0x1000);
      }
      break;

    case 76:
      banks = OSCR::Util::power<2>(NES_CHR) * 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, 2);
        write_prg_byte(0x8001, i);
        dumpBankCHR(0x0, 0x800);
      }
      break;

    case 77:  // 32K
      banks = OSCR::Util::power<2>(NES_CHR) * 2;
      for (size_t i = 0; i < banks; i++) {  // 2K Banks
        write_prg_byte(0x8000, i << 4);     // CHR Bank 0
        dumpBankCHR(0x0, 0x800);
      }
      break;

    case 78:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {  // 8K Banks
        write_prg_byte(0x8000, i << 4);     // CHR Bank 0
        dumpBankCHR(0x0, 0x2000);           // 8K Banks ($0000-$1FFF)
      }
      break;

    case 79:
    case 146:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x4100, i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 80:   // 128K/256K
    case 82:   // 128K/256K
    case 207:  // 128K [CART SOMETIMES NEEDS POWERCYCLE]
    case 552:
      banks = OSCR::Util::power<2>(NES_CHR) * 2;
      write_prg_byte(0x7EF6, 0);  // CHR mode [2x 2KiB banks at $0000-$0FFF]
      for (size_t i = 0; i < banks; i += 2) {
        write_prg_byte(0x7EF0, i << 1);
        write_prg_byte(0x7EF1, (i + 1) << 1);
        dumpBankCHR(0x0, 0x1000);
      }
      break;

    case 85:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i += 8) {
        write_prg_byte(0xA000, i);      // CHR Bank 0
        write_prg_byte(0xA008, i + 1);  // CHR Bank 1
        write_prg_byte(0xB000, i + 2);  // CHR Bank 2
        write_prg_byte(0xB008, i + 3);  // CHR Bank 3
        write_prg_byte(0xC000, i + 4);  // CHR Bank 4
        write_prg_byte(0xC008, i + 5);  // CHR Bank 5
        write_prg_byte(0xD000, i + 6);  // CHR Bank 6
        write_prg_byte(0xD008, i + 7);  // CHR Bank 7
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 86:  // 64K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {  // 8K Banks
        if (i < 4)
          write_prg_byte(0x6000, i & 0x3);
        else
          write_prg_byte(0x6000, (i | 0x40) & 0x43);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 87:  // 16K/32K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {  // 16K/32K
        write_prg_byte(0x6000, ((i & 0x1) << 1) | ((i & 0x2) >> 1));
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 88:
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      write_prg_byte(0xA001, 0x80);
      for (size_t i = 0; i < banks; i += 2) {
        if (i < 64) {
          write_prg_byte(0x8000, 0);
          write_prg_byte(0x8001, i);
          dumpBankCHR(0x0, 0x800);
        } else {
          write_prg_byte(0x8000, 2);
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 3);
          write_prg_byte(0x8001, i + 1);
          dumpBankCHR(0x1000, 0x1800);
        }
      }
      break;

    case 89:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {  // 8K Banks
        if (i < 8)
          write_prg_byte(0x8000, i & 0x7);
        else
          write_prg_byte(0x8000, (i | 0x80) & 0x87);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 91:
      banks = OSCR::Util::power<2>(NES_CHR) * 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + ((i & 0x100) >> 8), i);  // CHR A19 (submapper 0 only)
        write_prg_byte(0x6000, i);                       // CHR A18-A11
        dumpBankCHR(0x0, 0x800);
      }
      break;

    case 92:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      write_prg_byte(0x8000, 0);            // Reset Register
      for (size_t i = 0; i < banks; i++) {  // 8K Banks
        write_prg_byte(0x8000, i | 0x40);   // CHR Command + Bank
        write_prg_byte(0x8000, i);          // CHR Bank
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 107:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xC000, i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 112:
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, 6);
        write_prg_byte(0xA000, i);
        dumpBankCHR(0x1000, 0x1400);
      }
      break;

    case 113:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x4100, (i & 0x08) << 3 | (i & 0x07));
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 114:  // Submapper 0
    case 182:
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x6001, (i & 0x80) >> 7);
        write_prg_byte(0xA000, 6);
        write_prg_byte(0xC000, i);
        dumpBankCHR(0x1000, 0x1400);
      }
      break;

    case 117:
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xA000, i);
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 122:
    case 184:  // 16K/32K
      banks = OSCR::Util::power<2>(NES_CHR);
      for (size_t i = 0; i < banks; i++) {  // 4K Banks
        write_prg_byte(0x6000, i);          // CHR LOW (Bits 0-2) ($0000-$0FFF)
        dumpBankCHR(0x0, 0x1000);           // 4K Banks ($0000-$0FFF)
      }
      break;

    case 140:  // 32K/128K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {  // 8K Banks
        write_prg_byte(0x6000, i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 154:  // 128K
      for (size_t i = 0; i < 64; i += 2) {
        write_prg_byte(0x8000, 0);
        write_prg_byte(0x8001, i);
        dumpBankCHR(0x0, 0x800);
      }
      for (size_t i = 0; i < 64; i++) {
        write_prg_byte(0x8000, 2);
        write_prg_byte(0x8001, i);
        dumpBankCHR(0x1000, 0x1400);
      }
      break;

    case 165:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR);
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, 0x02);
        write_prg_byte(0x8001, i);
        dumpBankCHR(0x1000, 0x2000);
      }
      break;

    case 174:  // 64K
      for (size_t i = 0; i < 8; i++) {
        write_prg_byte(0xFF00 + (i << 1), 0);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 175:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xA000, i << 2);
        write_prg_byte(0x8000, i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 185:                           // 8K [READ 32K TO OVERRIDE LOCKOUT]
      for (size_t i = 0; i < 4; i++)    // Read 32K to locate valid 8K
      {
        write_prg_byte(0x8000, i);

        uint8_t chrcheck = read_chr_byte(0);

        for (size_t address = 0x0; address < 0x2000; address += 512)
        {
          for (size_t x = 0; x < 512; x++)
          {
            OSCR::Storage::Shared::buffer[x] = read_chr_byte(address + x);
          }

          if (chrcheck != 0xFF)
          {
            OSCR::Storage::Shared::writeBuffer();

            OSCR::UI::ProgressBar::advance(512);
          }
        }
      }
      break;

    case 200:
    case 212:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + (i & 0x07), 0);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 201:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + (i & 0xFF), 0);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 202:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 | (i << 1), 0);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 203:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, (i & 0x03));
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 210:  // 128K/256K
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      write_prg_byte(0xE800, 0xC0);  // CHR RAM DISABLE (Bit 6 and 7) [WRITE NO RAM]
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, i);  // CHR Bank 0
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 214:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 | (i << 2), 0);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 225:
    case 255:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + ((i & 0x40) << 8) + (i & 0x3F), i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 228:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      write_prg_byte(0x8000, 0);
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + ((i & 0x3C) >> 2), i & 0x03);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 229:
      for (size_t i = 0; i < 32; i++) {
        write_prg_byte(0x8000 + i, i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 236:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 | (i & 0x0F), 0);  // A16-A13
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 240:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x5FFF, (i & 0xF));
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 246:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i += 4) {
        write_prg_byte(0x6004, (i | 0));
        write_prg_byte(0x6005, (i | 1));
        write_prg_byte(0x6006, (i | 2));
        write_prg_byte(0x6007, (i | 3));
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 252:
    case 253:
      banks = OSCR::Util::power<2>(NES_CHR) * 4;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xB000, i & 0x0F);
        write_prg_byte(0xB004, ((i >> 4) & 0x0F) | ((i >> 8) << 4));
        dumpBankCHR(0x0, 0x400);
      }
      break;

    case 261:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xF000 + (i & 0x0F), i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 286:
      banks = OSCR::Util::power<2>(NES_CHR) * 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + i, i);
        dumpBankCHR(0x0, 0x800);
      }
      break;

    case 288:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 + (i & 0x07), i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 290:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000 | ((i << 5) & 0x300) | (i & 0x07), i);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 314: // 512K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_pulsem2(0x5000, ((i & 0x3) << 1)); // chr 8k bank (bits 0-1)
        write_prg_pulsem2(0x5002, i >> 2); // chr 8k bank (bits 2-5)
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 319:  // 64K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x6000, i << 4);
        dumpBankCHR(0x0, 0x2000);
      }
      break;

    case 331:
      banks = OSCR::Util::power<2>(NES_CHR);
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0xE000, i >> 3);
        write_prg_byte(0xA000, i << 3);
        dumpBankCHR(0x0, 0x1000);
      }
      break;

    case 332:  // 128K
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        for (uint16_t address = 0x0; address < 0x2000; address += 512)
        {
          write_prg_pulsem2(0x6001, 0x30 | (i & 0x07));
          write_prg_pulsem2(0x6000, (i & 0x08) << 3);
          dumpCHR_pulsem2(address);
        }
      }
      break;

    case 519:
      banks = OSCR::Util::power<2>(NES_CHR) / 2;
      for (size_t i = 0; i < banks; i++) {
        write_prg_byte(0x8000, i & 0x7F);
        dumpBankCHR(0x0, 0x2000);
      }
      break;
    }

    if (!readrom)
    {
      cartOff();

      OSCR::Storage::Shared::close();
    }

    set_address(0);
    NES_PHI2_HI;
    NES_ROMSEL_HI;
  }

  /******************************************
     RAM Functions
  *****************************************/
  void readRAM()
  {
    uint16_t banks;

    set_address(0);
    _delay_us(1);
    if (NES_RAM == 0)
    {
        OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
    }
    else
    {
      CreateRAMFileInSD();
      uint16_t base = 0x6000;

      if (OSCR::Storage::Shared::sharedFile.isOpen())
      {
        switch (NES_MAPPER)
        {
          case 0:                                       // 2K/4K
            dumpBankPRG(0x0, (0x800 * NES_RAM), base);  // 2K/4K
            break;                                      // SWITCH MUST BE IN OFF POSITION

          case 1:
          case 155:                               // 8K/16K/32K
            banks = OSCR::Util::power<2>(NES_RAM) / 2;      // banks = 1,2,4
            for (size_t i = 0; i < banks; i++) {  // 8K Banks ($6000-$7FFF)
              write_prg_byte(0x8000, 0x80);       // Clear Register
              write_mmc1_byte(0x8000, 1 << 3);
              write_mmc1_byte(0xE000, 0);
              if (banks == 4)  // 32K
                write_mmc1_byte(0xA000, i << 2);
              else
                write_mmc1_byte(0xA000, i << 3);
              dumpBankPRG(0x0, 0x2000, base);  // 8K
            }
            break;

          case 4:                                                                // 1K/8K (MMC6/MMC3)
            if (mmc6) {                                                          // MMC6 1K
              write_prg_byte(0x8000, 0x20);                                      // PRG RAM ENABLE
              write_prg_byte(0xA001, 0x20);                                      // PRG RAM PROTECT - Enable reading RAM at $7000-$71FF
              for (size_t address = 0x1000; address < 0x1200; address += 512) {  // 512B
                dumpMMC5RAM(base, address);
              }
              write_prg_byte(0x8000, 0x20);                                      // PRG RAM ENABLE
              write_prg_byte(0xA001, 0x80);                                      // PRG RAM PROTECT - Enable reading RAM at $7200-$73FF
              for (size_t address = 0x1200; address < 0x1400; address += 512) {  // 512B
                dumpMMC5RAM(base, address);
              }
              write_prg_byte(0x8000, 6);       // PRG RAM DISABLE
            } else {                           // MMC3 8K
              write_prg_byte(0xA001, 0xC0);    // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
              dumpBankPRG(0x0, 0x2000, base);  // 8K
            }
            break;

          case 5:                                         // 8K/16K/32K
            write_prg_byte(0x5100, 3);                    // 8K PRG Banks
            banks = OSCR::Util::power<2>(NES_RAM) / 2;              // banks = 1,2,4
            if (banks == 2) {                             // 16K - Split SRAM Chips 8K/8K
              for (size_t i = 0; i < (banks / 2); i++) {  // Chip 1
                write_prg_byte(0x5113, i);
                for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                  dumpMMC5RAM(base, address);
                }
              }
              for (size_t j = 4; j < (banks / 2) + 4; j++) {  // Chip 2
                write_prg_byte(0x5113, j);
                for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                  dumpMMC5RAM(base, address);
                }
              }
            } else {                                // 8K/32K Single SRAM Chip
              for (size_t i = 0; i < banks; i++) {  // banks = 1 or 4
                write_prg_byte(0x5113, i);
                for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                  dumpMMC5RAM(base, address);
                }
              }
            }
            break;

          case 16:  // 256-byte EEPROM 24C02
          case 159:
            { // 128-byte EEPROM 24C01 [Little Endian]
              size_t eepsize;

              if (NES_MAPPER == 159)
              {
                eepsize = 128;
              }
              else
              {
                eepsize = 256;
              }

              for (size_t address = 0; address < eepsize; address++)
              {
                eepromRead(address);
              }

              OSCR::Storage::Shared::writeBuffer(eepsize);
              break;
            }
          case 19:
            if (NES_RAM == 2) // PRG RAM 128B
            {
              for (size_t x = 0; x < 128; x++)
              {
                write_ram_byte(0xF800, x);            // PRG RAM ENABLE
                OSCR::Storage::Shared::buffer[x] = read_prg_byte(0x4800);  // DATA PORT
              }

              OSCR::Storage::Shared::writeBuffer(128);
            }
            else // SRAM 8K
            {
              for (size_t i = 0; i < 64; i++) // Init Register
              {
                write_ram_byte(0xE000, 0);
              }

              dumpBankPRG(0x0, 0x2000, base);  // 8K
            }
            break;

          case 80:                              // 1K
            write_prg_byte(0x7EF8, 0xA3);       // PRG RAM ENABLE 0
            write_prg_byte(0x7EF9, 0xA3);       // PRG RAM ENABLE 1

            for (size_t x = 0; x < 128; x++)    // PRG RAM 1K ($7F00-$7FFF) MIRRORED ONCE
            {
              OSCR::Storage::Shared::buffer[x] = read_prg_byte(0x7F00 + x);
            }

            OSCR::Storage::Shared::writeBuffer(128);

            write_prg_byte(0x7EF8, 0xFF);  // PRG RAM DISABLE 0
            write_prg_byte(0x7EF9, 0xFF);  // PRG RAM DISABLE 1
            break;

          case 82:                                                          // 5K
            write_prg_byte(0x7EF7, 0xCA);                                   // PRG RAM ENABLE 0 ($6000-$67FF)
            write_prg_byte(0x7EF8, 0x69);                                   // PRG RAM ENABLE 1 ($6800-$6FFF)
            write_prg_byte(0x7EF9, 0x84);                                   // PRG RAM ENABLE 2 ($7000-$73FF)

            for (size_t address = 0x0; address < 0x1400; address += 512)    // PRG RAM 5K ($6000-$73FF)
            {
              dumpMMC5RAM(base, address);
            }

            write_prg_byte(0x7EF7, 0xFF);  // PRG RAM DISABLE 0 ($6000-$67FF)
            write_prg_byte(0x7EF8, 0xFF);  // PRG RAM DISABLE 1 ($6800-$6FFF)
            write_prg_byte(0x7EF9, 0xFF);  // PRG RAM DISABLE 2 ($7000-$73FF)
            break;

          default:
            if (NES_MAPPER == 118)               // 8K
              write_prg_byte(0xA001, 0xC0);  // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
            else if (NES_MAPPER == 19) {
              for (size_t i = 0; i < 64; i++) {  // Init Register
                write_ram_byte(0xE000, 0);
              }
            } else if ((NES_MAPPER == 21) || (NES_MAPPER == 25))  // 8K
              write_prg_byte(0x8000, 0);
            else if (NES_MAPPER == 26)           // 8K
              write_prg_byte(0xB003, 0x80);  // PRG RAM ENABLE
            else if (NES_MAPPER == 68)           // 8K
              write_reg_byte(0xF000, 0x10);  // PRG RAM ENABLE [WRITE RAM SAFE]
            else if (NES_MAPPER == 69) {         // 8K
              write_prg_byte(0x8000, 8);     // Command Register - PRG Bank 0
              write_prg_byte(0xA000, 0xC0);  // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
            } else if (NES_MAPPER == 85)         // 8K
              write_ram_byte(0xE000, 0x80);  // PRG RAM ENABLE
            else if (NES_MAPPER == 153)          // 8K
              write_prg_byte(0x800D, 0x20);  // PRG RAM Chip Enable
            dumpBankPRG(0x0, 0x2000, base);  // 8K
            if (NES_MAPPER == 85)                // 8K
              write_reg_byte(0xE000, 0);     // PRG RAM DISABLE [WRITE RAM SAFE]
            break;
        }

        OSCR::Storage::Shared::close();

        OSCR::UI::printLabel(OSCR::Strings::Common::CRCSum);
        OSCR::UI::printLineSync(OSCR::CRC32::current);
      }
    }
    set_address(0);
    NES_PHI2_HI;
    NES_ROMSEL_HI;
  }

  void writeBankPRG(size_t const from, size_t const to, size_t const base)
  {
    for (size_t address = from; address < to; address += 512)
    {
      OSCR::Storage::Shared::fill();

      for (size_t x = 0; x < 512; x++)
      {
        write_prg_byte(base + address + x, OSCR::Storage::Shared::buffer[x]);
      }
    }
  }

  void writeBankWRAM(size_t const from, size_t const to, size_t const base)
  {
    for (size_t address = from; address < to; address += 512)
    {
      OSCR::Storage::Shared::fill();

      for (size_t x = 0; x < 512; x++)
      {
        write_wram_byte(base + address + x, OSCR::Storage::Shared::buffer[x]);
      }
    }
  }

  void writeRAM()
  {
    printHeader();

    if (NES_RAM == 0)
    {
      OSCR::UI::error(FS(OSCR::Strings::Errors::NotSupportedByCart));
      return;
    }

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    uint16_t base = 0x6000;
    uint16_t banks;
    size_t eepsize;

    printHeader();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    switch (NES_MAPPER)
    {
    case 0: // 2K/4K
      writeBankPRG(0x0, (0x800 * NES_RAM), base);
      break; // SWITCH MUST BE IN OFF POSITION

    case 1:
    case 155:
      banks = OSCR::Util::power<2>(NES_RAM) / 2;    // banks = 1,2,4

      for (size_t i = 0; i < banks; i++)      // 8K Banks ($6000-$7FFF)
      {
        write_prg_byte(0x8000, 0x80);         // Clear Register
        write_mmc1_byte(0x8000, 1 << 3);      // PRG ROM MODE 32K
        write_mmc1_byte(0xE000, 0);           // PRG RAM ENABLED

        if (banks == 4)                       // 32K
        {
          write_mmc1_byte(0xA000, i << 2);
        }
        else
        {
          write_mmc1_byte(0xA000, i << 3);
        }

        writeBankPRG(0x0, 0x2000, base);    // 8K
      }
      break;

    case 4:                                   // 1K/8K (MMC6/MMC3)
      if (mmc6)                               // MMC6 1K
      {
        write_prg_byte(0x8000, 0x20);         // PRG RAM ENABLE
        write_prg_byte(0xA001, 0x30);         // PRG RAM PROTECT - Enable reading/writing to RAM at $7000-$71FF
        writeBankWRAM(0x1000, 0x1200, base);  // 512B

        write_prg_byte(0x8000, 0x20);         // PRG RAM ENABLE
        write_prg_byte(0xA001, 0xC0);         // PRG RAM PROTECT - Enable reading/writing to RAM at $7200-$73FF
        writeBankWRAM(0x1200, 0x1400, base);  // 512B

        write_prg_byte(0x8000, 0x6);          // PRG RAM DISABLE
      }
      else                                    // MMC3 8K
      {
        write_prg_byte(0xA001, 0x80);         // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
        writeBankPRG(0x0, 0x2000, base);      // 8K
        write_prg_byte(0xA001, 0xC0);         // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
      }
      break;

    case 5:                                         // 8K/16K/32K
      write_prg_byte(0x5100, 3);                    // 8K PRG Banks
      banks = OSCR::Util::power<2>(NES_RAM) / 2;          // banks = 1,2,4

      if (banks == 2) // 16K - Split SRAM Chips 8K/8K [ETROM = 16K (ONLY 1ST 8K BATTERY BACKED)]
      {
        for (size_t i = 0; i < (banks / 2); i++)    // Chip 1
        {
          write_prg_byte(0x5113, i);

          for (size_t address = 0; address < 0x2000; address += 512) // 8K
          {
            writeMMC5RAM(base, address);
          }
        }

        for (size_t j = 4; j < (banks / 2) + 4; j++) // Chip 2
        {
          write_prg_byte(0x5113, j);

          for (size_t address = 0; address < 0x2000; address += 512) // 8K
          {
            writeMMC5RAM(base, address);
          }
        }
      }
      else // 8K/32K Single SRAM Chip [EKROM = 8K BATTERY BACKED, EWROM = 32K BATTERY BACKED]
      {
        for (size_t i = 0; i < banks; i++) {  // banks = 1 or 4
          write_prg_byte(0x5113, i);
          for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
            writeMMC5RAM(base, address);
          }
        }
      }
      break;

    case 16:  // 256-byte EEPROM 24C02
    case 159: // 128-byte EEPROM 24C01 [Little Endian]
      eepsize = (NES_MAPPER == 159) ? 128 : 256;

      OSCR::Storage::Shared::readBuffer(eepsize);

      for (size_t address = 0; address < eepsize; address++)
      {
        eepromWrite(address);

        if ((address % 128) == 0)
        {
          OSCR::UI::clear();
        }

        OSCR::UI::printSync(F("."));
      }

      break;

    case 19:
      if (NES_RAM == 2) // PRG RAM 128B
      {
        OSCR::Storage::Shared::readBuffer(128);

        for (size_t x = 0; x < 128; x++)
        {
          write_ram_byte(0xF800, x);            // PRG RAM ENABLE
          write_prg_byte(0x4800, OSCR::Storage::Shared::buffer[x]);  // DATA PORT
        }
      }
      else
      {                                   // SRAM 8K
        for (size_t i = 0; i < 64; i++)   // Init Register
        {
          write_ram_byte(0xF800, 0x40);   // PRG RAM WRITE ENABLE
        }

        write_ram_byte(0xF800, 0x40);     // PRG RAM WRITE ENABLE
        writeBankPRG(0x0, 0x2000, base);  // 8K
        write_ram_byte(0xF800, 0x0F);     // PRG RAM WRITE PROTECT
      }

      break;

    case 80:                                                             // 1K
      write_prg_byte(0x7EF8, 0xA3);                                      // PRG RAM ENABLE 0
      write_prg_byte(0x7EF9, 0xA3);                                      // PRG RAM ENABLE 1
      for (size_t address = 0x1F00; address < 0x2000; address += 512)    // PRG RAM 1K ($7F00-$7FFF)
      {
        OSCR::Storage::Shared::readBuffer(128);

        for (size_t x = 0; x < 128; x++)
        {
          write_prg_byte(base + address + x, OSCR::Storage::Shared::buffer[x]);
        }
      }

      write_prg_byte(0x7EF8, 0xFF);  // PRG RAM DISABLE 0
      write_prg_byte(0x7EF9, 0xFF);  // PRG RAM DISABLE 1

      break;

    case 82:                                                           // 5K
      write_prg_byte(0x7EF7, 0xCA);                                    // PRG RAM ENABLE 0 ($6000-$67FF)
      write_prg_byte(0x7EF8, 0x69);                                    // PRG RAM ENABLE 1 ($6800-$6FFF)
      write_prg_byte(0x7EF9, 0x84);                                    // PRG RAM ENABLE 2 ($7000-$73FF)

      for (size_t address = 0x0; address < 0x1400; address += 1024)    // PRG RAM 5K ($6000-$73FF)
      {
        OSCR::Storage::Shared::fill();

        uint8_t firstbyte = OSCR::Storage::Shared::buffer[0];

        for (size_t x = 0; x < 512; x++)
        {
          write_prg_byte(base + address + x, OSCR::Storage::Shared::buffer[x]);
        }

        OSCR::Storage::Shared::fill();

        for (size_t x = 0; x < 512; x++)
        {
          write_prg_byte(base + address + x + 512, OSCR::Storage::Shared::buffer[x]);
        }

        write_prg_byte(base + address, firstbyte);  // REWRITE 1ST BYTE
      }

      write_prg_byte(0x7EF7, 0xFF);  // PRG RAM DISABLE 0 ($6000-$67FF)
      write_prg_byte(0x7EF8, 0xFF);  // PRG RAM DISABLE 1 ($6800-$6FFF)
      write_prg_byte(0x7EF9, 0xFF);  // PRG RAM DISABLE 2 ($7000-$73FF)

      break;

    default:
      if (NES_MAPPER == 118)            // 8K
      {
        write_prg_byte(0xA001, 0x80);   // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
      }
      else if ((NES_MAPPER == 21) || (NES_MAPPER == 25)) // 8K
      {
        write_prg_byte(0x8000, 0);
      }
      else if (NES_MAPPER == 26)        // 8K
      {
        write_prg_byte(0xB003, 0x80);   // PRG RAM ENABLE
      }
      //else if (NES_MAPPER == 68)        // 8K
      //{
      //  write_reg_byte(0xF000, 0x10);     // PRG RAM ENABLE [WRITE RAM SAFE]
      //}
      else if (NES_MAPPER == 69)        // 8K
      {
        write_prg_byte(0x8000, 8);      // Command Register - PRG Bank 0
        write_prg_byte(0xA000, 0xC0);   // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
      }
      else if (NES_MAPPER == 85)        // 8K
      {
        write_ram_byte(0xE000, 0x80);   // PRG RAM ENABLE
      }
      else if (NES_MAPPER == 153)       // 8K
      {
        write_prg_byte(0x800D, 0x20);   // PRG RAM Chip Enable
      }

      writeBankPRG(0x0, 0x2000, base);

      if (NES_MAPPER == 118)            // 8K
      {
        write_prg_byte(0xA001, 0xC0);   // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
      }
      else if (NES_MAPPER == 26)        // 8K
      {
        write_prg_byte(0xB003, 0);      // PRG RAM DISABLE
      }
      //else if (NES_MAPPER == 68)        // 8K
      //{
      //  write_reg_byte(0xF000, 0x00);     // PRG RAM DISABLE [WRITE RAM SAFE]
      //}
      else if (NES_MAPPER == 69)        // 8K
      {
        write_prg_byte(0x8000, 8);  // Command Register - PRG Bank 0
        write_prg_byte(0xA000, 0);  // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
      }
      else if (NES_MAPPER == 85)      // 8K
      {
        write_reg_byte(0xE000, 0);  // PRG RAM DISABLE [WRITE RAM SAFE]
      }

      break;
    }

    OSCR::Storage::Shared::close();
  }

  /******************************************
     Eeprom Functions
  *****************************************/
  // EEPROM MAPPING
  // 00-01 FOLDER #
  // 02-05 SNES/GB READER SETTINGS
  // 06 LED - ON/OFF [SNES/GB]
  // 07 MAPPER
  // 08 PRG SIZE
  // 09 CHR SIZE
  // 10 RAM SIZE

  void eepromStart()
  {
    write_prg_byte(0x800D, 0x00);  // sda low, scl low
    write_prg_byte(0x800D, 0x60);  // sda, scl high
    write_prg_byte(0x800D, 0x20);  // sda low, scl high
    write_prg_byte(0x800D, 0x00);  // START
  }

  void eepromStop()
  {
    write_prg_byte(0x800D, 0x00);  // sda, scl low
    write_prg_byte(0x800D, 0x20);  // sda low, scl high
    write_prg_byte(0x800D, 0x60);  // sda, scl high
    write_prg_byte(0x800D, 0x40);  // sda high, scl low
    write_prg_byte(0x800D, 0x00);  // STOP
  }

  void eepromSet0()
  {
    write_prg_byte(0x800D, 0x00);  // sda low, scl low
    write_prg_byte(0x800D, 0x20);  // sda low, scl high // 0
    write_prg_byte(0x800D, 0x00);  // sda low, scl low
  }

  void eepromSet1()
  {
    write_prg_byte(0x800D, 0x40);  // sda high, scl low
    write_prg_byte(0x800D, 0x60);  // sda high, scl high // 1
    write_prg_byte(0x800D, 0x40);  // sda high, scl low
    write_prg_byte(0x800D, 0x00);  // sda low, scl low
  }

  void eepromStatus()
  {        // ACK
    write_prg_byte(0x800D, 0x40);  // sda high, scl low
    write_prg_byte(0x800D, 0x60);  // sda high, scl high
    write_prg_byte(0x800D, 0xE0);  // sda high, scl high, read high
    uint8_t eepStatus = 1;
    do {
      eepStatus = (read_prg_byte(0x6000) & 0x10) >> 4;
      delayMicroseconds(4);
    } while (eepStatus == 1);
    write_prg_byte(0x800D, 0x40);  // sda high, scl low
  }

  void eepromReadData()
  {
    // read serial data into buffer
    for (uint8_t i = 0; i < 8; i++) {
      write_prg_byte(0x800D, 0x60);                     // sda high, scl high, read low
      write_prg_byte(0x800D, 0xE0);                     // sda high, scl high, read high
      eepbit[i] = (read_prg_byte(0x6000) & 0x10) >> 4;  // Read 0x6000 with Mask 0x10 (bit 4)
      write_prg_byte(0x800D, 0x40);                     // sda high, scl low
    }
  }

  void eepromDevice()
  {  // 24C02 ONLY
    eepromSet1();
    eepromSet0();
    eepromSet1();
    eepromSet0();
    eepromSet0();  // A2
    eepromSet0();  // A1
    eepromSet0();  // A0
  }

  void eepromReadMode()
  {
    eepromSet1();    // READ
    eepromStatus();  // ACK
  }

  void eepromWriteMode()
  {
    eepromSet0();    // WRITE
    eepromStatus();  // ACK
  }

  void eepromFinish()
  {
    write_prg_byte(0x800D, 0x00);  // sda low, scl low
    write_prg_byte(0x800D, 0x40);  // sda high, scl low
    write_prg_byte(0x800D, 0x60);  // sda high, scl high
    write_prg_byte(0x800D, 0x40);  // sda high, scl low
    write_prg_byte(0x800D, 0x00);  // sda low, scl low
  }

  // 24C01 [Little Endian]
  void eepromSetAddress01(uint8_t address)
  {
    for (uint8_t i = 0; i < 7; i++)
    {
      if (address & 0x1)  // Bit is HIGH
      {
        eepromSet1();
      }
      else  // Bit is LOW
      {
        eepromSet0();
      }

      address >>= 1;  // rotate to the next bit
    }
  }

  // 24C02
  void eepromSetAddress02(uint8_t address)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      if ((address >> 7) & 0x1)  // Bit is HIGH
      {
        eepromSet1();
      }
      else  // Bit is LOW
      {
        eepromSet0();
      }

      address <<= 1;  // rotate to the next bit
    }

    eepromStatus();  // ACK
  }

  // 24C01 [Little Endian]
  void eepromWriteData01(uint8_t & data)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      if (data & 0x1)  // Bit is HIGH
      {
        eepromSet1();
      }
      else  // Bit is LOW
      {
        eepromSet0();
      }

      data >>= 1;  // rotate to the next bit
    }

    eepromStatus();  // ACK
  }

  // 24C02
  void eepromWriteData02(uint8_t & data)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      if ((data >> 7) & 0x1)  // Bit is HIGH
      {
        eepromSet1();
      }
      else  // Bit is LOW
      {
        eepromSet0();
      }

      data <<= 1;  // rotate to the next bit
    }

    eepromStatus();  // ACK
  }

  void eepromRead(uint8_t address)
  {
    eepromStart();

    // 24C01
    if (NES_MAPPER == 159)
    {
      eepromSetAddress01(address);  // 24C01 [Little Endian]
      eepromReadMode();
      eepromReadData();
      eepromFinish();
      eepromStop();  // STOP

      // OR 8 bits into byte
      OSCR::Storage::Shared::buffer[address] = eepbit[7] << 7 | eepbit[6] << 6 | eepbit[5] << 5 | eepbit[4] << 4 | eepbit[3] << 3 | eepbit[2] << 2 | eepbit[1] << 1 | eepbit[0];

      return;
    }

    // 24C02
    eepromDevice();  // DEVICE [1010] + ADDR [A2-A0]
    eepromWriteMode();
    eepromSetAddress02(address);
    eepromStart();
    eepromDevice();  // DEVICE [1010] + ADDR [A2-A0]
    eepromReadMode();
    eepromReadData();
    eepromFinish();
    eepromStop();

    // OR 8 bits into byte
    OSCR::Storage::Shared::buffer[address] = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  }

  void eepromWrite(uint8_t address)
  {
    eepromStart();

    if (NES_MAPPER == 159) // 24C01
    {
      eepromSetAddress01(address);  // 24C01 [Little Endian]
      eepromWriteMode();
      eepromWriteData01(OSCR::Storage::Shared::buffer[address]);  // 24C01 [Little Endian]
    }
    else // 24C02
    {
      eepromDevice();   // DEVICE [1010] + ADDR [A2-A0]
      eepromWriteMode();
      eepromSetAddress02(address);
      eepromWriteData02(OSCR::Storage::Shared::buffer[address]);
    }

    eepromStop();  // STOP
  }

# if HAS_FLASH
  /******************************************
     NESmaker Flash Cart [SST 39SF40]
  *****************************************/
  void NESmaker_Cmd(uint8_t cmd)
  {
    write_prg_byte(0xC000, 0x01);
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xC000, 0x00);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(0xC000, 0x01);
    write_prg_byte(0x9555, cmd);
  }

  // SST 39SF040 Software ID
  void NESmaker_ID()
  {
    // Read Flash ID
    NESmaker_Cmd(0xFF);  // Reset
    NESmaker_Cmd(0x90);  // Software ID Entry

    flashid = read_prg_byte(0x8000) << 8;
    flashid |= read_prg_byte(0x8001);

    NESmaker_Cmd(0xF0);     // Software ID Exit

    if (flashid == 0xBFB7)  // SST 39SF040
    {
      flashfound = 1;
    }
  }

  void NESmaker_SectorErase(uint8_t bank, uint16_t address)
  {
    NESmaker_Cmd(0x80);
    write_prg_byte(0xC000, 0x01);
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xC000, 0x00);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(0xC000, bank);   // $00-$1F
    write_prg_byte(address, 0x30);  // Sector Erase ($8000/$9000/$A000/$B000)
  }

  void NESmaker_ByteProgram(uint8_t bank, uint16_t address, uint8_t data)
  {
    NESmaker_Cmd(0xA0);
    write_prg_byte(0xC000, bank);   // $00-$1F
    write_prg_byte(address, data);  // $8000-$BFFF
  }

  // SST 39SF040 Chip Erase [NOT IMPLEMENTED]
  void NESmaker_ChipErase()
  {
    // Typical 70ms
    NESmaker_Cmd(0x80);
    NESmaker_Cmd(0x10);  // Chip Erase
  }

  void writeFlash()
  {
    printHeader();

    if (!flashfound)
    {
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotSupportedByCart));
    }

    uint16_t base = 0x8000;
    uint8_t bytecheck;

    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    OSCR::UI::printHexLine(flashid);
    OSCR::UI::printLine();
    OSCR::UI::printLine(F("NESmaker Flash Found"));
    OSCR::UI::waitButton();

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    uint16_t banks = OSCR::Util::power<2>(NES_PRG); // 256K/512K

    for (size_t i = 0; i < banks; i++) // 16K Banks
    {
      for (size_t sector = 0; sector < 0x4000; sector += 0x1000) // 4K Sectors ($8000/$9000/$A000/$B000)
      {
        // Sector Erase
        NESmaker_SectorErase(i, base + sector);

        delay(18); // Typical 18ms

        for (uint8_t j = 0; j < 2; j++) // Confirm erase twice
        {
          do
          {
            bytecheck = read_prg_byte(base + sector);
            delay(18);
          }
          while (bytecheck != 0xFF);
        }

        // Program Byte
        for (size_t addr = 0x0; addr < 0x1000; addr += 512)
        {
          OSCR::Storage::Shared::fill();

          for (size_t x = 0; x < 512; x++)
          {
            uint16_t location = base + sector + addr + x;

            NESmaker_ByteProgram(i, location, OSCR::Storage::Shared::buffer[x]);

            delayMicroseconds(14); // Typical 14us

            for (uint8_t k = 0; k < 2; k++) // Confirm write twice
            {
              do
              {
                bytecheck = read_prg_byte(location);

                delayMicroseconds(14);
              }
              while (bytecheck != OSCR::Storage::Shared::buffer[x]);
            }
          }
        }
      }

      OSCR::UI::print(F("*"));

#   if !((HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864) || defined(ENABLE_OLED))
      if ((i != 0) && ((i + 1) % 16 == 0))
        OSCR::UI::printLine();
#   endif /* !(ENABLE_LCD || ENABLE_OLED) */
    }

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine();
    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }


  /******************************************
     A29040B Flash Cart [A29040B]
  *****************************************/

  // A29040B Software ID
  void A29040B_ID()
  {  // Read Flash ID
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(0x9555, 0x90);

    flashid = read_prg_byte(0x8000) << 8;
    flashid |= read_prg_byte(0x8001);
    if (flashid == 0x3786)  // A29040B
    {
      flashfound = 1;
    }

    A29040B_PRG_ResetFlash();
  }

  void A29040B_PRG_ResetFlash()
  {
    // Reset Flash
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(0x9555, 0xF0);  // Reset
    delayMicroseconds(14);         // Typical 14us
  }

  void A29040B_PRG_Write(uint16_t address, uint8_t data)
  {
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(0x9555, 0xA0);
    write_prg_byte(address, data);  // $8000-$BFFF
    delayMicroseconds(20);          // Typical 14us
  }

  void A29040B_PRG_SectorErase(uint16_t sec)
  {
    if (flashfound)
    {
      write_prg_byte(0x9555, 0xAA);
      write_prg_byte(0xAAAA, 0x55);
      write_prg_byte(0x9555, 0x80);  //->setup
      write_prg_byte(0x9555, 0xAA);
      write_prg_byte(0xAAAA, 0x55);
      write_prg_byte(sec, 0x30);  //->erase
      delay(1000);                // WAIT MORE
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotSupportedByCart));
    }
  }

  void A29040B_PRG_ChipErase()
  {
    if (flashfound)
    {
      write_prg_byte(0x9555, 0xAA);
      write_prg_byte(0xAAAA, 0x55);
      write_prg_byte(0x9555, 0x80);  //->setup
      write_prg_byte(0x9555, 0xAA);
      write_prg_byte(0xAAAA, 0x55);
      write_prg_byte(0x9555, 0x10);  //->erase
      delay(8000);                   // WAIT MORE
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotSupportedByCart));
    }
  }

  // CHR ================================================

  // Reset Flash
  void A29040B_CHR_ResetFlash()
  {
    write_chr_byte(0x0555, 0xAA);  // Original address for CHR
    write_chr_byte(0x02AA, 0x55);  // Original address for CHR
    write_chr_byte(0x0555, 0xF0);  // Reset command with original address
    delayMicroseconds(14);         // Typical 14us
  }

  void A29040B_CHR_Write(uint16_t address, uint8_t data)
  {
    write_chr_byte(0x0555, 0xAA);   // Original address for CHR
    write_chr_byte(0x02AA, 0x55);   // Original address for CHR
    write_chr_byte(0x0555, 0xA0);   // Program command with original address
    write_chr_byte(address, data);  // CHR address range (0x0000 - 0x1FFF)
    delayMicroseconds(20);          // Typical 14us
  }

  void A29040B_CHR_SectorErase(uint16_t sec)
  {
    if (flashfound)
    {
      write_chr_byte(0x0555, 0xAA);  // Original address for CHR
      write_chr_byte(0x02AA, 0x55);  // Original address for CHR
      write_chr_byte(0x0555, 0x80);  // Erase Setup with original address
      write_chr_byte(0x0555, 0xAA);  // Original address for CHR
      write_chr_byte(0x02AA, 0x55);  // Original address for CHR
      write_chr_byte(sec, 0x30);     // Sector Erase Command with sector address
      delay(1000);                   // WAIT MORE
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotSupportedByCart));
    }
  }

  void A29040B_CHR_ChipErase()
  {
    if (flashfound)
    {
      write_chr_byte(0x0555, 0xAA);  // Original address for CHR
      write_chr_byte(0x02AA, 0x55);  // Original address for CHR
      write_chr_byte(0x0555, 0x80);  // Erase Setup with original address
      write_chr_byte(0x0555, 0xAA);  // Original address for CHR
      write_chr_byte(0x02AA, 0x55);  // Original address for CHR
      write_chr_byte(0x0555, 0x10);  // Chip Erase Command with original address
      delay(8000);                   // WAIT MORE
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotSupportedByCart));
    }
  }

  void printHeaderA29040B()
  {
    OSCR::UI::printHeader(FS(headerA29040B));
  }

  void A29040B_writeFlash()
  {
    uint32_t prgSize = 0;
    uint32_t chrSize = 0;

    A29040B_ID();

    if (!flashfound)
    {
      OSCR::UI::printErrorHeader(FS(headerA29040B));

      OSCR::UI::printLabel(OSCR::Strings::Common::ID);
      OSCR::UI::printHexLine(flashid);

      OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
      return;
    }

    printHeaderA29040B();

    OSCR::UI::printLabel(OSCR::Strings::Common::ID);

    OSCR::UI::printHexLine(flashid);
    OSCR::UI::printLineSync();

    delay(3000);

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    // Step 1: Read the header and extract PRG and CHR sizes
    uint8_t header[16];
    uint32_t prgAddress = 0x8000;

    OSCR::Storage::Shared::sharedFile.read(header, 16);  // Read the 16-byte header

    prgSize = (uint32_t)header[4] * 16384;  // PRG size in bytes (header[4] gives size in 16 KB units)
    chrSize = (uint32_t)header[5] * 8192;   // CHR size in bytes (header[5] gives size in 8 KB units)

    // Output the sizes for verification
    printHeaderA29040B();

    OSCR::UI::printSize(OSCR::Strings::Common::PRG, prgSize);
    OSCR::UI::printSize(OSCR::Strings::Common::CHR, chrSize);

    delay(3000);

    // Step 2: Erase the entire PRG space
    printHeaderA29040B();

    A29040B_PRG_ResetFlash();

      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    A29040B_PRG_ChipErase();

    if (read_prg_byte(prgAddress) != 0xFF)
    {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::FAIL));
    }
    else
    {
        OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
    }

    // Verify that the first byte has been erased
    uint8_t erase_check = read_prg_byte(0x8000);

    if (erase_check != 0xFF)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::NotBlank));
      OSCR::UI::printHexLine(erase_check);
      return;
    }

    delay(18);  // Adjust delay as needed

    OSCR::UI::printLine();
    OSCR::UI::printLineSync(F("Writing PRG Data..."));

    A29040B_PRG_ResetFlash();

    // Step 3: Write PRG data
    uint32_t bytesProcessed = 0;
    OSCR::Storage::Shared::sharedFile.seekSet(16);  // Skip header to start of PRG data

    while (bytesProcessed < prgSize)
    {
      int bytesRead = OSCR::Storage::Shared::fill();
      if (bytesRead <= 0) break;

      for (int i = 0; i < bytesRead; i++)
      {
        A29040B_PRG_Write(prgAddress++, OSCR::Storage::Shared::buffer[i]);
        delayMicroseconds(14);  // Typical 14us

        uint8_t readByte = read_prg_byte(prgAddress - 1);

        delayMicroseconds(14);  // Typical 14us

        if (readByte != OSCR::Storage::Shared::buffer[i])
        {
          OSCR::Storage::Shared::close();

          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::HardwareProblem));
          OSCR::UI::printHex(readByte);
          OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
          OSCR::UI::printHexLine(OSCR::Storage::Shared::buffer[i]);

          OSCR::UI::printLabel(OSCR::Strings::Common::Address);
          OSCR::UI::printHexLine(prgAddress - 1);
          break;
        }
      }

      bytesProcessed += bytesRead;
    }

    // Step 4: Erase and Write CHR data
    A29040B_CHR_ResetFlash();

    printHeaderA29040B();
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    A29040B_CHR_ChipErase();

    delay(20);

    OSCR::UI::clear();

    uint32_t chrAddress = 0x0000;
    bytesProcessed = 0;
    OSCR::Storage::Shared::sharedFile.seekSet(16 + prgSize);  // Seek to the start of CHR data

    if (read_chr_byte(chrAddress) != 0xFF)
    {
      OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::HardwareProblem));
      OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
      delay(500);
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    }

      OSCR::UI::printLineSync(F("Writing CHR Data..."));

    while (bytesProcessed < chrSize)
    {
      int bytesRead = OSCR::Storage::Shared::readBuffer();

      if (bytesRead <= 0) break;

      for (int i = 0; i < bytesRead; i++)
      {
        A29040B_CHR_Write(chrAddress++, OSCR::Storage::Shared::buffer[i]);

        delayMicroseconds(14);  // Typical 14us

        uint8_t readByte = read_chr_byte(chrAddress - 1);

        delayMicroseconds(14);  // Typical 14us

        if (readByte != OSCR::Storage::Shared::buffer[i])
        {
          OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::HardwareProblem));

          OSCR::UI::printHex(readByte);
          OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
          OSCR::UI::printHexLine(OSCR::Storage::Shared::buffer[i]);

          OSCR::UI::printLabel(OSCR::Strings::Common::Address);
          OSCR::UI::printHexLine(chrAddress - 1);
          break;
        }
      }

      bytesProcessed += bytesRead;
    }

    delay(3000);

    OSCR::Storage::Shared::close();
  }
# endif /* HAS_FLASH */

} /* namespace OSCR::Cores::NES */

#endif /* HAS_NES */
