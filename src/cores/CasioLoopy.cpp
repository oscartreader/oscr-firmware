//******************************************
// CASIO LOOPY MODULE
//******************************************
#include "config.h"

#if HAS_LOOPY
# include "cores/include.h"
# include "cores/CasioLoopy.h"

namespace OSCR::Cores::CasioLoopy
{
  // Casio Loopy
  // Cartridge Pinout
  // 90P 2.1mm pitch connector
  //
  //         +--------+
  //    +5V -|  1  90 |- D11
  //        -|  2  89 |- +5V
  //        -|  3  88 |- D9
  //        -|  4  87 |-
  //        -|  5  86 |- D7
  //        -|  6  85 |- D5
  //        -|  7  84 |- D15
  //        -|  8  83 |- D13
  // RAMCS1 -|  9  82 |- D12
  //        -| 10  81 |- D1
  //  RAMWE -| 11  80 |-
  //        -| 12  79 |-
  //    GND -| 13  78 |- A2
  //    +5V -| 14  77 |- A4
  //        -| 15  76 |- A19
  //        -| 16  75 |- GND
  //        -| 17  74 |- A18
  //        -| 18  73 |- A16
  //        -| 19  72 |- A17
  //        -| 20  71 |- A14
  //        -| 21  70 |- A5
  //    A12 -| 22  69 |- A7
  //    A10 -| 23  68 |- A9
  //     A8 -| 24  67 |- A11
  //     A6 -| 25  66 |-
  //    A13 -| 26  65 |-
  //    A15 -| 27  64 |-
  //    A20 -| 28  63 |-
  //    +5V -| 29  62 |-
  //     A3 -| 30  61 |-
  //    A21 -| 31  60 |- CLK
  //     A1 -| 32  59 |-
  //     A0 -| 33  58 |-
  //        -| 34  57 |- +5V
  //     D0 -| 35  56 |- OE
  //  RESET -| 36  55 |-
  //     D2 -| 37  54 |- ROMCE
  //     D3 -| 38  53 |-
  //    D14 -| 39  52 |-
  //     D4 -| 40  51 |-
  //     D6 -| 41  50 |-
  //     D8 -| 42  49 |-
  //        -| 43  48 |-
  //    D10 -| 44  47 |- GND
  //    GND -| 45  46 |-
  //         +--------+
  //
  // * Blank pins have various uses depending on cartridge but are not necessary for dumping.
  // IMPORTANT: All data are stored as BIG-ENDIAN. Many ROM dumps online are little endian.
  // See https://github.com/kasamikona/Loopy-Tools/blob/master/Documentation/ROM%20Structure.md
  //
  // By @partlyhuman
  // Special thanks to @kasamikona

  // SH-1 memory map locations, ROM starts here
  constexpr uint32_t const LOOPY_MAP_ROM_ZERO = 0x0E000000;
  constexpr uint32_t const LOOPY_MAP_SRAM_ZERO = 0x02000000;

  // Control pins
  constexpr int const LOOPY_ROMCE = 42;
  constexpr int const LOOPY_OE = 43;
  constexpr int const LOOPY_RAMWE = 6;
  constexpr int const LOOPY_RAMCS1 = 7;
  constexpr int const LOOPY_RESET = A7;

  // The internal checksum read from the cart header at 08h, will be checked against an actual sum
  uint32_t loopyChecksum;
  uint32_t loopyChecksumStart;
  uint32_t loopyChecksumEnd;

  char loopyRomNameLong[64];

  //******************************************
  //  MENU
  //******************************************

  // Base Menu
  constexpr char const loopyMenuItem4[] PROGMEM = "Format SRAM";
  constexpr char const * const menuOptions[] PROGMEM = {
    OSCR::Strings::MenuOptions::RefreshCart,
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    loopyMenuItem4,
    OSCR::Strings::MenuOptions::Back,
  };

