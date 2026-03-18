//******************************************
// ATARI LYNX MODULE
//******************************************
#include "config.h"

#if HAS_LYNX
# include "cores/include.h"
# include "cores/AtariLynx.h"
# include "cores/Flash.h"

namespace OSCR::Cores::AtariLynx
{
  // For use with SNES-Lynx adapter
  // +----+
  // |  1 |- GND
  // |  2 |- D3
  // |  3 |- D2
  // |  4 |- D4
  // |  5 |- D1
  // |  6 |- D5
  // |  7 |- D0
  // |  8 |- D6
  // |  9 |- D7
  // | 10 |- /OE
  // | 11 |- A1
  // | 12 |- A2
  // | 13 |- A3
  // | 14 |- A6
  // | 15 |- A4
  // | 16 |- A5
  // | 17 |- A0
  // | 18 |- A7
  // | 19 |- A16
  // | 20 |- A17
  // | 21 |- A18
  // | 22 |- A19
  // | 23 |- A15
  // | 24 |- A14
  // | 25 |- A13
  // | 26 |- A12
  // | 27 |- /WE
  // | 28 |- A8
  // | 29 |- A9
  // | 30 |- A10
  // | 31 |- VCC
  // | 32 |- AUDIN
  // | 33 |- VCC
  // | 34 |- SWVCC
  // +----+
  //
  // Version 1.2
  // By @partlyhuman
  // This implementation would not be possible without the invaluable
  // documentation on
  // https://atarilynxvault.com/
  // by Igor (@theatarigamer) of K-Retro Gaming / Atari Lynx Vault
  // and the reference implementation of the Lynx Cart Programmer Pi-Hat
  // https://bitbucket.org/atarilynx/lynx/src/master/
  // by Karri Kaksonen (whitelynx.fi) and Igor as well as countless contributions
  // by the Atari Lynx community
  //

#pragma region DEFS

  #define LYNX_HEADER_SIZE 64
  #define LYNX_WE 8
  #define LYNX_OE 9
  #define LYNX_AUDIN_ON 0x80000
  #define LYNX_BLOCKADDR 2048UL
  #define LYNX_BLOCKCOUNT 256

  using OSCR::Cores::Flash::flashSize;

  // Includes \0
  constexpr char const PROGMEM LYNX[5] = "LYNX";

  // Cart information
  static bool lynxUseAudin;
  static uint16_t lynxBlockSize;

#pragma region LOWLEVEL

  void setup()
  {
    OSCR::Power::setVoltage(OSCR::Voltage::k5V);
    OSCR::Power::enableCartridge();

    // Address pins output
    // A0-7, A8-A16 (A11 doesn't exist)
    DDRF = 0xFF;
    DDRK = 0xFF;
    DDRL = 0xFF;

    // Data pins input
    DDRC = 0x00;

    // Control pins output
    // CE is tied low, not accessible
    pinMode(LYNX_WE, OUTPUT);
    pinMode(LYNX_OE, OUTPUT);
    digitalWrite(LYNX_WE, HIGH);
    digitalWrite(LYNX_OE, HIGH);

    setOutName_P(LYNX);
  }

  static void dataDir(uint8_t direction)
  {
    DDRC = (direction == OUTPUT) ? 0xFF : 0x00;
  }

  static void setAddr(uint32_t addr)
  {
    PORTF = addr & 0xFF;
    PORTK = (addr >> 8) & 0xFF;
    // AUDIN connected to L3
    PORTL = ((addr >> 16) & 0b1111);
  }

  static uint8_t readByte(uint32_t addr)
  {
    digitalWrite(LYNX_OE, HIGH);
    setAddr(addr);
    digitalWrite(LYNX_OE, LOW);
    delayMicroseconds(20);
    uint8_t data = PINC;
    digitalWrite(LYNX_OE, HIGH);
    return data;
  }

#pragma region HIGHLEVEL

  static void compareStride(uint8_t b, size_t i, size_t stride)
  {
    uint8_t other = readByte(i + stride);
    if (other == 0xFF || other == 0x00) {
      // If this is NOR flash, these in-between spaces should be formatted to all 1's
      // in which case, we DON'T report this as an unmirrored area
      // Additionally, we have encountered commercial carts where the maskrom is larger than it needs to be
      // and in this case, the maskrom will be initialized to 0s, not 1s.
      return;
    }
    if (b != other) {
      // if these bytes differ, they're likely in the same block, which means the block size contains both addresses (next POT beyond the stride)
      lynxBlockSize = max(lynxBlockSize, stride << 1);
    }
  }

