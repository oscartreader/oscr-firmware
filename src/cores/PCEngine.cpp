//******************************************
// PC Engine & TurboGrafx dump code by tamanegi_taro
// April 18th 2018 Revision 1.0.1 Initial version
// August 12th 2019 Revision 1.0.2 Added Tennokoe Bank support
// June 29th 2024 Revision 1.0.3 Added repro HuCard flashing
//
// Special thanks
// sanni - Arduino cart reader
// skaman - ROM size detection
// NO-INTRO - CRC list for game name detection
// Chris Covell - Tennokoe bank support
// partlyhuman - repro HuCard support
//
//******************************************
#include "config.h"

#if HAS_PCE
# include "cores/include.h"
# include "cores/PCEngine.h"
# include "cores/Flash.h"

namespace OSCR::Cores::PCEngine
{
  using OSCR::Cores::Flash::flashSize;

  /******************************************
    Defines
  *****************************************/
  #define DETECTION_SIZE 64
  #define FORCED_SIZE 1024
  #define CHKSUM_SKIP 0
  #define CHKSUM_OK 1
  #define CHKSUM_ERROR 2

  enum class Mode : uint8_t
  {
    HuCard,
    TurboChip,
    Flash,

    HuCard_NotSwapped = TurboChip,
  };

  /******************************************
     Prototype Declarations
  *****************************************/
  /* Several PCE dedicated functions */


  /******************************************
     Variables
  *****************************************/
  Mode mode = Mode::HuCard;

  /******************************************
    Menu
  *****************************************/
  // PCE start menu
  constexpr char const PROGMEM pceMenuItem1[] = "HuCard";
  constexpr char const PROGMEM pceMenuItem2[] = "TurboChip";
  constexpr char const PROGMEM pceMenuItem3[] = "Flash";

  constexpr char const * const PROGMEM menuOptions[] = {
    pceMenuItem1,
    pceMenuItem2,
# ifdef HAS_FLASH
    pceMenuItem3,
# endif /* HAS_FLASH */
    OSCR::Strings::MenuOptions::Back,
  };

  // PCE card menu items
  constexpr char const PROGMEM menuOptionsCart_1[] = "Read RAM Bank %d";
  constexpr char const PROGMEM menuOptionsCart_2[] = "Write RAM Bank %d";
  constexpr char const PROGMEM menuOptionsCart_3[] = "Inc Bank Number";
  constexpr char const PROGMEM menuOptionsCart_4[] = "Dec Bank Number";
  constexpr char const PROGMEM menuOptionsCart_5[] = "Set %dK ROM size";
  constexpr char const PROGMEM menuOptionsCart_5_fmt[] = "ROM size now %dK";

  // Turbochip menu items
  constexpr char const * const PROGMEM menuOptionsTC[] = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::Back,
  };

#ifdef HAS_FLASH
  // Flash repro menu items
  // constexpr char const PROGMEM menuOptionsFlash1[] = "Program";
  constexpr char const * const PROGMEM menuOptionsFlash[] = {
    OSCR::Strings::Common::Flash,
    OSCR::Strings::MenuOptions::Back,
  };
#endif /* HAS_FLASH */

  // PCE start menu, first a device type is selected and set in mode
  void menu(void)
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::PCEngine), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // HuCard (Swapped)
        mode = Mode::HuCard;
        hucardMenu();
        break;

      case 1: // TurboChip & HuCard (Not Swapped)
        mode = Mode::TurboChip;
        turbochipMenu();
        break;

      case 2: // Flash
# if HAS_FLASH
        mode = Mode::Flash;
        break;