  void refreshCart()
  {
    // Set control
    dataIn();

    // First word after header stored as 32-bit pointer at 0h, final word (inclusive) at 4h
    // based on SH-1 memory mapped location of ROM (shift to rebase on zero)
    loopyChecksumStart = (((uint32_t)readWord(0x0) << 16) | (uint32_t)readWord(0x2)) - LOOPY_MAP_ROM_ZERO;
    loopyChecksumEnd = (((uint32_t)readWord(0x4) << 16) | (uint32_t)readWord(0x6)) - LOOPY_MAP_ROM_ZERO;

    // Full cart size DOES include the header, don't subtract it off :)
    cartSize = loopyChecksumEnd + 2;

    // SRAM first and last byte locations stored at 10h and 14h, based on SH-1 memory mapped location of SRAM
    uint32_t loopySramStart = (((uint32_t)readWord(0x10) << 16) | (uint32_t)readWord(0x12)) - LOOPY_MAP_SRAM_ZERO;
    uint32_t loopySramEnd = (((uint32_t)readWord(0x14) << 16) | (uint32_t)readWord(0x16)) - LOOPY_MAP_SRAM_ZERO;

    sramSize = loopySramEnd - loopySramStart + 1;

    // TODO sanity check these values

    // Get internal checksum from header
    loopyChecksum = ((uint32_t)readWord(0x8) << 16) | (uint32_t)readWord(0xA);

    // The loopy's DB stores the checksum from the header, so to get the name
    //  we cast it as a CRC32, even though it isn't one, because making a new
    //  CRDB just for this would be a waste.
    crc32sum = loopyChecksum;
    fromCRDB = OSCR::Databases::Basic::searchDatabase(&crc32sum);

    if (!fromCRDB)
    {
      useDefaultName();
    }

    printHeader();

    if (fromCRDB)
    {
      OSCR::UI::printValue(OSCR::Strings::Common::Name, fileName);
    }
    else
    {
      useDefaultName();
      OSCR::UI::printValue_P(OSCR::Strings::Common::Name, OSCR::Strings::Common::Unknown);
    }

    OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);
    OSCR::UI::printLine(crc32sum);

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, cartSize * 8);

    OSCR::UI::printSize(OSCR::Strings::Common::RAM, sramSize * 8);
  }

  void menu()
  {
    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::CasioLoopy), menuOptions, sizeofarray(menuOptions)))
      {
      case 0:
        setup();
        return;

      case 1: // Read ROM
        readROM();
        break;

      case 2: // Read SRAM
        OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Reading));
        readSRAM();
        break;

      case 3: // Write SRAM
        OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_READ);

        writeSRAM();
        break;

      case 4: // Format SRAM
        formatSRAM();
        break;

      case 5: // Back
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::CasioLoopy));
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
    // PK1-PK7, PA1-PA7, PC0-PC3, PL0-PL3
    // Take whole port and unset the exceptions later
    //DDRK = DDRA = DDRC = DDRL = 0xFF; // disableCartridge() already did this

    // Control pins, all active low
    pinMode(LOOPY_ROMCE, OUTPUT);
    pinMode(LOOPY_OE, OUTPUT);
    pinMode(LOOPY_RAMWE, OUTPUT);
    pinMode(LOOPY_RAMCS1, OUTPUT);
    pinMode(LOOPY_RESET, OUTPUT);
    digitalWrite(LOOPY_ROMCE, HIGH);
    digitalWrite(LOOPY_OE, HIGH);
    digitalWrite(LOOPY_RAMWE, HIGH);
    digitalWrite(LOOPY_RAMCS1, HIGH);
    digitalWrite(LOOPY_RESET, HIGH);

    // Set Pins (D0-D15) to Input
    dataIn();
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Basic::setup(FS(OSCR::Strings::FileType::CasioLoopy));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  //******************************************
  //  LOW LEVEL FUNCTIONS
  //******************************************

  void setAddress(uint32_t A)
  {
    // PK1  A0
    // PK2  A1
    // PK3  A21
    // PK4  A3
    // PK5  A20
    // PK6  A15
    // PK7  A13
    PORTK = (bitRead(A, 0) << 1)
            | (bitRead(A, 1) << 2)
            | (bitRead(A, 21) << 3)
            | (bitRead(A, 3) << 4)
            | (bitRead(A, 20) << 5)
            | (bitRead(A, 15) << 6)
            | (bitRead(A, 13) << 7);
    // PA1  A2
    // PA2  A4
    // PA3  A19
    // PA4  A18
    // PA5  A16
    // PA6  A17
    // PA7  A14
    PORTA = (bitRead(A, 2) << 1)
            | (bitRead(A, 4) << 2)
            | (bitRead(A, 19) << 3)
            | (bitRead(A, 18) << 4)
            | (bitRead(A, 16) << 5)
            | (bitRead(A, 17) << 6)
            | (bitRead(A, 14) << 7);
    // PC0  A6
    // PC1  A8
    // PC2  A10
    // PC3  A12
    PORTC = (bitRead(A, 6))
            | (bitRead(A, 8) << 1)
            | (bitRead(A, 10) << 2)
            | (bitRead(A, 12) << 3);
    // CAUTION PORTL is shared, writing to PORTL indiscriminately will mess with CE/OE
    // D42	PL7	CE
    // D43	PL6	OE
    // D44	PL5
    // D45	PL4
    // D46	PL3	A11
    // D47	PL2	A9
    // D48	PL1	A7
    // D49	PL0	A5
    digitalWrite(46, bitRead(A, 11));
    digitalWrite(47, bitRead(A, 9));
    digitalWrite(48, bitRead(A, 7));
    digitalWrite(49, bitRead(A, 5));
    // PORTL = (bitRead(A, 5))
    //         | (bitRead(A, 7) << 1)
    //         | (bitRead(A, 9) << 2)
    //         | (bitRead(A, 11) << 3);
  }

  uint16_t getWord()
  {
    // A8   PK0 D0
    // D22  PA0 D1
    // A6   PF6 D2
    // A5   PF5 D3
    // A3   PF3 D4
    // D40  PG1 D5
    // A2   PF2 D6
    // D41  PG0 D7
    // A1   PF1 D8
    // D3   PE5 D9
    // A0   PF0 D10
    // D2   PE4 D11
    // D14  PJ1 D12
    // D15  PJ0 D13
    // A4   PF4 D14
    // D4   PG5 D15
    return bitRead(PINK, 0)
          | (bitRead(PINA, 0) << 1)
          | (bitRead(PINF, 6) << 2)
          | (bitRead(PINF, 5) << 3)
          | (bitRead(PINF, 3) << 4)
          | (bitRead(PING, 1) << 5)
          | (bitRead(PINF, 2) << 6)
          | (bitRead(PING, 0) << 7)
          | (bitRead(PINF, 1) << 8)
          | (bitRead(PINE, 5) << 9)
          | (bitRead(PINF, 0) << 10)
          | (bitRead(PINE, 4) << 11)
          | (bitRead(PINJ, 1) << 12)
          | (bitRead(PINJ, 0) << 13)
          | (bitRead(PINF, 4) << 14)
          | (bitRead(PING, 5) << 15);
  }

  uint8_t getByte()
  {
    return bitRead(PINK, 0)
          | (bitRead(PINA, 0) << 1)
          | (bitRead(PINF, 6) << 2)
          | (bitRead(PINF, 5) << 3)
          | (bitRead(PINF, 3) << 4)
          | (bitRead(PING, 1) << 5)
          | (bitRead(PINF, 2) << 6)
          | (bitRead(PING, 0) << 7);
  }

  void setByte(uint8_t D)
  {
    // Since D lines are spread among so many ports, this is far more legible, and only used for SRAM
    digitalWrite(A8, bitRead(D, 0));
    digitalWrite(22, bitRead(D, 1));
    digitalWrite(A6, bitRead(D, 2));
    digitalWrite(A5, bitRead(D, 3));
    digitalWrite(A3, bitRead(D, 4));
    digitalWrite(40, bitRead(D, 5));
    digitalWrite(A2, bitRead(D, 6));
    digitalWrite(41, bitRead(D, 7));
  }

  uint8_t readByte(uint32_t myAddress)
  {
    setAddress(myAddress);

    // 100ns MAX
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    return getByte();
  }

  void writeByte(uint32_t myAddress, uint8_t myData)
  {
    setAddress(myAddress);

    digitalWrite(LOOPY_RAMWE, LOW);

    // tWHZ 35
    NOP;
    NOP;
    dataOut();

    setByte(myData);

    // tWP 60
    NOP;
    NOP;
    NOP;
    NOP;

    digitalWrite(LOOPY_RAMWE, HIGH);
    dataIn();
  }

  uint16_t readWord(uint32_t myAddress)
  {
    setAddress(myAddress);

    digitalWrite(LOOPY_ROMCE, LOW);
    digitalWrite(LOOPY_OE, LOW);

    // 16mhz = 62.5ns
    NOP;
    NOP;

    uint16_t tempWord = getWord();

    digitalWrite(LOOPY_ROMCE, HIGH);
    digitalWrite(LOOPY_OE, HIGH);

    return tempWord;
  }

  // Switch data pins to write
  void dataOut()
  {
    // // PA0
    // DDRA |= 0x01;
    // // PK0
    // DDRK |= 0x01;
    // // PG0, PG1, PG5 (rest unused?)
    // DDRG = 0xFF;
    // // PJ0-1 (rest unused?)
    // DDRJ = 0xFF;
    // // PE4-PE5 (rest unused?)
    // DDRE = 0xFF;
    // // PF0-PF6
    // DDRF |= 0b0111111;

    // Only bothering to change lower bits since we never write words just bytes
    pinMode(A8, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(A6, OUTPUT);
    pinMode(A5, OUTPUT);
    pinMode(A3, OUTPUT);
    pinMode(40, OUTPUT);
    pinMode(A2, OUTPUT);
    pinMode(41, OUTPUT);
    // pinMode(A1, OUTPUT);
    // pinMode(3, OUTPUT);
    // pinMode(A0, OUTPUT);
    // pinMode(2, OUTPUT);
    // pinMode(14, OUTPUT);
    // pinMode(15, OUTPUT);
    // pinMode(A4, OUTPUT);
    // pinMode(4, OUTPUT);
  }

  // Switch data pins to read
  void dataIn()
  {
    // // PA0
    // DDRA &= ~0x01;
    // // PK0
    // DDRK &= ~0x01;
    // // PG0, PG1, PG5 (rest unused?)
    // DDRG = 0x00;
    // // PJ0-1 (rest unused?)
    // DDRJ = 0x00;
    // // PE4-PE5 (rest unused?)
    // DDRE = 0x00;
    // // PF0-PF6
    // DDRF &= ~0b0111111;
    pinMode(A8, INPUT);
    pinMode(22, INPUT);
    pinMode(A6, INPUT);
    pinMode(A5, INPUT);
    pinMode(A3, INPUT);
    pinMode(40, INPUT);
    pinMode(A2, INPUT);
    pinMode(41, INPUT);
    pinMode(A1, INPUT);
    pinMode(3, INPUT);
    pinMode(A0, INPUT);
    pinMode(2, INPUT);
    pinMode(14, INPUT);
    pinMode(15, INPUT);
    pinMode(A4, INPUT);
    pinMode(4, INPUT);
  }

  //******************************************
  // READ CODE
  //******************************************

  void readROM()
  {
    dataIn();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::CasioLoopy), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::Raw));

    OSCR::UI::ProgressBar::init((uint32_t)(cartSize));

    constexpr size_t const sdBufferSize = 512;
    uint32_t sum = 0;

    digitalWrite(LOOPY_ROMCE, LOW);

    for (uint32_t ptr = 0; ptr < cartSize;)
    {
      uint16_t myWord = readWord(ptr);

      // aggregate checksum over 16-bit words, starting at 80h, this address is stored in header but never varies
      if (ptr >= loopyChecksumStart)
      {
        sum += myWord;
      }

      // Store in buffer
      OSCR::Storage::Shared::buffer[ptr++ % sdBufferSize] = (myWord >> 8) & 0xFF;
      OSCR::Storage::Shared::buffer[ptr++ % sdBufferSize] = myWord & 0xFF;

      // Flush when buffer full
      if (ptr % sdBufferSize == 0)
      {
        OSCR::Storage::Shared::writeBuffer();
      }

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::ProgressBar::finish();

    // TODO this assumes size is divisible by 512
    OSCR::Storage::Shared::close();

    digitalWrite(LOOPY_ROMCE, HIGH);

    // Instead of the CRC32, check the internal integrity based on the header checksum
    OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);
    OSCR::UI::printHex(sum);

    if (sum == loopyChecksum)
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::Arrow));
      OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    }
    else
    {
      OSCR::UI::print(FS(OSCR::Strings::Symbol::NotEqual));
      OSCR::UI::printHexLine(loopyChecksum);
    }
  }

  //******************************************
  // SRAM
  //******************************************

  void writeSRAM()
  {
    cartOn();

    // Being nice to the SRAM and not touching the data bus except when WE is LOW
    dataIn();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    digitalWrite(LOOPY_ROMCE, HIGH);
    digitalWrite(LOOPY_RAMCS1, LOW);
    digitalWrite(LOOPY_RESET, HIGH);
    digitalWrite(LOOPY_OE, LOW);

    for (uint32_t currByte = 0; currByte < sramSize; currByte++)
    {
      writeByte(currByte, OSCR::Storage::Shared::sharedFile.read());
    }

    OSCR::Storage::Shared::close();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Common::DONE));

    digitalWrite(LOOPY_RAMCS1, HIGH);
    digitalWrite(LOOPY_OE, HIGH);

    dataIn();

    writeErrors = 0;

    digitalWrite(LOOPY_ROMCE, HIGH);
    digitalWrite(LOOPY_RAMCS1, LOW);
    digitalWrite(LOOPY_OE, LOW);

    for (uint32_t currBuffer = 0; currBuffer < sramSize; currBuffer += 512)
    {
      for (int currByte = 0; currByte < 512; currByte++)
      {
        OSCR::Storage::Shared::buffer[currByte] = readByte(currBuffer + currByte);
      }

      for (int i = 0; i < 512; i++)
      {
        if (OSCR::Storage::Shared::sharedFile.read() != OSCR::Storage::Shared::buffer[i])
        {
          writeErrors++;
        }
      }
    }

    OSCR::Storage::Shared::close();

    digitalWrite(LOOPY_OE, HIGH);
    digitalWrite(LOOPY_RAMCS1, HIGH);

    if (writeErrors != 0)
    {
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }
  }

  void formatSRAM()
  {
    printHeader();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    dataIn();
    digitalWrite(LOOPY_ROMCE, HIGH);
    digitalWrite(LOOPY_RAMCS1, LOW);
    digitalWrite(LOOPY_RESET, HIGH);
    digitalWrite(LOOPY_OE, LOW);

    for (uint32_t currByte = 0; currByte < sramSize; currByte++)
    {
      writeByte(currByte, 0);
    }

    digitalWrite(LOOPY_RAMCS1, HIGH);
    digitalWrite(LOOPY_OE, HIGH);
    dataIn();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  void readSRAM()
  {
    dataIn();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::CasioLoopy), FS(OSCR::Strings::Directory::Save), fileName, FS(OSCR::Strings::FileType::Save));

    digitalWrite(LOOPY_ROMCE, HIGH);
    digitalWrite(LOOPY_RAMCS1, LOW);
    digitalWrite(LOOPY_OE, LOW);

    constexpr size_t const sdBufferSize = 512;

    for (uint32_t ptr = 0; ptr < sramSize;)
    {
      uint8_t b = readByte(ptr);
      OSCR::Storage::Shared::buffer[ptr++ % sdBufferSize] = b;

      if (ptr % sdBufferSize == 0)
      {
        OSCR::Storage::Shared::writeBuffer();
      }
    }

    digitalWrite(LOOPY_OE, HIGH);
    digitalWrite(LOOPY_RAMCS1, HIGH);

    OSCR::Storage::Shared::close();
  }
} /* namespace OSCR::Cores::CasioLoopy */

#endif /* HAS_LOOPY */