  static bool detectCart()
  {
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    lynxUseAudin = false;
    lynxBlockSize = 0;

    // Somewhat arbitrary, however many bytes would be unlikely to be
    // coincidentally mirrored
    constexpr size_t const DETECT_BYTES = 128;

    for (size_t i = 0; i < DETECT_BYTES && lynxBlockSize < LYNX_BLOCKADDR; i++)
    {
      uint8_t b = readByte(i);
      // If any differences are detected when AUDIN=1, AUDIN is used to bankswitch
      // meaning we also use the maximum block size
      // (1024kb cart / 256 blocks = 4kb block bank switched between lower/upper 2kb blocks)
      if (b != readByte(i + LYNX_AUDIN_ON)) {
        lynxUseAudin = true;
        lynxBlockSize = 2048;
        break;
      }
      // Identify mirroring of largest stride
      // Valid cart sizes of 128kb, 256kb, 512kb / 256 blocks = block sizes of 512b, 1024b, 2048b
      compareStride(b, i, 256);
      compareStride(b, i, 512);
      compareStride(b, i, 1024);
    }

    if (lynxBlockSize == 0)
    {
      OSCR::UI::error(FS(OSCR::Strings::Common::FAIL));
      return false;
    }

    return true;

    //OSCR::UI::print(F("AUDIN="));
    //OSCR::UI::print((uint8_t)lynxUseAudin);
    //OSCR::UI::print(F(" BLOCK="));
    //OSCR::UI::printLineSync(lynxBlockSize);
  }

  static void writeHeader()
  {
    char header[LYNX_HEADER_SIZE] = {};
    // Magic number
    strcpy_P(header, LYNX);
    // Cart name (dummy)
    strcpy_P(header + 10, LYNX);
    // Manufacturer (dummy)
    strcpy_P(header + 42, LYNX);
    // Version
    header[8] = 1;
    // Bank 0 page size
    header[4] = lynxBlockSize & 0xFF;
    // Bank 1 page size
    header[5] = (lynxBlockSize >> 8) & 0xFF;
    // AUDIN used
    header[59] = lynxUseAudin;
    // TODO detect EEPROM?
    // header[60] = lynxUseEeprom;
    OSCR::Storage::Shared::sharedFile.write(header, LYNX_HEADER_SIZE);
  }

  static bool readROM()
  {
    if (!detectCart()) return false;

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::AtariLynx), FS(OSCR::Strings::Directory::ROM), fileName);

    writeHeader();

    dataDir(INPUT);

    // The upper part of the address is used as a block address
    // There are always 256 blocks, but the size of the block can vary
    // So outer loop always steps through block addresses
    constexpr uint32_t const upto = LYNX_BLOCKCOUNT * LYNX_BLOCKADDR;

    OSCR::UI::ProgressBar::init((uint32_t)(upto));

    for (size_t blockAddr = 0; blockAddr < upto; blockAddr += LYNX_BLOCKADDR)
    {
      for (size_t i = 0; i < lynxBlockSize; i++)
      {
        uint32_t addr = blockAddr + i;

        if (lynxUseAudin && i >= lynxBlockSize / 2)
        {
          addr += LYNX_AUDIN_ON - lynxBlockSize / 2;
        }

        uint8_t byte = readByte(addr);

        OSCR::Storage::Shared::buffer[i % 512] = byte;

        if ((i + 1) % 512 == 0)
        {
          OSCR::Storage::Shared::writeBuffer();
        }
      }

      OSCR::UI::ProgressBar::advance(LYNX_BLOCKADDR);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Databases::Basic::compareCRC(FS(OSCR::Strings::FileType::AtariLynx), 0, LYNX_HEADER_SIZE);

    OSCR::Storage::Shared::close();

    return true;
  }

#pragma region FLASH