# endif

      case 3: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  // PC Engine Menu
  void hucardMenu()
  {
    OSCR::UI::MenuOptions< 7, 20> huCardMenu;
    uint8_t tennokoe_bank_index = 0;

    romSize = 0;

    do
    {
      strlcpy_P(huCardMenu[0], OSCR::Strings::MenuOptions::ReadROM, menuoptionsize(huCardMenu));
      snprintf_P(huCardMenu[1], menuoptionsize(huCardMenu), menuOptionsCart_1, tennokoe_bank_index + 1);
      snprintf_P(huCardMenu[2], menuoptionsize(huCardMenu), menuOptionsCart_2, tennokoe_bank_index + 1);
      strlcpy_P(huCardMenu[3], menuOptionsCart_3, menuoptionsize(huCardMenu));
      strlcpy_P(huCardMenu[4], menuOptionsCart_4, menuoptionsize(huCardMenu));

      if (romSize > 0)
      {
        snprintf_P(huCardMenu[5], menuoptionsize(huCardMenu), menuOptionsCart_5_fmt, romSize);
      }
      else
      {
        snprintf_P(huCardMenu[5], menuoptionsize(huCardMenu), menuOptionsCart_5, FORCED_SIZE);
      }

      strlcpy_P(huCardMenu[6], OSCR::Strings::MenuOptions::Back, menuoptionsize(huCardMenu));

      huCardMenu.count = 7;

      switch (OSCR::UI::menu(F("PCE HuCard"), huCardMenu))
      {
      case 0:
        readROM();
        break;

      case 1:
        read_tennokoe_bank(tennokoe_bank_index);
        break;

      case 2:
        write_tennokoe_bank(tennokoe_bank_index);
        break;

      case 3:
        if (tennokoe_bank_index < 3)
        {
          tennokoe_bank_index++;
        }
        break;

      case 4:
        if (tennokoe_bank_index > 0)
        {
          tennokoe_bank_index--;
        }
        break;

      case 5:
        romSize = FORCED_SIZE;
        break;

      case 6:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void turbochipMenu()
  {
    mode = Mode::TurboChip;

    do
    {
      switch (OSCR::UI::menu(F("TG TURBOCHIP"), menuOptionsTC, sizeofarray(menuOptionsTC)))
      {
      case 0:
        readROM();
        break;

      case 1:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

#ifdef HAS_FLASH
  void flashMenu()
  {
    do
    {
      switch (OSCR::UI::menu(F("FLASH REPRO"), menuOptionsFlash, sizeofarray(menuOptionsFlash)))
      {
      case 0:
        flash();
        break;

      case 1:
        return;
      }

      OSCR::UI::waitButton();
    }
    while(true);
  }
#endif /* HAS_FLASH */

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::PCEngine));
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set cicrstPin(PG1) to Output
    DDRG |= (1 << 1);

    // Output a high to disable CIC
    PORTG |= (1 << 1);

    //Set Address Pins to input and pull up
    DDRF = 0x00;
    PORTF = 0xFF;
    DDRK = 0x00;
    PORTK = 0xFF;
    DDRL = 0x00;
    PORTL = 0xFF;
    DDRH &= ~((1 << 0) | (1 << 3) | (1 << 5) | (1 << 6));
    PORTH = (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);

    // Set IRQ(PH4) to Input
    DDRH &= ~(1 << 4);

    // Activate Internal Pullup Resistors
    PORTH |= (1 << 4);

    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;

    // Enable Internal Pullups
    PORTC = 0xFF;
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::PCEngine));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  void pin_read_write(void)
  {
    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;

    //A8-A15
    DDRK = 0xFF;

    //A16-A19
    DDRL = (DDRL & 0xF0) | 0x0F;

    //Set Control Pin to Output CS(PL4)
    DDRL |= (1 << 4);

    //Set CS(PL4) to HIGH
    PORTL |= (1 << 4);

    // Set Control Pins to Output RST(PH0) RD(PH3) WR(PH5)
    DDRH |= (1 << 0) | (1 << 3) | (1 << 5);

    // Switch all of above to HIGH
    PORTH |= (1 << 0) | (1 << 3) | (1 << 5);

    // Set IRQ(PH4) to Input
    DDRH &= ~(1 << 4);

    // Activate Internal Pullup Resistors
    PORTH |= (1 << 4);

    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;

    // Enable Internal Pullups
    PORTC = 0xFF;

    set_cs_rd_low();

    reset_cart();
  }

  void reset_cart()
  {
    //Set RESET as Low
    PORTH &= ~(1 << 0);
    delay(200);

    //Set RESET as High
    PORTH |= (1 << 0);
    delay(200);
  }

  void set_address(uint32_t address)
  {
    //Set address
    PORTF = address & 0xFF;
    PORTK = (address >> 8) & 0xFF;
    PORTL = (PORTL & 0xF0) | ((address >> 16) & 0x0F);
  }

  void set_cs_rd_low()
  {
    // Set CS(PL4) and RD(PH3) as LOW
    PORTL &= ~(1 << 4);
    PORTH &= ~(1 << 3);
  }

  uint8_t read_byte(uint32_t address)
  {
    uint8_t ret;

    set_address(address);

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

    //read byte
    ret = PINC;

    //Swap bit order for PC Engine HuCard
    if (mode == Mode::HuCard || mode == Mode::Flash)
    {
      ret = ((ret & 0x01) << 7) | ((ret & 0x02) << 5) | ((ret & 0x04) << 3) | ((ret & 0x08) << 1) | ((ret & 0x10) >> 1) | ((ret & 0x20) >> 3) | ((ret & 0x40) >> 5) | ((ret & 0x80) >> 7);
    }

    //return read data
    return ret;
  }

  void data_output()
  {
    // Set Data Pins (D0-D7) to Output
    DDRC = 0xFF;
  }

  void data_input()
  {
    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;

    // Enable Internal Pullups
    PORTC = 0xFF;

    set_cs_rd_low();
  }

  void write_byte(uint32_t address, uint8_t data)
  {
    //PORTH |= (1 << 3); // RD HIGH
    set_address(address);

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

    //Swap bit order for PC Engine HuCard
    if (mode == Mode::HuCard || mode == Mode::Flash)
    {
      data = ((data & 0x01) << 7) | ((data & 0x02) << 5) | ((data & 0x04) << 3) | ((data & 0x08) << 1) | ((data & 0x10) >> 1) | ((data & 0x20) >> 3) | ((data & 0x40) >> 5) | ((data & 0x80) >> 7);
    }

    //write byte
    PORTC = data;

    // Set CS(PL4) and WR(PH5) as LOW
    PORTL &= ~(1 << 4);
    PORTH &= ~(1 << 5);

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

    // Set CS(PL4) and WR(PH5) as HIGH
    PORTL |= (1 << 4);
    PORTH |= (1 << 5);
  }

  //Confirm the size of ROM
  void detect_rom_size()
  {
    uint8_t readByte;
    uint8_t current_byte;
    uint8_t detect_16, detect_32, detect_128, detect_256, detect_512, detect_768;

    //Initialize variables
    detect_16 = 0;
    detect_32 = 0;
    detect_128 = 0;
    detect_256 = 0;
    detect_512 = 0;
    detect_768 = 0;

    //Set pins to read PC Engine cart
    pin_read_write();

    //Confirm where mirror address start from (16KB, 32KB, 128KB, 256KB, 512KB, 768KB, or 1024KB)
    for (current_byte = 0; current_byte < DETECTION_SIZE; current_byte++)
    {
      if ((current_byte != detect_16) && (current_byte != detect_32) && (current_byte != detect_128) && (current_byte != detect_256) && (current_byte != detect_512) && (current_byte != detect_768))
      {
        //If none matched, it is 1024KB
        break;
      }

      //read byte for 16KB, 32KB, 128KB, 256KB, 512KB detection
      readByte = read_byte(current_byte);

      //16KB detection
      if (current_byte == detect_16)
      {
        if (read_byte(current_byte + 16UL * 1024UL) == readByte)
        {
          detect_16++;
        }
      }

      //32KB detection
      if (current_byte == detect_32)
      {
        if (read_byte(current_byte + 32UL * 1024UL) == readByte)
        {
          detect_32++;
        }
      }

      //128KB detection
      if (current_byte == detect_128)
      {
        if (read_byte(current_byte + 128UL * 1024UL) == readByte)
        {
          detect_128++;
        }
      }

      //256KB detection
      if (current_byte == detect_256)
      {
        if (read_byte(current_byte + 256UL * 1024UL) == readByte)
        {
          detect_256++;
        }
      }

      //512KB detection
      if (current_byte == detect_512)
      {
        if (read_byte(current_byte + 512UL * 1024UL) == readByte)
        {
          detect_512++;
        }
      }

      //768KB detection
      readByte = read_byte(current_byte + 512UL * 1024UL);

      if (current_byte == detect_768)
      {
        if (read_byte(current_byte + 768UL * 1024UL) == readByte)
        {
          detect_768++;
        }
      }
    }

    //ROM size detection by result
    if (detect_16 == DETECTION_SIZE)
    {
      romSize = 16;
    }
    else if (detect_32 == DETECTION_SIZE)
    {
      romSize = 32;
    }
    else if (detect_128 == DETECTION_SIZE)
    {
      romSize = 128;
    }
    else if (detect_256 == DETECTION_SIZE)
    {
      if (detect_512 == DETECTION_SIZE)
      {
        romSize = 256;
      }
      else
      {
        //romSize = 1024;
        //Another confirmation for 384KB because 384KB hucard has data in 0x0--0x40000 and 0x80000--0xA0000(0x40000 is mirror of 0x00000)
        romSize = 384;
      }
    }
    else if (detect_512 == DETECTION_SIZE)
    {
      romSize = 512;
    }
    else if (detect_768 == DETECTION_SIZE)
    {
      romSize = 768;
    }
    else
    {
      romSize = 1024;
    }

    //If rom size is more than or equal to 512KB, detect special cards
    if (romSize >= 512)
    {
      //Street Fighter II' - Champion Edition (Japan)
      if (read_byte(0x7FFF9) == 'N' && read_byte(0x7FFFA) == 'E' && read_byte(0x7FFFB) == 'C' && read_byte(0x7FFFC) == ' ' && read_byte(0x7FFFD) == 'H' && read_byte(0x7FFFE) == 'E')
      {
        romSize = 2560;
      }

      //Populous (Japan)
      if (read_byte(0x1F26) == 'P' && read_byte(0x1F27) == 'O' && read_byte(0x1F28) == 'P' && read_byte(0x1F29) == 'U' && read_byte(0x1F2A) == 'L' && read_byte(0x1F2B) == 'O' && read_byte(0x1F2C) == 'U' && read_byte(0x1F2D) == 'S')
      {
        romSize = 512;
      }

      //Dinoforce (Japan)
      if (read_byte(0x15A) == 'D' && read_byte(0x15B) == 'I' && read_byte(0x15C) == 'N' && read_byte(0x15D) == 'O' && read_byte(0x15E) == '-' && read_byte(0x15F) == 'F' && read_byte(0x160) == 'O' && read_byte(0x161) == 'R' && read_byte(0x162) == 'C' && read_byte(0x163) == 'E')
      {
        romSize = 512;
      }
    }

    if (romSize == 384)
    {
      //"CD-ROM² Super System Card (v3.0)(Japan)" or "Arcade Card Pro CD-ROM²"
      if (read_byte(0x29D1) == 'V' && read_byte(0x29D2) == 'E' && read_byte(0x29D3) == 'R' && read_byte(0x29D4) == '.' && read_byte(0x29D5) == ' ' && read_byte(0x29D6) == '3' && read_byte(0x29D7) == '.' && read_byte(0x29D8) == '0' && read_byte(0x29D9) == '0')
      {
        romSize = 256;
      }
    }
  }

  /* Must be address_start and address_end should be 512 byte aligned */
  void read_bank_ROM(uint32_t address_start, uint32_t address_end)
  {
    uint32_t currByte;
    uint16_t c;

    for (currByte = address_start; currByte < address_end; currByte += 512)
    {
      for (c = 0; c < 512; c++)
      {
        OSCR::Storage::Shared::buffer[c] = read_byte(currByte + c);
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }
  }

  void read_bank_RAM(uint32_t address_start, int block_index)
  {
    uint32_t start = address_start + block_index * 512;

    for (uint16_t c = 0; c < 512; c++)
    {
      OSCR::Storage::Shared::buffer[c] = read_byte(start + c);
    }
  }

  void unlock_tennokoe_bank_RAM()
  {
    data_output();

    write_byte(0x0D0000, 0x68);  //Unlock RAM sequence 1 Bank 68
    write_byte(0x0F0000, 0x00);  //Unlock RAM sequence 2 Bank 78
    write_byte(0x0F0000, 0x73);  //Unlock RAM sequence 3 Bank 78
    write_byte(0x0F0000, 0x73);  //Unlock RAM sequence 4 Bank 78
    write_byte(0x0F0000, 0x73);  //Unlock RAM sequence 5 Bank 78

    data_input();
  }

  void lock_tennokoe_bank_RAM()
  {
    data_output();

    write_byte(0x0D0000, 0x68);  //Lock RAM sequence 1 Bank 68
    write_byte(0x0F0001, 0x00);  //Lock RAM sequence 2 Bank 78
    write_byte(0x0C0001, 0x60);  //Lock RAM sequence 3 Bank 60

    data_input();
  }

  void read_tennokoe_bank(int bank_index)
  {
    printHeader();

    OSCR::UI::print(FS(OSCR::Strings::Labels::SAVE_SIZE));
    OSCR::UI::printLine(FS(OSCR::Strings::Units::Size2KB));

    // Get name, add extension and convert to char array for sd lib
    snprintf_P(fileName, kFileNameMax, PSTR("BANKRAM%d"), bank_index + 1);

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::PCEngine), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::Save));

    pin_read_write();

    for (int block_index = 0; block_index < 4; block_index++)
    {
      //Unlock Tennokoe Bank RAM

      // Disable interrupts
      noInterrupts();

      unlock_tennokoe_bank_RAM();

      // Read Tennokoe bank RAM
      read_bank_RAM(0x080000 + 2048UL * bank_index, block_index);

      // Lock Tennokoe Bank RAM
      lock_tennokoe_bank_RAM();

      // Enable interrupts
      interrupts();

      // hexdump:
      // for (int c = 0; c < 512; c += 16)
      // {
      //   for (int i = 0; i < 16; i++)
      //   {
      //     uint8_t b = OSCR::Storage::Shared::buffer[c + i];
      //     OSCR::UI::printHex(b);
      //     //OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));
      //   }
      //   OSCR::UI::printLine();
      // }

      if (block_index == 0)
      {
        OSCR::UI::print(F("Header: "));

        for (int i = 0; i < 4; i++)
        {
          uint8_t b = OSCR::Storage::Shared::buffer[i];
          OSCR::UI::printHex<false>(b);
        }

        OSCR::UI::printLine();
      }

      if (block_index == 0 && OSCR::Storage::Shared::buffer[2] == 0x42 && OSCR::Storage::Shared::buffer[3] == 0x4D)
      {
        if (OSCR::Storage::Shared::buffer[0] != 0x48 || OSCR::Storage::Shared::buffer[1] != 0x55)
        {
          OSCR::Storage::Shared::buffer[0] = 0x48;  // H
          OSCR::Storage::Shared::buffer[1] = 0x55;  // U

          OSCR::UI::printLine(F("Corrected header"));
        }
        else
        {
          OSCR::UI::printLine(F("Header is correct"));
        }
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::waitButton();
  }

  void write_tennokoe_bank(int bank_index)
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    fileSize = OSCR::Storage::Shared::getSize();

    if (fileSize != 2 * 1024UL)
    {
      OSCR::Storage::Shared::close();

      OSCR::UI::error(FS(OSCR::Strings::Errors::IncorrectFileSize));
      //OSCR::UI::printLine(FS(OSCR::Strings::Units::Size2KB));

      return;
    }

    pin_read_write();

    for (int block_index = 0; block_index < 4; block_index++)
    {
      OSCR::Storage::Shared::fill();

      //Unlock Tennokoe Bank RAM

      //Disable interrupts
      noInterrupts();

      unlock_tennokoe_bank_RAM();

      //Write file to Tennokoe BANK RAM
      uint32_t offset = 0x080000 + (bank_index * 2048UL) + (block_index * 512UL);

      for (uint16_t c = 0; c < 512; c++)
      {
        write_byte(offset + c, OSCR::Storage::Shared::buffer[c]);
      }

      //Lock Tennokoe Bank RAM
      lock_tennokoe_bank_RAM();

      //Enable interrupts
      interrupts();
    }

    // verify
    int diffcnt = 0;

    OSCR::Storage::Shared::rewind();

    for (int block_index = 0; block_index < 4; block_index++)
    {
      //Unlock Tennokoe Bank RAM

      //Disable interrupts
      noInterrupts();

      unlock_tennokoe_bank_RAM();

      //Read Tennokoe bank RAM
      read_bank_RAM(0x080000 + 2048UL * bank_index, block_index);

      //Lock Tennokoe Bank RAM
      lock_tennokoe_bank_RAM();

      //Enable interrupts
      interrupts();

      uint16_t diffcnt = 0;

      for (uint16_t c = 0; c < 512; c += 16)
      {
        uint8_t ram_b = OSCR::Storage::Shared::buffer[c];
        uint8_t file_b = OSCR::Storage::Shared::sharedFile.read();

        if (ram_b != file_b)
        {
          diffcnt++;
        }
      }
    }

    if (diffcnt == 0)
    {
      OSCR::UI::printLine(F("Verify OK..."));
    }
    else
    {
      OSCR::UI::printLine(F("Verify failed..."));
      OSCR::Lang::printErrorVerifyBytes(diffcnt);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(F("Finished"));

    OSCR::UI::waitButton();
  }

  void readROM()
  {
    bool forcedSize = (romSize == 0);

    printHeader();

    // Get name, add extension and convert to char array for sd lib
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::PCEngine), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::PCEngine));

    cartOn();

    if (!forcedSize)
    {
      detect_rom_size();
    }

    OSCR::UI::print(FS(OSCR::Strings::Labels::SIZE));

    OSCR::Lang::printBytes(romSize * 1024);

    OSCR::UI::printLine(FS(forcedSize ? OSCR::Strings::Symbol::Asterisk : OSCR::Strings::Symbol::Empty));

    pin_read_write();

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(romSize * 1024));

    if (romSize == 384)
    {
      //Read two sections. 0x000000--0x040000 and 0x080000--0x0A0000 for 384KB
      read_bank_ROM(0, 0x40000);
      read_bank_ROM(0x80000, 0xA0000);
    }
    else if (romSize == 2560)
    {
      //Dump Street fighter II' Champion Edition
      read_bank_ROM(0, 0x80000);  //Read first bank
      data_output();
      write_byte(0x1FF0, 0xFF);  //Display second bank
      data_input();
      read_bank_ROM(0x80000, 0x100000);  //Read second bank
      data_output();
      write_byte(0x1FF1, 0xFF);  //Display third bank
      data_input();
      read_bank_ROM(0x80000, 0x100000);  //Read third bank
      data_output();
      write_byte(0x1FF2, 0xFF);  //Display forth bank
      data_input();
      read_bank_ROM(0x80000, 0x100000);  //Read forth bank
      data_output();
      write_byte(0x1FF3, 0xFF);  //Display fifth bank
      data_input();
      read_bank_ROM(0x80000, 0x100000);  //Read fifth bank
    }
    else
    {
      //Read start form 0x000000 and keep reading until end of ROM
      read_bank_ROM(0, romSize * 1024UL);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();

    OSCR::UI::waitButton();
  }

