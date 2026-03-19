//******************************************************************************
//* Atari Jaguar Module                                                        *
//*                                                                            *
//* Author:  skaman / cephiros                                                 *
//* Date:    2024-07-05                                                        *
//*                                                                            *
//******************************************************************************
#include "config.h"

#if HAS_JAGUAR
# include "cores/include.h"
# include "cores/AtariJaguar.h"

namespace OSCR::Cores::AtariJaguar
{
  // HARDWARE PROFILE
  // PIN ASSIGNMENTS
  // DATA D0-D7 - PORTF
  // DATA D8-D15 - PORTK
  // DATA D16-D23 - PORTL
  // DATA D24-D31 - PORTC
  // 74HC595 (ADDR) PINS - PORTE: ADDR (PE3-PJ0), SRCLR (PE4), SRCLK (PE5)
  //                       PORTG: RCLK (PG5)
  // EEPROM PINS - EEPDO (PA5), EEPSK (PA6), EEPCS (PA7), [EEPDI = DATA D0 (PF0)]
  // CONTROL PINS - PORTH: OEH (PH3), OEL (PH4), CE (PH5), WE (PH6)

  using OSCR::Databases::Standard::romDetail;
  using OSCR::Databases::Standard::romRecord;

  //******************************************
  //  Defines
  //******************************************
  uint16_t tempDataLO = 0;
  uint16_t tempDataHI = 0;

  uint32_t cartSize;  // (0x20000)/0x100000/0x200000/0x400000
  uint8_t romSize = 0;        // 0 = 1MB, 1 = 2MB, 2 = 4MB
  // Variable to count errors
  //uint32_t writeErrors;
  // SAVE TYPE
  // 0 = SERIAL EEPROM
  // 1 = FLASH
  uint8_t saveType = 0;  // Serial EEPROM

  // SERIAL EEPROM 93CX6
  // CONTROL: EEPCS (PA7), EEPSK (PA6), EEPDI (PF0) [DATA D0]
  // SERIAL DATA OUTPUT: EEPDO (PA5)
  #define EEP_CS_SET PORTA |= (1 << 7)
  #define EEP_CS_CLEAR PORTA &= ~(1 << 7)
  #define EEP_SK_SET PORTA |= (1 << 6)
  #define EEP_SK_CLEAR PORTA &= ~(1 << 6)
  #define EEP_DI_SET PORTF |= (1 << 0)
  #define EEP_DI_CLEAR PORTF &= ~(1 << 0)

  // SERIAL EEPROM SIZES
  // 0 = 93C46 = 128 byte = Standard
  // 1 = 93C56 = 256 byte = Aftermarket
  // 2 = 93C66 = 512 byte = Aftermarket
  // 3 = 93C76 = 1024 byte = Aftermarket
  // 4 = 93C86 = 2048 byte = Aftermarket - Battlesphere Gold
  int eepSize;
  uint8_t eepBuf[2];
  bool eepBit[16];

  // MEMORY TRACK CART
  bool memorytrack = 0;
  //char flashID[5];            // AT29C010 = "1FD5"
  uint16_t flashID;
  uint32_t flaSize = 0;  // AT29C010 = 128K

  // JAGUAR EEPROM MAPPING
  // 08 ROM SIZE
  // 10 EEP SIZE

  //******************************************
  //  MENU
  //******************************************
  // Base Menu

