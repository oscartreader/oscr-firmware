//******************************************
// LITTLE JAMMER PRO MODULE
//******************************************
#include "config.h"

#if HAS_LJPRO
# include "cores/include.h"
# include "cores/LittleJammerPro.h"

namespace OSCR::Cores::LittleJammerPro
{
  // Little Jammer Pro
  // Cartridge Pinout
  // 48P 1.25mm pitch connector
  //
  // FORM FACTOR IS SAME AS BANDAI WONDERSWAN/BENESSE POCKET CHALLENGE V2/LITTLE JAMMER
  // WIRING IS COMPLETELY DIFFERENT!
  //
  // LEFT SIDE
  // 1 GND
  // 2 GND
  // 3 S1 (GND)
  // 4 S2 (GND)
  // 5 U1_WP#/ACC
  // 6 U1_SCLK
  // 7 U1_SCLK
  // 8 U1_SI
  // 9 U1_SI
  // 10 U1_SO/PO7
  // 11 U1_SO/PO7
  // 12 U1_PO6
  // 13 U1_PO5
  // 14 U1_PO4
  // 15 U1_PO3
  // 16 U1_PO2
  // 17 U1_PO1
  // 18 U1_PO0
  // 19 U1_CS#
  // 20 U1_CS#
  // 21 U1_HOLD#
  // 22 U1_HOLD#
  // 23 VCC (+3V)
  // 24 VCC (+3V)
  // 25 VCC (+3V)
  // 26 VCC (+3V)
  // 27 U2_SCLK
  // 28 U2_SCLK
  // 29 U2_SI
  // 30 U2_SI
  // 31 U2_SO/PO7
  // 32 U2_SO/PO7
  // 33 U2_PO6
  // 34 U2_PO5
  // 35 U2_PO4
  // 36 U2_PO3
  // 37 U2_PO2
  // 38 U2_PO1
  // 39 U2_PO0
  // 40 U2_CS#
  // 41 U2_CS#
  // 42 U2_HOLD#
  // 43 U2_HOLD#
  // 44 U2_WP#/ACC
  // 45 S3 (GND)
  // 46 S4 (GND)
  // 47 GND
  // 48 GND
  // RIGHT SIDE

  // CONTROL PINS:
  // U1_HOLD#    (PH4) - SNES /IRQ
  // U1_CS#      (PK0) - SNES A8
  // U1_SI       (PK1) - SNES A9
  // U1_WP#/ACC  (PK2) - SNES A10
  //
  // U2_HOLD#    (PH0) - SNES RESET
  // U2_SI       (PH3) - SNES /CS
  // U2_WP#/ACC  (PH5) - SNES /WR
  // U2_CS#      (PH6) - SNES /RD
  //
  // S1          (PK4) - SNES A12
  // S2          (PK5) - SNES A13
  // S3          (PK6) - SNES A14
  // S4          (PK7) - SNES A15

  // COMBINE U1_SCLK + U2_SCLK INTO SINGLE SCLK
  // SCLK(PH1)   - SNES CPUCLK

  // DATA PINS:
  // U1 D0-D7 (PORTF)
  // U2 D0-D7 (PORTC)

  // NOTES:
  // HOLD# NOT USED FOR PARALLEL MODE - PULLED UP TO VCC ON CARTS
  // WP#/ACC PULLED DOWN TO GND ON CARTS

  //******************************************
  // DEFINES
  //******************************************
  #define CS1_LOW   PORTK &= ~(1 << 0)
  #define CS1_HIGH  PORTK |= (1 << 0)
  #define CS2_LOW   PORTH &= ~(1 << 6)
  #define CS2_HIGH  PORTH |= (1 << 6)

  //******************************************
  // VARIABLES
  //******************************************
  uint8_t flash1size;
  uint8_t flash2size;

  // EEPROM MAPPING
  // 08 ROM SIZE