#ifdef HAS_FLASH
  void flash_mode()
  {
    pin_read_write();
    data_output();
    PORTH |= (1 << 3);  // RD HIGH
    // write_byte sets WR
  }

  // Implements data complement status checking
  // We only look at D7, or the highest bit of expected
  void flash_wait_status(uint8_t expected)
  {
    uint8_t status;

    set_cs_rd_low();
    data_input();

    do
    {
      PORTH &= ~(1 << 3);  // RD low
      // one nop = 62.5ns
      // tOE = 30-50ns depending on flash
      NOP;
      status = PINC;
      PORTH |= (1 << 3);  // RD high
      // reversed, bit 0 is the MSB
    }
    while ((status & 0x1) != (expected >> 7));

    data_output();
    // leave RD high on exit
  }

  // Flashes a reproduction HuCard that's directly wired to a flash chip
  // Supported flash: SST39SF0x0, MX29F0x0 1Mbit-8Mbit
  // Developed against Ichigobankai's design https://github.com/partlyhuman/HuCARD-repro
  void flash()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Checking));

    // SOFTWARE ID PROGRAM
    flash_mode();
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x90);
    data_input();
    // tIDA = 150ns
    NOP; NOP; NOP;
    // MFG,DEVICE
    uint16_t deviceId = (read_byte(0x0) << 8) | read_byte(0x1);

    // EXIT SOFTWARE ID PROGRAM
    flash_mode();
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0xF0);

    flashSize = 0;

    switch (deviceId)
    {
    case 0xBFB5: // SST39SF010 = 1Mbit
      flashSize = 131072UL;
      break;

    case 0xBFB6: // SST39SF020 = 2Mbit
      flashSize = 262144UL;
      break;

    case 0xBFB7: // SST39SF040 = 4Mbit
      flashSize = 524288UL;
      break;

    case 0xC2A4: // MX29F040 = 4Mbit
      flashSize = 524288UL;
      break;

    case 0xC2D5: // MX29F080 = 8Mbit
    case 0x20F1: // M29F080 = 8Mbit
      flashSize = 1048576UL;
      break;
    }

    if (flashSize == 0)
    {
      OSCR::UI::print(F("UNKNOWN "));
      OSCR::UI::printLine(deviceId);
      OSCR::UI::waitButton();
      return;
    }

    OSCR::UI::print(FS(OSCR::Strings::Labels::SIZE));
    OSCR::Lang::printBytesLine(flashSize);

    OSCR::UI::waitButton();

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    // CHIP ERASE PROGRAM
    flash_mode();
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x80);
    write_byte(0x5555, 0xAA);
    write_byte(0x2AAA, 0x55);
    write_byte(0x5555, 0x10);

    // Data complement polling, wait until highest bit is 1
    flash_wait_status(0xFF);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    OSCR::UI::ProgressBar::init((uint32_t)(fileSize));

    flash_mode();

    for (uint32_t currAddr = 0; currAddr < fileSize; currAddr += 512)
    {
      OSCR::Storage::Shared::fill();

      for (int currByte = 0; currByte < 512; currByte++)
      {
        // BYTE PROGRAM
        uint8_t b = OSCR::Storage::Shared::buffer[currByte];

        write_byte(0x5555, 0xAA);
        write_byte(0x2AAA, 0x55);
        write_byte(0x5555, 0xA0);
        write_byte(currAddr + currByte, b);

        flash_wait_status(b);
      }

      // update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();

    cartOff();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::waitButton();
  }
#endif
} /* namespace OSCR::Cores::PCEngine */

#endif /* HAS_PCE */