  constexpr char const PROGMEM jagMenuItem6[] = "Read MEMORY TRACK";
  constexpr char const PROGMEM jagMenuItem7[] = "Write FLASH";

  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::SelectCart,
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::SetSize,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    jagMenuItem6,
    jagMenuItem7,
    OSCR::Strings::MenuOptions::Back,
  };

  constexpr char const PROGMEM jagRomItem1[] = "1MB ROM";
  constexpr char const PROGMEM jagRomItem2[] = "2MB ROM";
  constexpr char const PROGMEM jagRomItem3[] = "4MB ROM";
  constexpr char const * const PROGMEM jagRomMenu[] = {
    jagRomItem1,
    jagRomItem2,
    jagRomItem3,
  };

  constexpr char const PROGMEM jagEepItem1[] = "128B (93C46)";
  constexpr char const PROGMEM jagEepItem2[] = "256B (93C56)";
  constexpr char const PROGMEM jagEepItem3[] = "512B (93C66)";
  constexpr char const PROGMEM jagEepItem4[] = "1024B (93C76)";
  constexpr char const PROGMEM jagEepItem5[] = "2048B (93C86)";

  constexpr char const * const PROGMEM jagSaveMenu[] = {
    jagEepItem1,
    jagEepItem2,
    jagEepItem3,
    jagEepItem4,
    jagEepItem5,
  };

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::AtariJaguar));
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::AtariJaguar));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  void menu()
  {
    openCRDB();

    readID();

    if (flashID == 0x1FD5)
    {  // AT29C010 128K FLASH
      memorytrack = 1;                   // Memory Track Found
      saveType = 1;                      // FLASH
      strcpy_P(fileName, PSTR("jagMemorytrack"));
      cartSize = 0x20000;
      flaSize = 0x20000;
    }
    else
    {
      setCart();
    }

    getCartInfo();
    OSCR::UI::waitButton();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::AtariJaguar), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Select Cart
        setCart();
        break;

      case 1: // Read ROM
        readROM();
        break;

      case 2: // Set ROM Size
        sizeMenu();
        eepMenu();
        getCartInfo();
        break;

      case 3: // Read EEPROM
        if (saveType != 0)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotSupportedByCart));
          break;
        }

        readEEP();

        break;

      case 4: // Write EEPROM
        if (saveType != 0)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotSupportedByCart));
          break;
        }

        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);
        writeEEP();

        break;

      case 5: // Read MEMORY TRACK
        readMEMORY();

        if (saveType != 1)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotSupportedByCart));
          break;
        }

        readFLASH();

        break;

      case 6: // Write FLASH
        if (saveType != 1)
        {
          OSCR::UI::printLine(FS(OSCR::Strings::Errors::NotSupportedByCart));
          break;
        }

        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

        writeFLASH();
        verifyFLASH();

        break;

      case 7: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  //******************************************
  // CART SELECT CODE
  //******************************************

  void setCart()
  {
    OSCR::Databases::Standard::browseDatabase();

    switch (romDetail->mapper)
    {
    case 1:
      romSize = 0;
      cartSize = 0x100000;
      break;

    case 2:
      romSize = 1;
      cartSize = 0x200000;
      break;

    case 4:
      romSize = 2;
      cartSize = 0x400000;
      break;

    default:
      // unknown size
      return;
    }

    eepSize = romDetail->size;
    fromCRDB = true;
  }

  //******************************************************************************
  // ROM SIZE MENU
  //******************************************************************************
  void sizeMenu()
  {
    uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetROMSize), jagRomMenu, sizeofarray(jagRomMenu));

    switch (menuSelection)
    {
    case 0:
      romSize = 0;
      cartSize = 0x100000;
      break;

    case 1:
      romSize = 1;
      cartSize = 0x200000;
      break;

    case 2:
      romSize = 2;
      cartSize = 0x400000;
      break;
    }

    fromCRDB = false;
  }

  //******************************************************************************
  // EEPROM SIZE MENU
  //******************************************************************************
  void eepMenu()
  {
    uint8_t menuSelection = OSCR::UI::menu(FS(OSCR::Strings::MenuOptions::SetSize), jagSaveMenu, sizeofarray(jagSaveMenu));

    switch (menuSelection)
    {
    case 0:
      eepSize = 0;  // 128B
      //OSCR::UI::printLine(F("128B (93C46)"));
      break;

    case 1:
      eepSize = 1;  // 256B
      //OSCR::UI::printLine(F("256B (93C56)"));
      break;

    case 2:
      eepSize = 2;  // 512B
      //OSCR::UI::printLine(F("512B (93C66)"));
      break;

    case 3:
      eepSize = 3;  // 1024B
      //OSCR::UI::printLine(F("1024B (93C76)"));
      break;

    case 4:
      eepSize = 4;  // 2048B
      //OSCR::UI::printLine(F("2048B (93C86)"));
      break;
    }

    fromCRDB = false;
  }

  void cartOn()
  {
    // Request 5V
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output ADDR(PE3)(PJ0), SRCLR(PE4), SRCLK(PE5), RCLK(PG5)
    DDRE |= (1 << 3) | (1 << 4) | (1 << 5);
    DDRG |= (1 << 5);

    // Set Control Pins to Output OEH(PH3), OEL(PH4), CE(PH5), WE(PH6)
    DDRH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set EEPROM Control Pins to Output EEPSK(PA6), EEPCS(PA7)
    DDRA |= (1 << 6) | (1 << 7);

    // Set EEPROM Data to Input EEPDO(PA5)
    DDRA &= ~(1 << 5);

    // Set Data Pins (D0-D31) to Input
    DDRF = 0x00;
    DDRK = 0x00;
    DDRL = 0x00;
    DDRC = 0x00;

    // Set Control Pins HIGH - OEH(PH3), OEL(PH4), CE(PH5), WE(PH6)
    PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

    // Set 74HC595 (Address) Clear LOW - SRCLR (PE4)
    PORTE &= ~(1 << 4);  // Disable Address Shift Register

    // Print all the info
    if (romSize > 2)
    {
      romSize = 1;  // default 2MB
    }

    cartSize = long(OSCR::Util::power<2>(romSize)) * 0x100000;

    if (eepSize > 4)
    {
      eepSize = 0;  // default 128B
    }
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  //******************************************************************************
  // GET CART INFO
  //******************************************************************************
  void getCartInfo()
  {
    // Check for Memory Track NVRAM Cart

    printHeader();
    OSCR::UI::printValue(OSCR::Strings::Common::Name, fileName);

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, cartSize);

    OSCR::UI::printType_P(OSCR::Strings::Common::Save, (saveType == 0) ? OSCR::Strings::Common::EEPROM : OSCR::Strings::Common::Flash);

    OSCR::UI::printSize(OSCR::Strings::Common::Save, (saveType == 0) ? (OSCR::Util::power<2>(eepSize) * 128) : flaSize);  // 128/256/512/1024/2048 BYTES
  }

  //******************************************************************************
  // SHIFT OUT
  //******************************************************************************
  // SHIFT OUT ADDRESS CODE
  // SN74HC595 (ADDR) PINS - PORTE: ADDR (PJ3-PE3), SRCLR (PE4), SRCLK (PE5)
  //                         PORTG: RCLK (PG5)
  // DATA (SER/DS) PIN 14 = PE3 PJ0
  // /OE (GND) PIN 13
  // LATCH (RCLK/STCP) PIN 12 - LO/HI TO OUTPUT ADDRESS = PG5
  // CLOCK (SRCLK/SHCP) PIN 11 - LO/HI TO READ ADDRESS  = PE5
  // RESET (/SRCLR//MR) PIN 10  = PE4

  #define SER_CLEAR PORTE &= ~(1 << 3);
  #define SER_SET PORTE |= (1 << 3);
  #define SRCLR_CLEAR PORTE &= ~(1 << 4);
  #define SRCLR_SET PORTE |= (1 << 4);
  #define CLOCK_CLEAR PORTE &= ~(1 << 5);
  #define CLOCK_SET PORTE |= (1 << 5);
  #define LATCH_CLEAR PORTG &= ~(1 << 5);
  #define LATCH_SET PORTG |= (1 << 5);

  // INPUT ADDRESS BYTE IN MSB
  // LATCH LO BEFORE FIRST SHIFTOUT
  // LATCH HI AFTER LAST SHIFOUT
  void shiftOutFAST(uint8_t addr)
  {
    for (int i = 7; i >= 0; i--) // MSB
    {
      CLOCK_CLEAR;

      if (addr & (1 << i))
      {
        SER_SET; // 1
      }
      else
      {
        SER_CLEAR; // 0
      }

      CLOCK_SET;  // shift bit
      SER_CLEAR;  // reset 1
    }

    CLOCK_CLEAR;
  }

  //******************************************
  // READ DATA
  //******************************************
  void readData(uint32_t myAddress)
  {
    SRCLR_CLEAR;
    SRCLR_SET;
    LATCH_CLEAR;

    shiftOutFAST((myAddress >> 16) & 0xFF);
    shiftOutFAST((myAddress >> 8) & 0xFF);
    shiftOutFAST(myAddress);
    LATCH_SET;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(
      AVR_INS("nop")
    );

    // Setting CE(PH5) LOW
    PORTH &= ~(1 << 5);
    // Setting OEH3(PH3) + OEL(PH4) LOW
    PORTH &= ~(1 << 3) & ~(1 << 4);

    // Long delay here or there will be read errors
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
    );

    // Read
    tempDataLO = (((PINK & 0xFF) << 8) | (PINF & 0xFF));  // D0-D15 [ROM U1]
    tempDataHI = (((PINC & 0xFF) << 8) | (PINL & 0xFF));  // D16-D31 [ROM U2]

    // Setting OEH(PH3) + OEL(PH4) HIGH
    PORTH |= (1 << 3) | (1 << 4);
    // Setting CE(PH5) HIGH
    PORTH |= (1 << 5);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    SRCLR_CLEAR;
  }

  // Switch data pins to write
  void dataOut()
  {
    DDRF = 0xFF;
    DDRK = 0xFF;
    DDRL = 0xFF;
    DDRC = 0xFF;
  }

  // Switch data pins to read
  void dataIn()
  {
    DDRF = 0x00;
    DDRK = 0x00;
    DDRL = 0x00;
    DDRC = 0x00;
  }

  //******************************************************************************
  // READ ROM
  //******************************************************************************
  // Read rom and save to the SD card
  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::AtariJaguar), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    // Set control
    //dataIn();

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(cartSize));

    for (uint32_t currBuffer = 0, d = 0; currBuffer < cartSize; currBuffer += 512)
    {
      // MaskROM does not use A0 + A1 - Shift Address *4
      for (int currWord = 0; currWord < 512; currWord += 4, d += 4)
      {
        readData(currBuffer + currWord);
        // Move WORD into SD Buffer
        OSCR::Storage::Shared::buffer[d] = (tempDataHI >> 8) & 0xFF;
        OSCR::Storage::Shared::buffer[d + 1] = tempDataHI & 0xFF;
        OSCR::Storage::Shared::buffer[d + 2] = (tempDataLO >> 8) & 0xFF;
        OSCR::Storage::Shared::buffer[d + 3] = tempDataLO & 0xFF;
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::ProgressBar::finish();

    OSCR::Databases::Standard::matchCRC();
  }

  //*****************************************************************************
  // MICROCHIP EEPROM 93C56/66/76/86 CODE
  // SERIAL DATA OUTPUT (DO):  PIN PA5
  //
  // ISSI 93C46-3P EEPROM ONLY USES WORD MODE
  // EEPROM CODE IS WRITTEN IN WORD MODE (16bit)
  //
  // 93C46 (16bit): A5..A0, EWEN/ERAL/EWDS XXXX
  // 93C56 (16bit): A6..A0, EWEN/ERAL/EWDS XXXXXX
  // 93C66 (16bit): A7..A0, EWEN/ERAL/EWDS XXXXXX
  // 93C76 (16bit): A8..A0, EWEN/ERAL/EWDS XXXXXXXX
  // 93C86 (16bit): A9..A0, EWEN/ERAL/EWDS XXXXXXXX
  //
  // MICROCHIP EEPROM - TIE PIN 6 (ORG) TO VCC (ORG = 1) TO ENABLE 16bit MODE
  // FOR 93C76 & 93C86, TIE PIN 7 (PE) TO VCC TO ENABLE PROGRAMMING
  //*****************************************************************************
  void eepromDisplayClear()
  {
    DDRF |= (1 << 0);   // DATA PIN PF0 AS OUTPUT
    DDRA &= ~(1 << 5);  // DO INPUT
    EEP_CS_CLEAR;
    EEP_SK_CLEAR;
    EEP_DI_CLEAR;
  }

  void eepromReset()
  {
    DDRF &= ~(1 << 0);  // DATA PIN PF0 AS INPUT
    EEP_CS_CLEAR;
    EEP_SK_CLEAR;
    EEP_DI_CLEAR;
  }

  void eeprom0()
  {
    EEP_DI_CLEAR;
    EEP_SK_SET;
    _delay_us(1);  // minimum 250ns
    EEP_DI_CLEAR;
    EEP_SK_CLEAR;
    _delay_us(1);  // minimum 250ns
  }

  void eeprom1()
  {
    EEP_DI_SET;
    EEP_SK_SET;
    _delay_us(1);  // minimum 250ns
    EEP_DI_CLEAR;
    EEP_SK_CLEAR;
    _delay_us(1);  // minimum 250ns
  }

  void eepromRead(uint16_t addr)
  {
    eepromDisplayClear();
    EEP_CS_SET;
    eeprom1();  // 1
    eeprom1();  // 1
    eeprom0();  // 0
    if ((eepSize == 1) || (eepSize == 3))
      eeprom0();  // Dummy 0 for 56/76
    eepromSetAddress(addr);
    _delay_us(12);  // From Willem Timing
    // DATA OUTPUT
    eepromreadData();
    EEP_CS_CLEAR;
    // OR 16 bits into two bytes
    for (int j = 0; j < 16; j += 8) {
      eepBuf[(j / 8)] = eepBit[0 + j] << 7 | eepBit[1 + j] << 6 | eepBit[2 + j] << 5 | eepBit[3 + j] << 4 | eepBit[4 + j] << 3 | eepBit[5 + j] << 2 | eepBit[6 + j] << 1 | eepBit[7 + j];
    }
  }

  // Capture 2 bytes in 16 bits into bit array eepBit[]
  void eepromreadData()
  {
    for (int i = 0; i < 16; i++) {
      EEP_SK_SET;
      _delay_us(1);                         // minimum 250ns
      eepBit[i] = ((PINA & 0x20) >> 5);  // Read DO (PA5) - PINA with Mask 0x20
      EEP_SK_CLEAR;
      _delay_us(1);  // minimum 250ns
    }
  }

  void eepromWrite(uint16_t addr)
  {
    eepromDisplayClear();
    EEP_CS_SET;
    eeprom1();  // 1
    eeprom0();  // 0
    eeprom1();  // 1
    if ((eepSize == 1) || (eepSize == 3))
      eeprom0();  // Dummy 0 for 56/76
    eepromSetAddress(addr);
    // DATA OUTPUT
    eepromWriteData();
    EEP_CS_CLEAR;
    eepromStatus();
  }

  void eepromWriteData()
  {
    uint8_t UPPER = eepBuf[1];
    uint8_t LOWER = eepBuf[0];
    for (int i = 0; i < 8; i++) {
      if (((UPPER >> 7) & 0x1) == 1) {  // Bit is HIGH
        eeprom1();
      } else {  // Bit is LOW
        eeprom0();
      }
      // rotate to the next bit
      UPPER <<= 1;
    }
    for (int j = 0; j < 8; j++) {
      if (((LOWER >> 7) & 0x1) == 1) {  // Bit is HIGH
        eeprom1();
      } else {  // Bit is LOW
        eeprom0();
      }
      // rotate to the next bit
      LOWER <<= 1;
    }
  }

  void eepromSetAddress(uint16_t addr)
  {  // 16bit
    uint8_t shiftaddr = eepSize + 6;      // 93C46 = 0 + 6, 93C56 = 7, 93C66 = 8, 93C76 = 9, 93C86 = 10
    for (int i = 0; i < shiftaddr; i++) {
      if (((addr >> shiftaddr) & 1) == 1) {  // Bit is HIGH
        eeprom1();
      } else {  // Bit is LOW
        eeprom0();
      }
      // rotate to the next bit
      addr <<= 1;
    }
  }

  // EWEN/ERAL/EWDS
  // 93C56/93C66 - 10000xxxxxx (6 PULSES)
  // 93C76/93C86 - 10000xxxxxxxx (8 PULSES)
  void eepromEWEN()
  {  // EWEN 10011xxxx
    eepromDisplayClear();
    EEP_CS_SET;
    eeprom1();  // 1
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom1();  // 1
    eeprom1();  // 1
    // 46 = 4x Trailing 0s for 16bit
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom0();  // 0
    // 56/66 = 6x Trailing 0s for 16bit
    if (eepSize > 0) {
      eeprom0();  // 0
      eeprom0();  // 0
    }
    // 76/86 = 8x Trailing 0s for 16bit
    if (eepSize > 2) {
      eeprom0();  // 0
      eeprom0();  // 0
    }
    EEP_CS_CLEAR;
    _delay_us(2);
  }

  void eepromERAL()
  {  // ERASE ALL 10010xxxx
    EEP_CS_SET;
    eeprom1();  // 1
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom1();  // 1
    eeprom0();  // 0
    // 46 = 4x Trailing 0s for 16bit
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom0();  // 0
    // 56/66 = 6x Trailing 0s for 16bit
    if (eepSize > 0) {
      eeprom0();  // 0
      eeprom0();  // 0
    }
    // 76/86 = 8x Trailing 0s for 16bit
    if (eepSize > 2) {
      eeprom0();  // 0
      eeprom0();  // 0
    }
    EEP_CS_CLEAR;
    eepromStatus();
  }

  void eepromEWDS()
  {  // DISABLE 10000xxxx
    eepromDisplayClear();
    EEP_CS_SET;
    eeprom1();  // 1
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom0();  // 0
    // 46 = 4x Trailing 0s for 16bit
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom0();  // 0
    eeprom0();  // 0
    // 56/66 = 6x Trailing 0s for 16bit
    if (eepSize > 0) {
      eeprom0();  // 0
      eeprom0();  // 0
    }
    // 76/86 = 8x Trailing 0s for 16bit
    if (eepSize > 2) {
      eeprom0();  // 0
      eeprom0();  // 0
    }
    EEP_CS_CLEAR;
    _delay_us(2);
  }

  // CHECK READY/BUSY
  void eepromStatus()
  {
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
    ); // CS LOW for minimum 100ns

    EEP_CS_SET;

    bool status = ((PINA & 0x20) >> 5);  // Check DO

    do {
      _delay_ms(1);
      status = ((PINA & 0x20) >> 5);
    } while (!status);  // status == 0 = BUSY
    EEP_CS_CLEAR;
  }

  //*****************************************************************************
  // EEPROM
  // (0) 93C46 128B STANDARD
  // (1) 93C56 256B AFTERMARKET
  // (2) 93C66 512B AFTERMARKET
  // (3) 93C76 1024B AFTERMARKET
  // (4) 93C86 2048B AFTERMARKET - Battlesphere Gold
  //*****************************************************************************

  // Read EEPROM and save to the SD card
  void readEEP()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::AtariJaguar), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::SaveEEPROM));

    cartOn();

    uint16_t eepEnd = OSCR::Util::power<2>(eepSize) * 64;  // WORDS

    if (eepSize > 1) // 93C66/93C76/93C86 - 256/512/1024 WORDS
    {
      for (uint16_t currWord = 0; currWord < eepEnd; currWord += 256)
      {
        for (int i = 0; i < 256; i++)
        {
          eepromRead((currWord + i) * 2);
          OSCR::Storage::Shared::buffer[(i * 2)] = eepBuf[1];
          OSCR::Storage::Shared::buffer[(i * 2) + 1] = eepBuf[0];
        }

        OSCR::Storage::Shared::writeBuffer();
      }
    }
    else // 93C46/93C56 - 64/128 WORDS
    {
      for (uint16_t currWord = 0; currWord < eepEnd; currWord++)
      {
        eepromRead(currWord * 2);
        OSCR::Storage::Shared::buffer[(currWord * 2)] = eepBuf[1];
        OSCR::Storage::Shared::buffer[(currWord * 2) + 1] = eepBuf[0];
      }

      OSCR::Storage::Shared::writeBuffer(eepEnd * 2);
    }

    eepromReset();

    cartOff();

    OSCR::Storage::Shared::close();
  }

  // NOTE:  TO WRITE TO 93C76 & 93C86, MUST TIE PE (PROGRAM ENABLE) PIN 7 TO VCC
  void writeEEP()
  {
    printHeader();

    cartOn();

    eepromEWEN();  // ERASE/WRITE ENABLE
    eepromERAL();  // ERASE ALL

    uint16_t eepEnd = OSCR::Util::power<2>(eepSize) * 64;  // WORDS

    if (eepSize > 1) // 93C66/93C76/93C86
    {
      for (uint16_t currWord = 0; currWord < eepEnd; currWord += 256)
      {
        OSCR::Storage::Shared::fill();

        for (int i = 0; i < 256; i++)
        {
          eepBuf[0] = OSCR::Storage::Shared::buffer[(i * 2)];
          eepBuf[1] = OSCR::Storage::Shared::buffer[(i * 2) + 1];

          eepromWrite((currWord + i) * 2);
        }
      }
    }
    else
    {  // 93C46/93C56
      OSCR::Storage::Shared::readBuffer(eepEnd * 2);

      for (uint16_t currWord = 0; currWord < eepEnd; currWord++)
      {
        eepBuf[0] = OSCR::Storage::Shared::buffer[currWord * 2];
        eepBuf[1] = OSCR::Storage::Shared::buffer[(currWord * 2) + 1];

        eepromWrite(currWord * 2);
      }
    }

    eepromEWDS(); // ERASE/WRITE DISABLE
    eepromReset();

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  //*****************************************************************************
  // MEMORY TRACK NVRAM FLASH
  //*****************************************************************************
  void readID()
  {  // AT29C010 Flash ID "1FD5"
    // Switch to write
    cartOn();

    dataOut();

    // ID command sequence
    writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
    writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
    writeBYTE_FLASH(0x15554, 0x90);  // 0x5555

    // Switch to read
    dataIn();

    flashID = (readBYTE_FLASH(0x0000) << 8) | readBYTE_FLASH(0x0004);

    resetFLASH();

    cartOff();
  }

  void eraseFLASH()
  {  // Chip Erase (NOT NEEDED FOR WRITES)
    // Switch to write
    dataOut();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    // Erase command sequence
    writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
    writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
    writeBYTE_FLASH(0x15554, 0x80);  // 0x5555
    writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
    writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
    writeBYTE_FLASH(0x15554, 0x10);  // 0x5555

    // Wait for command to complete
    busyCheck();

    // Switch to read
    dataIn();
  }

  void resetFLASH()
  {
    // Switch to write
    dataOut();

    // Reset command sequence
    writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
    writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
    writeBYTE_FLASH(0x15554, 0xF0);  // 0x5555

    // Switch to read
    //dataIn();
    delayMicroseconds(10);
  }

  void busyCheck()
  {
    // Switch to read
    dataIn();

    // Read register
    readBYTE_FLASH(0x0000);

    // CE or OE must be toggled with each subsequent status read or the
    // completion of a program or erase operation will not be evident.
    while (((PINL >> 6) & 0x1) == 0) {  // IO6 = PORTL PL6
      // Setting CE(PH5) HIGH
      PORTH |= (1 << 5);

      // Leave CE high for at least 60ns
      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Setting CE(PH5) LOW
      PORTH &= ~(1 << 5);

      // Leave CE low for at least 50ns
      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Read register
      readBYTE_FLASH(0x0000);
    }

    // Switch to write
    dataOut();
  }

  uint8_t readBYTE_FLASH(uint32_t myAddress)
  {
    SRCLR_CLEAR;
    SRCLR_SET;
    LATCH_CLEAR;
    shiftOutFAST((myAddress >> 16) & 0xFF);
    shiftOutFAST((myAddress >> 8) & 0xFF);
    shiftOutFAST(myAddress);
    LATCH_SET;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Setting CE(PH5) LOW
    PORTH &= ~(1 << 5);
    // Setting OEH(PH3) HIGH
    PORTH |= (1 << 3);
    // Setting OEL(PH4) LOW
    PORTH &= ~(1 << 4);

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
    );

    // Read
    uint8_t tempByte = PINL;  // D16..D23

    // Setting CE(PH5) HIGH
    PORTH |= (1 << 5);
    // Setting OEL(PH4) HIGH
    PORTH |= (1 << 4);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    return tempByte;
  }

  uint8_t readBYTE_MEMROM(uint32_t myAddress)
  {
    SRCLR_CLEAR;
    SRCLR_SET;
    LATCH_CLEAR;
    shiftOutFAST((myAddress >> 16) & 0xFF);
    shiftOutFAST((myAddress >> 8) & 0xFF);
    shiftOutFAST(myAddress);
    LATCH_SET;

    // Arduino running at 16Mhz -> one nop = 62.5ns
    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Setting CE(PH5) LOW
    PORTH &= ~(1 << 5);
    // Setting OEH(PH3) LOW
    PORTH &= ~(1 << 3);

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
    );

    // Read
    uint8_t tempByte = PINF;  // D0..D7

    // Setting CE(PH5) HIGH
    PORTH |= (1 << 5);
    // Setting OEH(PH3) HIGH
    PORTH |= (1 << 3);

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    return tempByte;
  }

  void writeBYTE_FLASH(uint32_t myAddress, uint8_t myData)
  {
    SRCLR_CLEAR;
    SRCLR_SET;
    LATCH_CLEAR;
    shiftOutFAST((myAddress >> 16) & 0xFF);
    shiftOutFAST((myAddress >> 8) & 0xFF);
    shiftOutFAST(myAddress);
    LATCH_SET;

    PORTL = myData;

    AVR_ASM(
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
      AVR_INS("nop")
    );

    // Setting OEL(PH4) HIGH
    PORTH |= (1 << 4);
    // Setting CE(PH5) LOW
    PORTH &= ~(1 << 5);
    // Switch WE(PH6) LOW
    PORTH &= ~(1 << 6);
    delayMicroseconds(10);

    // Switch WE(PH6) HIGH
    PORTH |= (1 << 6);
    // Setting CE(PH5) HIGH
    PORTH |= (1 << 5);
    delayMicroseconds(10);
  }

  void writeSECTOR_FLASH(uint32_t myAddress)
  {
    dataOut();

    // Enable command sequence
    writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
    writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
    writeBYTE_FLASH(0x15554, 0xA0);  // 0x5555

    for (int i = 0; i < 128; i++)
    {
      SRCLR_CLEAR;
      SRCLR_SET;
      LATCH_CLEAR;
      shiftOutFAST((((myAddress + i) * 4) >> 16) & 0xFF);
      shiftOutFAST((((myAddress + i) * 4) >> 8) & 0xFF);
      shiftOutFAST((myAddress + i) * 4);  // (myAddress + i) * 4
      LATCH_SET;

      PORTL = OSCR::Storage::Shared::buffer[i];

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
        AVR_INS("nop")
      );

      // Setting OEL(PH4) HIGH
      //    PORTH |= (1 << 4);
      // Setting CE(PH5) LOW
      PORTH &= ~(1 << 5);
      // Switch WE(PH6) LOW
      PORTH &= ~(1 << 6);

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
      ); // Minimum 90ns

      // Switch WE(PH6) HIGH
      PORTH |= (1 << 6);
      // Setting CE(PH5) HIGH
      PORTH |= (1 << 5);

      AVR_ASM(
        AVR_INS("nop")
        AVR_INS("nop")
      ); // Minimum 100ns
    }
    delay(30);
  }

  //*****************************************************************************
  // MEMORY TRACK ROM U1
  // U1 ADDR PINS A0..A18 [USES A0..A17 = 0x20000 BYTES]
  // U1 DATA PINS D0..D7
  // /CE ON PIN 20A (CONTROLS BOTH U1 AND U2)
  // /OEH ON PIN 23B (CONTROLS U1 ROM ONLY)
  //*****************************************************************************
  void readMEMORY()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::AtariJaguar), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName));

    cartOn();

    //dataIn();

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(cartSize));

    for (uint32_t currBuffer = 0; currBuffer < cartSize; currBuffer += 512)
    {
      for (int currByte = 0; currByte < 512; currByte++)
      {
        OSCR::Storage::Shared::buffer[currByte] = readBYTE_MEMROM(currBuffer + currByte);
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::ProgressBar::finish();
  }

  //*****************************************************************************
  // MEMORY TRACK FLASH U2
  // AT29C010 = 0x20000 BYTES = 128 KB
  // U2 ADDR PINS A2..A18
  // U2 DATA PINS D16..D23
  // /CE ON PIN 20A (CONTROLS BOTH U1 AND U2)
  // /OEL ON PIN 22B (CONTROLS U2 FLASH ONLY)
  //*****************************************************************************
  void readFLASH()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::AtariJaguar), FS(OSCR::Strings::Directory::ROM), (fromCRDB ? romDetail->name : fileName), FS(OSCR::Strings::FileType::SaveFlash));

    cartOn();

    //dataIn();

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(flaSize));

    for (uint32_t currByte = 0; currByte < flaSize; currByte += 512)
    {
      for (int i = 0; i < 512; i++)
      {
        OSCR::Storage::Shared::buffer[i] = readBYTE_FLASH((currByte + i) * 4); // Address Shift A2..A18
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::ProgressBar::finish();
  }

  // AT29C010 - Write in 128 BYTE Secrors
  // Write Enable then Write DATA
  void writeFLASH()
  {
    cartOn();

    dataOut();

    //Initialize progress bar
    OSCR::UI::ProgressBar::init((uint32_t)(flaSize));

    for (uint32_t currByte = 0; currByte < flaSize; currByte += 128)
    {
      OSCR::Storage::Shared::readBuffer(128);

      writeSECTOR_FLASH(currByte);

      OSCR::UI::ProgressBar::advance(128);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::ProgressBar::finish();
  }

  uint32_t verifyFLASH()
  {
    writeErrors = 0;

    printHeader();

    cartOn();

    //dataIn();

    OSCR::Storage::Shared::openRO();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Verifying));

    for (uint32_t currByte = 0; currByte < flaSize; currByte += 512)
    {
      for (int i = 0; i < 512; i++)
      {
        OSCR::Storage::Shared::buffer[i] = readBYTE_FLASH((currByte + i) * 4);
      }

      // Check buffer content against file on sd card
      for (int j = 0; j < 512; j++)
      {
        if (OSCR::Storage::Shared::sharedFile.read() != OSCR::Storage::Shared::buffer[j])
        {
          writeErrors++;
        }
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    if (!writeErrors)
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    else
      OSCR::Lang::printErrorVerifyBytes(writeErrors);

    // Return 0 if verified ok, or number of errors
    return writeErrors;
  }
} /* namespace OSCR::Cores::AtariJaguar */

#endif /* HAS_JAGUAR */