#if HAS_FLASH

  static void writeByte(uint32_t addr, uint8_t data)
  {
    digitalWrite(LYNX_OE, HIGH);
    digitalWrite(LYNX_WE, HIGH);
    setAddr(addr);
    PORTC = data;
    digitalWrite(LYNX_WE, LOW);
    delayMicroseconds(20);
    digitalWrite(LYNX_WE, HIGH);
  }

  // Implements data complement status checking
  // We only look at D7, or the highest bit of expected
  void waitForDataComplement(uint8_t expected)
  {
    dataDir(INPUT);
    uint8_t status;
    do {
      digitalWrite(LYNX_OE, LOW);
      // one nop = 62.5ns
      // tOE = 30-50ns depending on flash
      NOP;
      status = PINC;
      digitalWrite(LYNX_OE, HIGH);
      // test highest bit
    } while ((status ^ expected) >> 7);

    dataDir(OUTPUT);
  }

  static bool readHeader()
  {
    uint32_t romSize = fileSize;

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    uint8_t header[LYNX_HEADER_SIZE];

    OSCR::Storage::Shared::sharedFile.read(header, LYNX_HEADER_SIZE);

    // Check for header to start with LYNX, assume valid .LNX header
    if (strncmp_P((char *)header, LYNX, 4) == 0)
    {
      // Pull values from header
      lynxBlockSize = (header[5] << 8) | header[4];
      lynxUseAudin = header[59];

      romSize = fileSize - LYNX_HEADER_SIZE;
    }
    else
    {
      // Header not valid, assume unheadered, rewind so we don't skip valid data
      OSCR::Storage::Shared::rewind();

      // Get block size from file size
      lynxBlockSize = fileSize / LYNX_BLOCKCOUNT;
      lynxUseAudin = lynxBlockSize >= 2048;
      romSize = fileSize;
    }

    //OSCR::UI::print(F("AUDIN="));
    //OSCR::UI::print(lynxUseAudin ? 1 : 0);
    //OSCR::UI::print(F(" BLOCK="));
    //OSCR::UI::printLine(lynxBlockSize);

    //OSCR::UI::print(FS(OSCR::Strings::Labels::ROM_SIZE));
    //OSCR::Lang::printBytesLine(romSize);

    // Ensure valid block size, file size, and file fits in flash
    uint32_t expectedSize = (uint32_t)LYNX_BLOCKCOUNT * lynxBlockSize;

    if (lynxBlockSize % 256 != 0 || lynxBlockSize > LYNX_BLOCKADDR || lynxBlockSize <= 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::UI::error(FS(OSCR::Strings::Errors::InvalidType));
      OSCR::UI::waitButton();
      return false;
    }

    if (romSize != expectedSize)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::UI::error(FS(OSCR::Strings::Errors::InvalidType));
      return false;
    }

    if (expectedSize > flashSize)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::UI::error(FS(OSCR::Strings::Errors::NotLargeEnough));
      return false;
    }

    OSCR::UI::clearLine();
    OSCR::UI::update();

    return true;
  }

  static bool detectFlash()
  {
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    // SOFTWARE ID PROGRAM
    dataDir(OUTPUT);
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0x90);
    dataDir(INPUT);
    // tIDA = 150ns
    NOP;
    NOP;
    NOP;
    // MFG,DEVICE
    uint16_t deviceId = (readByte(0x0) << 8) | readByte(0x1);

    // EXIT SOFTWARE ID PROGRAM
    dataDir(OUTPUT);
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0xF0);

    flashSize = 0;
    switch (deviceId) {
      case 0xBFB5:
        // SST39SF010 = 1Mbit
        flashSize = 131072UL;
        break;
      case 0xBFB6:
        // SST39SF020 = 2Mbit
        flashSize = 262144UL;
        break;
      case 0xBFB7:
        // SST39SF040 = 4Mbit
        flashSize = 524288UL;
        break;
        // case 0xC2A4:
        //   // MX29F040 = 4Mbit
        //   flashSize = 524288UL;
        //   break;
        // case 0xC2D5:
        //   // MX29F080 = 8Mbit
        //   flashSize = 1048576UL;
        //   break;
    }

    if (flashSize <= 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
      return false;
    }

    //OSCR::Lang::printBytesLine(flashSize);

    OSCR::UI::clearLine();
    OSCR::UI::update();

    return true;
  }

  static void eraseFlash()
  {
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    // CHIP ERASE PROGRAM
    dataDir(OUTPUT);
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0x80);
    writeByte(0x5555, 0xAA);
    writeByte(0x2AAA, 0x55);
    writeByte(0x5555, 0x10);
    waitForDataComplement(0xFF);

    OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));
  }

  static bool writeROM()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    if (!detectFlash()) return false;
    if (!readHeader()) return false;

    // Pause to read debug info
    // OSCR::UI::wait();
    // or alternately auto-advance
    delay(2000);
    OSCR::UI::clear();

    eraseFlash();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

    dataDir(OUTPUT);

    constexpr uint32_t const upto = LYNX_BLOCKCOUNT * LYNX_BLOCKADDR;
    uint8_t block[lynxBlockSize];

    OSCR::UI::ProgressBar::init((uint32_t)(upto));

    for (size_t blockAddr = 0; blockAddr < upto; blockAddr += LYNX_BLOCKADDR)
    {
      OSCR::Storage::Shared::sharedFile.read(block, lynxBlockSize);

      for (size_t i = 0; i < lynxBlockSize; i++)
      {
        uint32_t addr = blockAddr + i;

        if (lynxUseAudin && i >= lynxBlockSize / 2)
        {
          addr += LYNX_AUDIN_ON - lynxBlockSize / 2;
        }

        // BYTE PROGRAM
        uint8_t b = block[i];
        writeByte(0x5555, 0xAA);
        writeByte(0x2AAA, 0x55);
        writeByte(0x5555, 0xA0);
        writeByte(addr, b);

        waitForDataComplement(b);
      }

      OSCR::UI::ProgressBar::advance(LYNX_BLOCKADDR);
    }

    OSCR::UI::ProgressBar::finish();

    OSCR::Storage::Shared::close();

    dataDir(INPUT);

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    return true;
  }
#endif

#pragma region MENU

  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::ReadROM,
#if HAS_FLASH
    OSCR::Strings::MenuOptions::WriteFlash,
#endif
    OSCR::Strings::MenuOptions::Back,
  };

  void menu()
  {
    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::AtariLynx), menuOptions, sizeofarray(menuOptions)))
      {
      case 0:
        if (!readROM()) continue;
        break;

  #if HAS_FLASH
      case 1:
        if (!writeROM()) continue;
        break;
  #endif

      default:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }
} /* namespace OSCR::Cores::AtariLynx */

#endif /* HAS_LYNX */