  //******************************************
  // MENU
  //******************************************
  // Base Menu
  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::Back,
  };

  // U1_HOLD#(PH4)    - SNES /IRQ
  // U1_CS#           - SNES A8
  // U1_WP#/ACC       - SNES A9
  // U1_SI            - SNES A10
  //
  // U2_HOLD#(PH0)    - SNES RESET
  // U2_SI(PH3)       - SNES /CS
  // U2_WP#/ACC(PH5)  - SNES /WR
  // U2_CS#(PH6)      - SNES /RD

  void menu()
  {
    openCRDB();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::LittleJammerPro), menuOptions, sizeofarray(menuOptions)))
      {
      case 0: // Read ROM
        readROM();
        break;

      case 1: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::LittleJammerPro));
  }

  //******************************************
  // SETUP
  //******************************************

  void cartOn()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    // LITTLE JAMMER PRO uses Serial Flash
    // Set Data Pins to Input
    DDRF = 0x00; // U1 Data
    DDRC = 0x00; // U2 Data
    // Set Unused Address Pins to Output
    DDRL = 0xFF;

    // Set Control Pins to Output
    //     U2_HLD(PH0) SCLK(PH1)  U2_SI(PH3) U1_HLD(PH4) U2_WP(PH5) U2_CS(PH6)
    DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    //      U1_CS(PK0) U1_SI(PK1) U1_WP(PK2) --------
    DDRK |=  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

    // FLASH Configuration Pins to Input
    //         S1(PK4)    S2(PK5)    S3(PK6)    S4(PK7)
    DDRK &= ~((1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));

    // Set TIME(PJ0) to Output (UNUSED)
    DDRJ |=  (1 << 0);

    // Setting Control Pins to HIGH
    //     U2_HLD(PH0) SCLK(PH1)  U2_SI(PH3) U1_HLD(PH4) U2_WP(PH5) U2_CS(PH6)
    PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    //      U1_CS(PK0)  U1_SI(PK1) U1_WP(PK2)  --------    S1(PK4)    S2(PK5)    S3(PK6)    S4(PK7)
    PORTK |=  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);

    // Set Unused Data Pins (PA0-PA7) to Output
    DDRA = 0xFF;

    // Set Unused Pins HIGH
    PORTA = 0xFF;
    PORTJ |= (1 << 0); // TIME(PJ0)

    if (!fromCRDB) useDefaultName();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Standard::setup(FS(OSCR::Strings::FileType::LittleJammerPro));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  // SERIAL MODE
  //******************************************
  // 25L4005/25L1605/25L3205/25L6405
  // Default Serial Mode

  void sendSerial_U1(uint8_t data)
  {
    for (int i = 0; i < 8; i++)
    {
      PORTH &= ~(1 << 1); // SCLK LOW

      if ((data >> 7) & 0x1) // Bit is HIGH
      {
        PORTK |= (1 << 1); // U1_SI HIGH;
      }
      else
      {
        PORTK &= ~(1 << 1); // U1_SI LOW;
      }

      PORTH |= (1 << 1); // SCLK HIGH

      // rotate to the next bit
      data <<= 1;
    }
  }

  void sendSerial_U2(uint8_t data)
  {
    for (int i = 0; i < 8; i++)
    {
      PORTH &= ~(1 << 1); // SCLK LOW

      if ((data >> 7) & 0x1) // Bit is HIGH
      {
        PORTH |= (1 << 3); // U2_SI HIGH;
      }
      else
      {
        PORTH &= ~(1 << 3); // U2_SI LOW;
      }

      PORTH |= (1 << 1); // SCLK HIGH

      // rotate to the next bit
      data <<= 1;
    }
  }

  uint8_t readSerial_U1()
  {
    bool serBits[9];

    for (uint8_t i = 0; i < 8; i++)
    {
      pulseClock(1);

      serBits[i] = (PINF >> 7) & 0x1;
    }

    uint8_t tempdata = serBits[0] << 7 | serBits[1] << 6 | serBits[2] << 5 | serBits[3] << 4 | serBits[4] << 3 | serBits[5] << 2 | serBits[6] << 1 | serBits[7];

    return tempdata;
  }

  uint8_t readSerial_U2()
  {
    bool serBits[9];

    for (uint8_t i = 0; i < 8; i++)
    {
      pulseClock(1);
      serBits[i] = (PINC >> 7) & 0x1;
    }

    uint8_t tempdata = serBits[0] << 7 | serBits[1] << 6 | serBits[2] << 5 | serBits[3] << 4 | serBits[4] << 3 | serBits[5] << 2 | serBits[6] << 1 | serBits[7];

    return tempdata;
  }

  // RDID
  // Manufacturer 0xC2
  // Memory Density 0x20
  // Device ID 0x13 [25L4005]/0x15 [25L1605]/0x16 [25L3205]/0x17 [25L6405]

  uint8_t deviceID(uint8_t id1, uint8_t id2)
  {
    if ((id1 != 0x20) || (id2 == 0)) return 0;

    switch (id2)
    {
    case 0x13:
      OSCR::UI::printLine(F("MX25L4005"));
      return 1;

    case 0x15:
      OSCR::UI::printLine(F("MX25L1605"));
      return 2;

    case 0x16:
      OSCR::UI::printLine(F("MX25L3205"));
      return 4;

    case 0x17:
      OSCR::UI::printLine(F("MX25L6405"));
      return 8;

    default:
      return 0;
    }
  }

  void readSerialID_U1()
  {
    CS1_LOW;

    DDRF = 0x00; // U1 Data Input

    sendSerial_U1(0x9F); // 0x9F = 10011111

    // Manufacturer Code
    //uint8_t id0 =
    readSerial_U1();

    // Device Code
    uint8_t id1 = readSerial_U1();
    uint8_t id2 = readSerial_U1();

    CS1_HIGH;

    // Flash ID
    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    flash1size = deviceID(id1, id2);
  }

  void readSerialID_U2()
  {
    CS2_LOW;

    DDRC = 0x00; // U2 Data Input
    sendSerial_U2(0x9F); // 0x9F = 10011111

    // Manufacturer Code
    //uint8_t id0 =
    readSerial_U2();

    // Device Code
    uint8_t id1 = readSerial_U2();
    uint8_t id2 = readSerial_U2();
    CS2_HIGH;

    // Flash ID
    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    flash2size = deviceID(id1, id2);
  }

  void readSerialData_U1(uint32_t startaddr, uint32_t endaddr)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (int x = 0; x < 512; x++)
      {
        OSCR::Storage::Shared::buffer[x] = readSerial_U1();
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  void readSerialData_U2(uint32_t startaddr, uint32_t endaddr)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (int x = 0; x < 512; x++)
      {
        OSCR::Storage::Shared::buffer[x] = readSerial_U2();
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  //******************************************
  // PARALLEL MODE
  //******************************************
  // 25L1605/25L3205/25L6405
  // Parallel Mode - Command 0x55
  // SCLK Frequency 1.2MHz (Cycle 833.33ns)
  // READ 0x03
  // WRITE 0x02

  void pulseClock(uint16_t times)
  {
    for (uint16_t i = 0; i < (times * 2); i++)
    {
      // Switch the clock pin to 0 if it's 1 and 0 if it's 1
      PORTH ^= (1 << 1);
      // without the delay the clock pulse would be 1.5us and 666kHz
      //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"));
    }
  }

  // Send one byte of data to Serial FLASH [Parallel Mode]
  void sendData_U1(uint8_t data)
  {
    DDRF = 0xFF; // U1 Data Output
    PORTF = data;
    pulseClock(8);
    DDRF = 0x00; // U1 Data Input
  }

  void sendData_U2(uint8_t data)
  {
    DDRC = 0xFF; // U2 Data Output
    PORTC = data;
    pulseClock(8);
    DDRC = 0x00; // U2 Data Input
  }

  void readData_U1(uint32_t startaddr, uint32_t endaddr)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (int x = 0; x < 512; x++)
      {
        pulseClock(1);
        OSCR::Storage::Shared::buffer[x] = PINF;
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  void readData_U2(uint32_t startaddr, uint32_t endaddr)
  {
    for (uint32_t addr = startaddr; addr < endaddr; addr += 512)
    {
      for (int x = 0; x < 512; x++)
      {
        pulseClock(1);
        OSCR::Storage::Shared::buffer[x] = PINC;
      }

      OSCR::Storage::Shared::writeBuffer();
    }
  }

  // RDID
  // Manufacturer 0xC2
  // Memory Density 0x20
  // Device ID 0x15 [25L1605]/0x16 [25L3205]/0x17 [25L6405]

  void readID_U1() // Parallel Mode
  {
    CS1_LOW; // U1 LOW
    sendSerial_U1(0x9F); // RDID Command

    pulseClock(1);

    //uint8_t id0 = PINF; // 0xC2

    pulseClock(1);

    uint8_t id1 = PINF; // 0x20

    pulseClock(1);

    uint8_t id2 = PINF; // 0x15 [MX25L1605]/0x16 [MX25L3205]/0x17 [MX25L6405]

    CS1_HIGH; // U1 HIGH

    // Flash ID
    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    flash1size = deviceID(id1, id2);
  }

  void readID_U2() // Parallel Mode
  {
    CS2_LOW; // U2 LOW

    sendSerial_U2(0x9F); // RDID Command

    pulseClock(1);

    //uint8_t id0 = PINC; // 0xC2

    pulseClock(1);

    uint8_t id1 = PINC; // 0x20

    pulseClock(1);

    uint8_t id2 = PINC; // 0x15 [MX25L1605]/0x16 [MX25L3205]/0x17 [MX25L6405]

    pulseClock(1);

    CS2_HIGH; // U2 HIGH

    // Flash ID
    OSCR::UI::printLabel(OSCR::Strings::Common::ID);
    flash1size = deviceID(id1, id2);

    OSCR::UI::update();
  }

  //******************************************
  // READ ROM
  //******************************************

  void readROM()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::LittleJammerPro), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    cartOn();

    // Little Jammer Pro PCB B1043-02A
    // Footprints for two 25L1605/25L3205 chips
    // Test carts only have one 25L1605 (2MB) installed
    // PCB could possibly install two 25L3205 chips (2x4MB = 8MB)

    // Little Jammer Pro PCB B1043-03B
    // Footprints for 25L4005 + 25L6405 chips
    // U1 = 25L4005 (512KB), U2 = 25L6405 (8MB)

    // Reset Flash Settings
    flash1size = 0;
    flash2size = 0;

    readSerialID_U1();

    if (!flash1size)
    {
      // Set U1 FLASH to Parallel Mode
      CS1_LOW; // U1 LOW
      sendSerial_U1(0x55); // Parallel Mode
      CS1_HIGH; // U1 HIGH
      readID_U1(); // Read ID using Parallel Mode
    }

    readSerialID_U2();

    if (!flash2size)
    {
      // Set U2 FLASH to Parallel Mode
      CS2_LOW; // U2 LOW
      sendSerial_U2(0x55); // Parallel Mode
      CS2_HIGH; // U2 HIGH
      readID_U2(); // Read ID using Parallel Mode
    }

    // NOTE:  Setting Flash to Parallel Mode stays in effect until Power Cycle

    // Read U1
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

    if (flash1size == 1) // 25L4005 - Serial Mode
    {
      CS1_LOW; // U1 LOW

      DDRF = 0x00; // U1 Data Input

      sendSerial_U1(0x03); // Read Array
      sendSerial_U1(0x00); // Address A23-A16
      sendSerial_U1(0x00); // Address A15-A8
      sendSerial_U1(0x00); // Address A7-A0

      readSerialData_U1(0x00000, 0x80000); // 512KB

      CS1_HIGH; // U1 HIGH
    }
    else // 25L1605/25L3205/25L6405 - Parallel Mode
    {
      // Set U1 FLASH to Parallel Mode
      CS1_LOW; // U1 LOW

      sendSerial_U1(0x55); // Parallel Mode

      CS1_HIGH; // U1 HIGH
      CS1_LOW; // U1 LOW

      DDRF = 0x00; // U1 Data Input

      sendSerial_U1(0x03); // Read Array
      sendSerial_U1(0x00); // Address A23-A16
      sendSerial_U1(0x00); // Address A15-A8
      sendSerial_U1(0x00); // Address A7-A0
      readData_U1(0x000000, 0x200000); // 2MB

      if (flash1size > 2)
      {
        readData_U1(0x200000, 0x400000); // +2MB = 4MB

        if (flash1size > 4)
        {
          readData_U1(0x400000, 0x800000); // +4MB = 8MB
        }
      }

      CS1_HIGH; // U1 HIGH
    }

    if (flash2size)
    {
      // Read U2
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));

      if (flash2size == 1) // 25L4005 - Serial Mode
      {
        CS2_LOW; // U2 LOW

        DDRC = 0x00; // U2 Data Input

        sendSerial_U2(0x03); // Read Array
        sendSerial_U2(0x00); // Address A23-A16
        sendSerial_U2(0x00); // Address A15-A8
        sendSerial_U2(0x00); // Address A7-A0

        readSerialData_U2(0x00000, 0x80000); // 512KB

        CS2_HIGH; // U2 HIGH
      }
      else // 25L1605/25L3205/25L6405 - Parallel Mode
      {
        // Set U2 FLASH to Parallel Mode
        CS2_LOW; // U2 LOW

        sendSerial_U2(0x55); // Parallel Mode

        CS2_HIGH; // U2 HIGH
        CS2_LOW; // U2 LOW

        DDRC = 0x00; // U2 Data Input

        sendSerial_U2(0x03); // Read Array
        sendSerial_U2(0x00); // Address A23-A16
        sendSerial_U2(0x00); // Address A15-A8
        sendSerial_U2(0x00); // Address A7-A0
        readData_U2(0x000000, 0x200000); // 2MB

        if (flash2size > 2)
        {
          readData_U2(0x200000, 0x400000); // +2MB = 4MB

          if (flash2size > 4)
          {
            readData_U2(0x400000, 0x800000); // +4MB = 8MB
          }
        }

        CS2_HIGH; // U2 HIGH
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::Databases::Standard::matchCRC();
  }
} /* namespace OSCR::Cores::LittleJammerPro */

#endif /* HAS_LJPRO */
