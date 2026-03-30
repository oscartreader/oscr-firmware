//******************************************
// NINTENDO 64 MODULE
//******************************************
#include "config.h"

#if defined(ENABLE_N64)
# include "cores/include.h"
# include "cores/N64.h"
# include "cores/Flash.h"

namespace OSCR::Cores::N64
{
  /******************************************
    Defines
  *****************************************/
  // These two macros toggle the eepDataPin/ControllerDataPin between input and output
  // External 1K pull-up resistor from eepDataPin to VCC required
  // 0x10 = 00010000 -> Port H Pin 4
# define N64_HIGH DDRH &= ~0x10
# define N64_LOW DDRH |= 0x10
  // Read the current state(0/1) of the eepDataPin
# define N64_QUERY (PINH & 0x10)

  /******************************************
     Variables
  *****************************************/
  using OSCR::Databases::Extended::romDetail;
  using OSCR::Databases::Extended::romRecord;

  using OSCR::Cores::Flash::flashid;
  using OSCR::Cores::Flash::flashSize;

  // Received N64 Eeprom data bits, 1 page
  uint16_t eepPages;

  // N64 Controller
  struct {
    char stick_x;
    char stick_y;
  } N64_status;
  //stings that hold the buttons
  String button = "N/A";
  String lastbutton = "N/A";

  // Rom base address
  uint32_t romBase = 0x10000000;

  // Flashram type
  uint8_t flashramType = 1;
  bool MN63F81MPN = false;

  //ControllerTest
  bool quit = 1;

  crc32_t crc1;

  constexpr uint16_t const kBufferSize8 = OSCR::Storage::kBufferSize / 8;
  constexpr uint32_t const kSaveAddress = 0x08000000;

  /******************************************
    Menu
  *****************************************/

  enum class MenuOption : uint8_t
  {
    Cart,
    Controller,
# if HAS_FLASH
    FlashRepro,
# endif /* HAS_FLASH */
    FlashGameshark,
    FlashXplorer64,
    Back,
  };

  enum class MenuOptionCart : uint8_t
  {
    ReadROM,
    ReadSave,
    WriteSave,
    SetSaveType,
    RefreshCart,
    Back,
  };

  // N64 start menu
  constexpr char const PROGMEM menuOptionFlashGameshark[]   = "Flash Gameshark";
  constexpr char const PROGMEM menuOptionFlashXplorer64[]   = "Flash Xplorer 64";

  constexpr char const * const PROGMEM menuOptions[] = {
    OSCR::Strings::MenuOptions::Cartridge,
    OSCR::Strings::MenuOptions::Controller,
# if HAS_FLASH
    OSCR::Strings::MenuOptions::WriteFlash,
# endif /* HAS_FLASH */
    menuOptionFlashGameshark,
    menuOptionFlashXplorer64,
    OSCR::Strings::MenuOptions::Back,
  };

  // N64 controller menu items
  constexpr char const PROGMEM menuOptionControllerTest[] = "Test";

  constexpr char const * const PROGMEM menuOptionsController[] = {
    OSCR::Strings::MenuOptions::Read,
    OSCR::Strings::MenuOptions::Write,
# if defined(ENABLE_CONTROLLERTEST)
    menuOptionControllerTest,
# endif
    OSCR::Strings::MenuOptions::Back,
  };

  // N64 cart menu items
  constexpr char const * const PROGMEM menuOptionsCart[] = {
    OSCR::Strings::MenuOptions::ReadROM,
    OSCR::Strings::MenuOptions::ReadSave,
    OSCR::Strings::MenuOptions::WriteSave,
    OSCR::Strings::MenuOptions::SetSaveType,
    OSCR::Strings::MenuOptions::RefreshCart,
    OSCR::Strings::MenuOptions::Back,
  };

  // Rom menu
  constexpr uint8_t menuOptionsROM[] = {
    4,
    8,
    12,
    16,
    32,
    64,
    128,
  };

  // Save menu
  constexpr char const PROGMEM N64SaveItem2[] = "4K EEPROM";
  constexpr char const PROGMEM N64SaveItem3[] = "16K EEPROM";
  constexpr char const PROGMEM N64SaveItem4[] = "SRAM";
  constexpr char const PROGMEM N64SaveItem5[] = "FLASH";

  constexpr char const * const PROGMEM saveOptionsN64[] = {
    OSCR::Strings::Common::None,
    N64SaveItem2,
    N64SaveItem3,
    N64SaveItem4,
    N64SaveItem5,
  };

  void printHeader()
  {
    OSCR::UI::printHeader(FS(OSCR::Strings::Cores::N64));
  }

  // N64 start menu
  void menu()
  {
    do
    {
      switch (static_cast<MenuOption>(OSCR::UI::menu(FS(OSCR::Strings::Cores::N64), menuOptions, sizeofarray(menuOptions))))
      {
      case MenuOption::Cart:
        cartMenu();
        continue;

      case MenuOption::Controller:
        controllerMenu();
        continue;

# if HAS_FLASH
      case MenuOption::FlashRepro:
        flashRepro();
        if (!refreshCart()) continue;
        cartMenu();
        continue;
# endif

      case MenuOption::FlashGameshark:
        if (!flashGameshark()) continue;
        if (!refreshCart()) continue;
        break;

      case MenuOption::FlashXplorer64:
        flashXplorer();
        continue;

      case MenuOption::Back:
        closeCRDB();
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  // N64 Controller Menu
  void controllerMenu()
  {
    setupController();

    do
    {
      switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::N64), menuOptionsController, sizeofarray(menuOptionsController)))
      {
      case 0:
        if (!readMPK()) continue;
        break;

      case 1:
        if (!writeMPK()) continue;
        break;

# if defined(ENABLE_CONTROLLERTEST)
      case 2:
        resetController();

        printHeader();

#   if (defined(ENABLE_OLED) || (HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864))
        controllerTest_Display();
#   elif defined(ENABLE_SERIAL)
        controllerTest_Serial();
#   endif /* ENABLE_OLED || ENABLE_LCD || ENABLE_SERIAL */

        quit = 1;

        break;
# endif /* ENABLE_CONTROLLERTEST */

      default:
        return;
      }

      OSCR::UI::waitButton();
    }
    while(true);
  }

  // N64 Cartridge Menu
  void cartMenu()
  {
    openCRDB();

    if (!refreshCart())
    {
      closeCRDB();
      return;
    }

    OSCR::UI::waitButton();

    do
    {
      switch (static_cast<MenuOptionCart>(OSCR::UI::menu(FS(OSCR::Strings::Cores::N64), menuOptionsCart, sizeofarray(menuOptionsCart))))
      {
      case MenuOptionCart::ReadROM:
        readRom();
        break;

      case MenuOptionCart::ReadSave:
        readSave();
        break;

      case MenuOptionCart::WriteSave:
        if (!writeSave()) continue;
        break;

      case MenuOptionCart::SetSaveType:
        menuSaveType();
        continue;


      case MenuOptionCart::RefreshCart:
        if (!refreshCart()) continue;
        break;

      case MenuOptionCart::Back:
        return;
      }

      OSCR::UI::waitButton();
    }
    while (true);
  }

  void menuSaveType()
  {
    switch (OSCR::UI::menu(FS(OSCR::Strings::Cores::N64), saveOptionsN64, sizeofarray(saveOptionsN64)))
    {
    case 0: // None
      saveType = 0;
      break;

    case 1: // 4K EEPROM
      saveType = 5;
      eepPages = 64;
      break;

    case 2: // 16K EEPROM
      saveType = 6;
      eepPages = 256;
      break;

    case 3: // SRAM
      saveType = 1;
      break;

    case 4: // FLASHRAM
      saveType = 4;
      break;
    }
  }

  /******************************************
     Setup
  *****************************************/
  void setupController()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    // Output a low signal
    PORTH &= ~(1 << 4);
    // Set Controller Data Pin(PH4) to Input
    DDRH &= ~(1 << 4);
  }

  void cartOn()
  {
    // Request 3.3V
    OSCR::Power::setVoltage(OSCR::Voltage::k3V3);
    OSCR::Power::enableCartridge();

    // Set Address Pins to Output and set them low
    //A0-A7
    DDRF = 0xFF;
    PORTF = 0x00;
    //A8-A15
    DDRK = 0xFF;
    PORTK = 0x00;

    // Set Control Pins to Output RESET(PH0) WR(PH5) RD(PH6) aleL(PC0) aleH(PC1)
    DDRH |= (1 << 0) | (1 << 5) | (1 << 6);
    DDRC |= (1 << 0) | (1 << 1);
    // Pull RESET(PH0) low until we are ready
    PORTH &= ~(1 << 0);
    // Output a high signal on WR(PH5) RD(PH6), pins are active low therefore everything is disabled now
    PORTH |= (1 << 5) | (1 << 6);
    // Pull aleL(PC0) low and aleH(PC1) high
    PORTC &= ~(1 << 0);
    PORTC |= (1 << 1);

# ifdef ENABLE_CLOCKGEN
    // Set Eeprom clock to 2Mhz
    OSCR::ClockGen::clockgen.set_freq(200000000ULL, SI5351_CLK1);

    // Start outputting Eeprom clock
    OSCR::ClockGen::clockgen.output_enable(SI5351_CLK1, 1);  // Eeprom clock
# else /* !ENABLE_CLOCKGEN */
    // Set Eeprom Clock Pin(PH1) to Output
    DDRH |= (1 << 1);
    // Output a high signal
    PORTH |= (1 << 1);
# endif /* ENABLE_CLOCKGEN */

    // Set Eeprom Data Pin(PH4) to Input
    DDRH &= ~(1 << 4);
    // Activate Internal Pullup Resistors
    //PORTH |= (1 << 4);

# ifdef ENABLE_CLOCKGEN
    // Wait for clock generator
    OSCR::ClockGen::clockgen.update_status();
# endif /* ENABLE_CLOCKGEN */

    // Wait until all is stable
    delay(300);

    // Pull RESET(PH0) high to start eeprom
    PORTH |= (1 << 0);
  }

  void cartOff()
  {
    OSCR::Power::disableCartridge();
  }

  void openCRDB()
  {
    OSCR::Databases::Extended::setup(FS(OSCR::Strings::FileType::N64F));
  }

  void closeCRDB()
  {
    resetGlobals();
  }

  /******************************************
     Low level functions
  *****************************************/
  static uint16_t addrCRC(uint16_t address);
  static uint8_t dataCRC(uint8_t* data);

  // Switch Cartridge address/data pins to write
#if ((OPTION_PERFORMANCE_FLAGS) & (PRFOPT_FAST64))
  inline __attribute__((always_inline, hot))
#else
  __hot
#endif
  void adOut()
  {
    //A0-A7
    DDRF = 0xFF;
    PORTF = 0x00;
    //A8-A15
    DDRK = 0xFF;
    PORTK = 0x00;
  }

  // Switch Cartridge address/data pins to read
#if ((OPTION_PERFORMANCE_FLAGS) & (PRFOPT_FAST64))
  inline __attribute__((always_inline, hot))
#else
  __hot
#endif
  void adIn()
  {
    //A0-A7
    DDRF = 0x00;
    //A8-A15
    DDRK = 0x00;
    //Enable internal pull-up resistors
    //PORTF = 0xFF;
    //PORTK = 0xFF;
  }

  // Set Cartridge address
#if ((OPTION_PERFORMANCE_FLAGS) & (PRFOPT_FAST64))
  inline __attribute__((always_inline, hot))
#else
  __hot
#endif
  void setAddress(uint32_t myAddress)
  {
    // Set address pins to output
    adOut();

    // Split address into two words
    uint16_t myAdrLowOut = myAddress & 0xFFFF;
    uint16_t myAdrHighOut = myAddress >> 16;

    // Switch WR(PH5) RD(PH6) ale_L(PC0) ale_H(PC1) to high (since the pins are active low)
    PORTH |= (1 << 5) | (1 << 6);
    PORTC |= (1 << 1);
    __asm__("nop\n\t"); // needed for repro
    PORTC |= (1 << 0);

    // Output high part to address pins
    PORTF = myAdrHighOut & 0xFF;
    PORTK = (myAdrHighOut >> 8) & 0xFF;

    // Leave ale_H high for additional 62.5ns
    __asm__("nop\n\t");

    // Pull ale_H(PC1) low
    PORTC &= ~(1 << 1);

    // Output low part to address pins
    PORTF = myAdrLowOut & 0xFF;
    PORTK = (myAdrLowOut >> 8) & 0xFF;

    // Leave ale_L high for ~125ns
    __asm__("nop\n\t"
            "nop\n\t");

    // Pull ale_L(PC0) low
    PORTC &= ~(1 << 0);

    // Set data pins to input
    adIn();
  }

  // Read one word out of the cartridge
  uint16_t readWord()
  {
    // Pull read(PH6) low
    PORTH &= ~(1 << 6);

    // Wait ~310ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Join bytes from PINF and PINK into a word
    uint16_t tempWord = ((PINK & 0xFF) << 8) | (PINF & 0xFF);

    // Pull read(PH6) high
    PORTH |= (1 << 6);

    return tempWord;
  }

  // Write one word to data pins of the cartridge
  void writeWord(uint16_t myWord)
  {
    // Set address pins to output
    adOut();

    // Output word to AD0-AD15
    PORTF = myWord & 0xFF;
    PORTK = (myWord >> 8) & 0xFF;

    // Wait ~62.5ns
    __asm__("nop\n\t");

    // Pull write(PH5) low
    PORTH &= ~(1 << 5);

    // Wait ~310ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Pull write(PH5) high
    PORTH |= (1 << 5);

    // Wait ~125ns
    __asm__("nop\n\t"
            "nop\n\t");

    // Set data pins to input
    adIn();
  }

  /******************************************
     N64 Controller CRC Functions
  *****************************************/
  static uint16_t addrCRC(uint16_t address)
  {
    char const n64_address_crc_table[] = { 0x15, 0x1F, 0x0B, 0x16, 0x19, 0x07, 0x0E, 0x1C, 0x0D, 0x1A, 0x01 };
    char const * cur_xor = n64_address_crc_table;

    uint8_t crc = 0;

    for (uint16_t mask = 0x0020; mask; mask <<= 1, cur_xor++)
    {
      if (address & mask)
      {
        crc ^= *cur_xor;
      }
    }

    return (address & 0xFFE0) | crc;
  }

  static uint8_t dataCRC(uint8_t* data)
  {
    uint8_t ret = 0;

    for (uint8_t i = 0; i <= 32; i++)
    {
      for (uint8_t mask = 0x80; mask; mask >>= 1)
      {
        uint8_t tmp = ret & 0x80 ? 0x85 : 0;

        ret <<= 1;

        if (i < 32)
        {
          if (data[i] & mask)
          {
            ret |= 0x1;
          }
        }

        ret ^= tmp;
      }
    }

    return ret;
  }

  // Macro producing a delay loop waiting an number of cycles multiple of 3, with
  // a range of 3 to 768 cycles (187.5ns to 48us). It takes 6 bytes to do so
  // (3 instructions) making it the same size as the equivalent 3-cycles NOP
  // delay. For shorter delays or non-multiple-of-3-cycle delays, add your own
  // NOPs.
# define N64_DELAY_LOOP(cycle_count) \
    do { \
      uint8_t i; \
      __asm__ __volatile__("\n" \
                          "\tldi %[i], %[loop_count]\n" \
                          ".delay_loop_%=:\n" \
                          "\tdec %[i]\n" \
                          "\tbrne .delay_loop_%=\n" \
                          : [i] "=r"(i) \
                          : [loop_count] "i"(cycle_count / 3) \
                          : "cc"); \
    } while (0)

  /******************************************
     N64 Controller Protocol Functions
  *****************************************/
  void sendJoyBus(uint8_t const * buffer, char length)
  {
    // Implemented in assembly as there is very little wiggle room, timing-wise.
    // Overall structure:
    //   outer_loop:
    //     mask = 0x80
    //     cur_byte = *(buffer++)
    //   inner_loop:
    //     falling edge
    //     if (cur_byte & mask) {
    //       wait 1us starting at the falling edge
    //       rising edge
    //       wait 2us starting at the rising edge
    //     } else {
    //       wait 3us starting at the falling edge
    //       rising edge
    //     }
    //   inner_common_codepath:
    //     mask >>= 1
    //     if (mask == 0)
    //       goto outer_loop_trailer
    //     wait +1us from the rising edge
    //     goto inner_loop
    //   outer_loop_trailer:
    //     length -= 1
    //     if (length == 0)
    //       goto stop_bit
    //     wait +1us from the rising edge
    //     goto outer_loop
    //   stop_bit:
    //     wait +1us from the rising edge
    //     falling edge
    //     wait 1us from the falling edge
    //     rising edge

    uint8_t mask, cur_byte, scratch;
    // Note on DDRH: retrieve the current DDRH value, and pre-compute the values
    // to write in order to drive the line high or low. This saves 3 cycles per
    // transition: sts (2 cycles) instead of lds, or/and, sts (2 + 1 + 2 cycles).
    // This means that no other code may run in parallel, but this function anyway
    // requires interrupts to be disabled in order to work in the expected amount
    // of time.
    uint8_t const line_low = DDRH | 0x10;
    uint8_t const line_high = line_low & 0xEF;
    __asm__ __volatile__("\n"
                        ".outer_loop_%=:\n"
                        // mask = 0x80
                        "\tldi  %[mask], 0x80\n"  // 1
                        // load byte to send from memory
                        "\tld   %[cur_byte], Z+\n"  // 2
                        ".inner_loop_%=:\n"
                        // Falling edge
                        "\tsts  %[out_byte], %[line_low]\n"  // 2
                        // Test cur_byte & mask, without clobbering either
                        "\tmov  %[scratch], %[cur_byte]\n"  // 1
                        "\tand  %[scratch], %[mask]\n"      // 1
                        "\tbreq .bit_is_0_%=\n"             // bit is 1: 1, bit is 0: 2

                        // bit is a 1
                        // Stay low for 1us (16 cycles).
                        // Time before: 3 cycles (mov, and, breq-false).
                        // Time after: sts (2 cycles).
                        // So 11 to go, so 3 3-cycles iterations and 2 nop.
                        "\tldi  %[scratch], 3\n"  // 1
                        ".delay_1_low_%=:\n"
                        "\tdec  %[scratch]\n"       // 1
                        "\tbrne .delay_1_low_%=\n"  // exit: 1, loop: 2
                        "\tnop\n"                   // 1
                        "\tnop\n"                   // 1
                        // Rising edge
                        "\tsts  %[out_byte], %[line_high]\n"  // 2
                        // Wait for 2us (32 cycles) to sync with the bot_is_0 codepath.
                        // Time before: 0 cycles.
                        // Time after: 2 cycles (rjmp).
                        // So 30 to go, so 10 3-cycles iterations and 0 nop.
                        "\tldi  %[scratch], 10\n"  // 1
                        ".delay_1_high_%=:\n"
                        "\tdec  %[scratch]\n"             // 1
                        "\tbrne .delay_1_high_%=\n"       // exit: 1, loop: 2
                        "\trjmp .inner_common_path_%=\n"  // 2

                        ".bit_is_0_%=:\n"
                        // bit is a 0
                        // Stay high for 3us (48 cycles).
                        // Time before: 4 cycles (mov, and, breq-true).
                        // Time after: 2 cycles (sts).
                        // So 42 to go, so 14 3-cycles iterations, and 0 nop.
                        "\tldi  %[scratch], 14\n"  // 1
                        ".delay_0_low_%=:\n"
                        "\tdec  %[scratch]\n"       // 1
                        "\tbrne .delay_0_low_%=\n"  // exit: 1, loop: 2
                        // Rising edge
                        "\tsts  %[out_byte], %[line_high]\n"  // 2

                        // codepath common to both possible values
                        ".inner_common_path_%=:\n"
                        "\tnop\n"                          // 1
                        "\tlsr  %[mask]\n"                 // 1
                        "\tbreq .outer_loop_trailer_%=\n"  // mask!=0: 1, mask==0: 2
                        // Stay high for 1us (16 cycles).
                        // Time before: 3 cycles (nop, lsr, breq-false).
                        // Time after: 4 cycles (rjmp, sts)
                        // So 9 to go, so 3 3-cycles iterations and 0 nop.
                        "\tldi  %[scratch], 3\n"  // 1
                        ".delay_common_high_%=:\n"
                        "\tdec  %[scratch]\n"             // 1
                        "\tbrne .delay_common_high_%=\n"  // exit: 1, loop: 2
                        "\trjmp .inner_loop_%=\n"         // 2

                        ".outer_loop_trailer_%=:\n"
                        "\tdec %[length]\n"      // 1
                        "\tbreq .stop_bit_%=\n"  // length!=0: 1, length==0: 2
                        // Stay high for 1us (16 cycles).
                        // Time before: 6 cycles (lsr, nop, breq-true, dec, breq-false).
                        // Time after: 7 cycles (rjmp, ldi, ld, sts).
                        // So 3 to go, so 3 nop (for simplicity).
                        "\tnop\n"                  // 1
                        "\tnop\n"                  // 1
                        "\tnop\n"                  // 1
                        "\trjmp .outer_loop_%=\n"  // 2
                        // Done sending data, send a stop bit.
                        ".stop_bit_%=:\n"
                        // Stay high for 1us (16 cycles).
                        // Time before: 7 cycles (lsr, nop, breq-true, dec, breq-true).
                        // Time after: 2 cycles (sts).
                        // So 7 to go, so 2 3-cycles iterations and 1 nop.
                        "\tldi  %[scratch], 2\n"  // 1
                        ".delay_stop_high_%=:\n"
                        "\tdec  %[scratch]\n"           // 1
                        "\tbrne .delay_stop_high_%=\n"  // exit: 1, loop: 2
                        "\tnop\n"
                        "\tsts  %[out_byte], %[line_low]\n"  // 2
                        // Stay low for 1us (16 cycles).
                        // Time before: 0 cycles.
                        // Time after: 2 cycles (sts).
                        // So 14 to go, so 4 3-cycles iterations and 2 nop.
                        "\tldi  %[scratch], 5\n"  // 1
                        ".delay_stop_low_%=:\n"
                        "\tdec  %[scratch]\n"          // 1
                        "\tbrne .delay_stop_low_%=\n"  // exit: 1, loop: 2
                        "\tnop\n"
                        "\tnop\n"
                        "\tsts  %[out_byte], %[line_high]\n"  // 2
                        // Notes on arguments:
                        // - mask and scratch are used wth "ldi", which can only work on registers
                        //   16 to 31, so tag these with "a" rather than the generic "r"
                        // - mark all output-only arguments as early-clobber ("&"), as input
                        //   registers are used throughout all iterations and both sets must be
                        //   strictly distinct
                        // - tag buffer with "z", to use the "ld r?, Z+" instruction (load from
                        //   16bits RAM address and postincrement, in 2 cycles).
                        //   XXX: any pointer register pair would do, but mapping to Z explicitly
                        //   because I cannot find a way to get one of "X", "Y" or "Z" to appear
                        //   when expanding "%[buffer]", causing the assembler to reject the
                        //   instruction. Pick Z as it is the only call-used such register,
                        //   avoiding the need to preserve any value a caller may have set it to.
                        : [buffer] "+z"(buffer),
                        [length] "+r"(length),
                        [cur_byte] "=&r"(cur_byte),
                        [mask] "=&a"(mask),
                        [scratch] "=&a"(scratch)
                        : [line_low] "r"(line_low),
                        [line_high] "r"(line_high),
                        [out_byte] "i"(&DDRH)
                        : "cc", "memory");
  }

  uint16_t recvJoyBus(uint8_t * output, uint8_t byte_count)
  {
    // listen for expected byte_count bytes of data back from the controller
    // return the number of bytes not (fully) received if the delay for a signal
    // edge takes too long.

    // Implemented in assembly as there is very little wiggle room, timing-wise.
    // Overall structure:
    //     mask = 0x80
    //     cur_byte = 0
    //   read_loop:
    //     wait for falling edge
    //     wait for a bit more than 1us
    //     if input:
    //       cur_byte |= mask
    //     mask >>= 1
    //     if (mask == 0)
    //       if (--byte_count == 0)
    //         goto read_end
    //       append cur_byte to output
    //       mask = 0x80
    //       cur_byte = 0
    //     wait for data high
    //     goto read_loop
    //   read_end:
    //     return byte_count

    uint8_t mask, cur_byte, timeout, scratch;
    __asm__ __volatile__("\n"
                        "\tldi  %[mask], 0x80\n"
                        "\tclr  %[cur_byte]\n"
                        ".read_loop_%=:\n"
                        // Wait for input to be low. Time out if it takes more than ~27us (~7 bits
                        // worth of time) for it to go low.
                        // Takes 5 cycles to exit on input-low iteration (lds, sbrs-false, rjmp).
                        // Takes 7 cycles to loop on input-high iteration (lds, sbrs-true, dec,
                        //  brne-true).
                        "\tldi  %[timeout], 0x3F\n"  // 1
                        ".read_wait_falling_edge_%=:\n"
                        "\tlds  %[scratch], %[in_byte]\n"      // 2
                        "\tsbrs %[scratch], %[in_bit]\n"       // low: 1, high: 2
                        "\trjmp .read_input_low_%=\n"          // 2
                        "\tdec  %[timeout]\n"                  // 1
                        "\tbrne .read_wait_falling_edge_%=\n"  // timeout==0: 1, timeout!=0: 2
                        "\trjmp .read_end_%=\n"                // 2

                        ".read_input_low_%=:\n"
                        // Wait for 1500 us (24 cycles) before reading input.
                        // As it takes from 5 to 7 cycles for the prevous loop to exit,
                        // this means this loop exits from 1812.5us to 1937.5us after the falling
                        // edge, so at least 812.5us after a 1-bit rising edge, and at least
                        // 1062.5us before a 0-bit rising edge.
                        // This also leaves us with up to 2062.5us (33 cycles) to update cur_byte,
                        // possibly moving on to the next byte, waiting for a high input, and
                        // waiting for the next falling edge.
                        // Time taken until waiting for input high for non-last byte:
                        // - shift to current byte:
                        //   - 1: 4 cycles (lds, sbrc-false, or)
                        //   - 0: 4 cycles (lds, sbrc-true)
                        // - byte done: 8 cycles (lsr, brne-false, st, dec, brne-false, ldi, clr)
                        // - byte not done: 3 cycles (lsr, brne-true)
                        // Total: 7 to 12 cycles, so there are at least 21 cycles left until the
                        // next bit.
                        "\tldi  %[timeout], 8\n"  // 1
                        ".read_wait_low_%=:\n"
                        "\tdec  %[timeout]\n"         // 1
                        "\tbrne .read_wait_low_%=\n"  // timeout=0: 1, timeout!=0: 2

                        // Sample input
                        "\tlds  %[scratch], %[in_byte]\n"  // 2
                        // Add to cur_byte
                        "\tsbrc %[scratch], %[in_bit]\n"  // high: 1, low: 2
                        "\tor   %[cur_byte], %[mask]\n"   // 1
                        // Shift mask
                        "\tlsr  %[mask]\n"
                        "\tbrne .read_wait_input_high_init_%=\n"  // mask==0: 1, mask!=0: 2
                        // A wole byte was read, store in output
                        "\tst   Z+, %[cur_byte]\n"  // 2
                        // Decrement byte count
                        "\tdec  %[byte_count]\n"  // 1
                        // Are we done reading ?
                        "\tbreq .read_end_%=\n"  // byte_count!=0: 1, byte_count==0: 2
                        // No, prepare for reading another
                        "\tldi  %[mask], 0x80\n"
                        "\tclr  %[cur_byte]\n"

                        // Wait for rising edge
                        ".read_wait_input_high_init_%=:"
                        "\tldi  %[timeout], 0x3F\n"  // 1
                        ".read_wait_input_high_%=:\n"
                        "\tlds  %[scratch], %[in_byte]\n"    // 2
                        "\tsbrc %[scratch], %[in_bit]\n"     // high: 1, low: 2
                        "\trjmp .read_loop_%=\n"             // 2
                        "\tdec  %[timeout]\n"                // 1
                        "\tbrne .read_wait_input_high_%=\n"  // timeout==0: 1, timeout!=0: 2
                        "\trjmp .read_end_%=\n"              // 2
                        ".read_end_%=:\n"
                        : [output] "+z"(output),
                        [byte_count] "+r"(byte_count),
                        [mask] "=&a"(mask),
                        [cur_byte] "=&r"(cur_byte),
                        [timeout] "=&a"(timeout),
                        [scratch] "=&a"(scratch)
                        : [in_byte] "i"(&PINH),
                        [in_bit] "i"(4)
                        : "cc", "memory");
    return byte_count;
  }

  /******************************************
     N64 Controller Functions
  *****************************************/
  void get_button()
  {
    // Command to send to the gamecube
    // The last bit is rumble, flip it to rumble
    uint8_t const command[] = { 0x01 };
    uint8_t response[4];

    // don't want interrupts getting in the way
    noInterrupts();
    sendJoyBus(command, sizeof(command));
    recvJoyBus(response, sizeof(response));
    // end of time sensitive code
    interrupts();

    // These are 8 bit values centered at 0x80 (128)
    N64_status.stick_x = response[2];
    N64_status.stick_y = response[3];

    // Buttons (A,B,Z,S,DU,DD,DL,DR,0,0,L,R,CU,CD,CL,CR)
    if (response[0] & 0x80)
      button = F("A");
    else if (response[0] & 0x40)
      button = FS(OSCR::Strings::Units::B);
    else if (response[0] & 0x20)
      button = F("Z");
    else if (response[0] & 0x10)
      button = F("START");
    else if (response[0] & 0x08)
      button = F("D-Up");
    else if (response[0] & 0x04)
      button = F("D-Down");
    else if (response[0] & 0x02)
      button = F("D-Left");
    else if (response[0] & 0x01)
      button = F("D-Right");
    //else if (response[1] & 0x80)
    //else if (response[1] & 0x40)
    else if (response[1] & 0x20)
      button = F("L");
    else if (response[1] & 0x10)
      button = F("R");
    else if (response[1] & 0x08)
      button = F("C-Up");
    else if (response[1] & 0x04)
      button = F("C-Down");
    else if (response[1] & 0x02)
      button = F("C-Left");
    else if (response[1] & 0x01)
      button = F("C-Right");
    else {
      lastbutton = button;
      button = F("Press a button");
    }
  }


  /******************************************
    N64 Controller Test
  *****************************************/
# if defined(ENABLE_CONTROLLERTEST)
#   ifdef ENABLE_SERIAL
  void controllerTest_Serial()
  {
    while (quit) {
      // Get Button and analog stick
      get_button();

      // Print Button
      String buttonc = String("Button: " + String(button) + "   ");
      OSCR::Serial::print(buttonc);

      // Print Stick X Value
      String stickx = String("X: " + String(N64_status.stick_x, DEC) + "   ");
      OSCR::Serial::print(stickx);

      // Print Stick Y Value
      String sticky = String(" Y: " + String(N64_status.stick_y, DEC) + "   ");
      OSCR::Serial::printLine(sticky);

      if (button == "Press a button" && lastbutton == "Z") {
        // Quit
        OSCR::Serial::printLine();
        quit = 0;
      }
    }
  }
#   endif /* ENABLE_SERIAL */

#   if ((HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864) || defined(ENABLE_OLED))
#     define CENTER 64

  // on which screens do we start
  int startscreen = 1;
  int test = 1;

  void printSTR(String st, int x, int y)
  {
    char buf[st.length() + 1];

    if (x == CENTER) {
      x = 64 - (((st.length() - 5) / 2) * 4);
    }

    st.toCharArray(buf, st.length() + 1);
    display.drawStr(x, y, buf);
  }

  void nextscreen()
  {
    if (button == "Press a button" && lastbutton == "START") {
      // reset button
      lastbutton = "N/A";

      display.clearDisplay();
      if (startscreen != 4)
        startscreen = startscreen + 1;
      else {
        startscreen = 1;
        test = 1;
      }
    } else if (button == "Press a button" && lastbutton == "Z" && startscreen == 4) {
      // Quit
      quit = 0;
    }
  }

  void controllerTest_Display()
  {
    bool cmode = 1;

    //name of the current displayed result
    String anastick = "";

    // Graph
    int xax = 24;  // midpoint x
    int yax = 24;  // midpoint y

    // variables to display test data of different sticks
    int upx = 0;
    int upy = 0;
    int uprightx = 0;
    int uprighty = 0;
    int rightx = 0;
    int righty = 0;
    int downrightx = 0;
    int downrighty = 0;
    int downx = 0;
    int downy = 0;
    int downleftx = 0;
    int downlefty = 0;
    int leftx = 0;
    int lefty = 0;
    int upleftx = 0;
    int uplefty = 0;

    // variables to save test data
    int bupx = 0;
    int bupy = 0;
    int buprightx = 0;
    int buprighty = 0;
    int brightx = 0;
    int brighty = 0;
    int bdownrightx = 0;
    int bdownrighty = 0;
    int bdownx = 0;
    int bdowny = 0;
    int bdownleftx = 0;
    int bdownlefty = 0;
    int bleftx = 0;
    int blefty = 0;
    int bupleftx = 0;
    int buplefty = 0;
    int results = 0;
    int prevStickX = 0;

    String stickx;
    String sticky;
    String stickx_old;
    String sticky_old;
    String button_old;

    while (quit) {
      // Get Button and analog stick
      get_button();

      switch (startscreen) {
        case 1:
          {
            display.drawStr(32, 8, "Controller Test");
            display.drawLine(0, 10, 128, 10);

            // Delete old button value
            if (button_old != button) {
              display.setDrawColor(0);
              for (uint8_t y = 13; y < 22; y++) {
                display.drawLine(0, y, 128, y);
              }
              display.setDrawColor(1);
            }
            // Print button
            printSTR("       " + button + "       ", CENTER, 20);
            // Save value
            button_old = button;

            // Update stick values
            stickx = String("X: " + String(N64_status.stick_x, DEC) + "   ");
            sticky = String("Y: " + String(N64_status.stick_y, DEC) + "   ");

            // Delete old stick values
            if ((stickx_old != stickx) || (sticky_old != sticky)) {
              display.setDrawColor(0);
              for (uint8_t y = 31; y < 38; y++) {
                display.drawLine(0, y, 128, y);
              }
              display.setDrawColor(1);
            }

            // Print stick values
            printSTR(stickx, 36, 38);
            printSTR(sticky, 74, 38);
            // Save values
            stickx_old = stickx;
            sticky_old = sticky;

            printSTR("(Continue with START)", 16, 55);

            //Update LCD
            display.updateDisplay();

            // go to next screen
            nextscreen();
            break;
          }
        case 2:
          {
            display.drawStr(36, 8, "Range Test");
            display.drawLine(0, 9, 128, 9);

            if (cmode == 0) {
              // Print Stick X Value
              String stickx = String("X:" + String(N64_status.stick_x, DEC) + "   ");
              printSTR(stickx, 22 + 54, 26);

              // Print Stick Y Value
              String sticky = String("Y:" + String(N64_status.stick_y, DEC) + "   ");
              printSTR(sticky, 22 + 54, 36);
            }

            // Draw Axis
            display.drawPixel(10 + xax, 12 + yax);
            display.drawPixel(10 + xax, 12 + yax - 80 / 4);
            display.drawPixel(10 + xax, 12 + yax + 80 / 4);
            display.drawPixel(10 + xax + 80 / 4, 12 + yax);
            display.drawPixel(10 + xax - 80 / 4, 12 + yax);

            // Draw corners
            display.drawPixel(10 + xax - 68 / 4, 12 + yax - 68 / 4);
            display.drawPixel(10 + xax + 68 / 4, 12 + yax + 68 / 4);
            display.drawPixel(10 + xax + 68 / 4, 12 + yax - 68 / 4);
            display.drawPixel(10 + xax - 68 / 4, 12 + yax + 68 / 4);

            //Draw Analog Stick
            if (cmode == 1) {
              display.drawPixel(10 + xax + N64_status.stick_x / 4, 12 + yax - N64_status.stick_y / 4);
              //Update LCD
              display.updateDisplay();
            } else {
              display.drawCircle(10 + xax + N64_status.stick_x / 4, 12 + yax - N64_status.stick_y / 4, 2);
              //Update LCD
              display.updateDisplay();
              OSCR::UI::clear();
            }

            // switch mode
            if (button == "Press a button" && lastbutton == "Z") {
              if (cmode == 0) {
                cmode = 1;
                display.clearDisplay();
              } else {
                cmode = 0;
                display.clearDisplay();
              }
            }
            // go to next screen
            nextscreen();
            break;
          }
        case 3:
          {
            display.setDrawColor(0);
            display.drawPixel(22 + prevStickX, 40);
            display.setDrawColor(1);
            printSTR("Skipping Test", 34, 8);
            display.drawLine(0, 9, 128, 9);
            display.drawFrame(22 + 0, 15, 22 + 59, 21);
            if (N64_status.stick_x > 0) {
              display.drawLine(22 + N64_status.stick_x, 15, 22 + N64_status.stick_x, 35);
              display.drawPixel(22 + N64_status.stick_x, 40);
              prevStickX = N64_status.stick_x;
            }

            printSTR("Try to fill the box by", 22, 45);
            printSTR("slowly moving right", 22, 55);
            //Update LCD
            display.updateDisplay();

            if (button == "Press a button" && lastbutton == "Z") {
              // reset button
              lastbutton = "N/A";

              display.clearDisplay();
            }
            // go to next screen
            nextscreen();
            break;
          }
        case 4:
          {
            switch (test) {
              case 0:  // Display results
                {
                  switch (results) {
                    case 0:
                      {
                        anastick = "Your Stick";
                        upx = bupx;
                        upy = bupy;
                        uprightx = buprightx;
                        uprighty = buprighty;
                        rightx = brightx;
                        righty = brighty;
                        downrightx = bdownrightx;
                        downrighty = bdownrighty;
                        downx = bdownx;
                        downy = bdowny;
                        downleftx = bdownleftx;
                        downlefty = bdownlefty;
                        leftx = bleftx;
                        lefty = blefty;
                        upleftx = bupleftx;
                        uplefty = buplefty;

                        if (button == "Press a button" && lastbutton == "A") {
                          // reset button
                          lastbutton = "N/A";
                          results = 1;
                          display.clearDisplay();
                          break;
                        }
                        printSTR(anastick, 22 + 50, 15);

                        display.drawStr(22 + 50, 25, "U:");
                        printSTR(String(upy), 100, 25);
                        display.drawStr(22 + 50, 35, "D:");
                        printSTR(String(downy), 100, 35);
                        display.drawStr(22 + 50, 45, "L:");
                        printSTR(String(leftx), 100, 45);
                        display.drawStr(22 + 50, 55, "R:");
                        printSTR(String(rightx), 100, 55);

                        display.drawLine(xax + upx / 4, yax - upy / 4, xax + uprightx / 4, yax - uprighty / 4);
                        display.drawLine(xax + uprightx / 4, yax - uprighty / 4, xax + rightx / 4, yax - righty / 4);
                        display.drawLine(xax + rightx / 4, yax - righty / 4, xax + downrightx / 4, yax - downrighty / 4);
                        display.drawLine(xax + downrightx / 4, yax - downrighty / 4, xax + downx / 4, yax - downy / 4);
                        display.drawLine(xax + downx / 4, yax - downy / 4, xax + downleftx / 4, yax - downlefty / 4);
                        display.drawLine(xax + downleftx / 4, yax - downlefty / 4, xax + leftx / 4, yax - lefty / 4);
                        display.drawLine(xax + leftx / 4, yax - lefty / 4, xax + upleftx / 4, yax - uplefty / 4);
                        display.drawLine(xax + upleftx / 4, yax - uplefty / 4, xax + upx / 4, yax - upy / 4);

                        display.drawPixel(xax, yax);

                        //Update LCD
                        display.updateDisplay();
                        break;
                      }
                    case 1:
                      {
                        anastick = "Original";
                        upx = 1;
                        upy = 84;
                        uprightx = 67;
                        uprighty = 68;
                        rightx = 83;
                        righty = -2;
                        downrightx = 67;
                        downrighty = -69;
                        downx = 3;
                        downy = -85;
                        downleftx = -69;
                        downlefty = -70;
                        leftx = -85;
                        lefty = 0;
                        upleftx = -68;
                        uplefty = 68;

                        if (button == "Press a button" && lastbutton == "A") {
                          // reset button
                          lastbutton = "N/A";
                          results = 0;
                          display.clearDisplay();
                          break;
                        }
                        printSTR(anastick, 22 + 50, 15);

                        display.drawStr(22 + 50, 25, "U:");
                        printSTR(String(upy), 100, 25);
                        display.drawStr(22 + 50, 35, "D:");
                        printSTR(String(downy), 100, 35);
                        display.drawStr(22 + 50, 45, "L:");
                        printSTR(String(leftx), 100, 45);
                        display.drawStr(22 + 50, 55, "R:");
                        printSTR(String(rightx), 100, 55);

                        display.drawLine(xax + upx / 4, yax - upy / 4, xax + uprightx / 4, yax - uprighty / 4);
                        display.drawLine(xax + uprightx / 4, yax - uprighty / 4, xax + rightx / 4, yax - righty / 4);
                        display.drawLine(xax + rightx / 4, yax - righty / 4, xax + downrightx / 4, yax - downrighty / 4);
                        display.drawLine(xax + downrightx / 4, yax - downrighty / 4, xax + downx / 4, yax - downy / 4);
                        display.drawLine(xax + downx / 4, yax - downy / 4, xax + downleftx / 4, yax - downlefty / 4);
                        display.drawLine(xax + downleftx / 4, yax - downlefty / 4, xax + leftx / 4, yax - lefty / 4);
                        display.drawLine(xax + leftx / 4, yax - lefty / 4, xax + upleftx / 4, yax - uplefty / 4);
                        display.drawLine(xax + upleftx / 4, yax - uplefty / 4, xax + upx / 4, yax - upy / 4);

                        display.drawPixel(xax, yax);

                        //Update LCD
                        display.updateDisplay();
                        break;
                      }

                  }  //results
                  break;
                }  //display results

              case 1:  // +y Up
                {
                  display.drawStr(34, 26, "Hold Stick Up");
                  display.drawStr(34, 34, "then press A");
                  //display.drawBitmap(110, 60, ana1);

                  if (button == "Press a button" && lastbutton == "A") {
                    bupx = N64_status.stick_x;
                    bupy = N64_status.stick_y;
                    // reset button
                    lastbutton = "N/A";

                    display.clearDisplay();
                    test = 2;
                  }
                  break;
                }

              case 2:  // +y+x Up-Right
                {
                  display.drawStr(42, 26, "Up-Right");
                  //display.drawBitmap(110, 60, ana2);

                  if (button == "Press a button" && lastbutton == "A") {
                    buprightx = N64_status.stick_x;
                    buprighty = N64_status.stick_y;
                    test = 3;
                    // reset button
                    lastbutton = "N/A";

                    display.clearDisplay();
                  }
                  break;
                }

              case 3:  // +x Right
                {
                  display.drawStr(50, 26, "Right");
                  //display.drawBitmap(110, 60, ana3);

                  if (button == "Press a button" && lastbutton == "A") {
                    brightx = N64_status.stick_x;
                    brighty = N64_status.stick_y;
                    test = 4;
                    // reset button
                    lastbutton = "N/A";

                    display.clearDisplay();
                  }
                  break;
                }

              case 4:  // -y+x Down-Right
                {
                  display.drawStr(38, 26, "Down-Right");
                  //display.drawBitmap(110, 60, ana4);

                  if (button == "Press a button" && lastbutton == "A") {
                    bdownrightx = N64_status.stick_x;
                    bdownrighty = N64_status.stick_y;
                    test = 5;
                    // reset button
                    lastbutton = "N/A";

                    display.clearDisplay();
                  }
                  break;
                }

              case 5:  // -y Down
                {
                  display.drawStr(49, 26, "Down");
                  //display.drawBitmap(110, 60, ana5);

                  if (button == "Press a button" && lastbutton == "A") {
                    bdownx = N64_status.stick_x;
                    bdowny = N64_status.stick_y;
                    test = 6;
                    // reset button
                    lastbutton = "N/A";

                    display.clearDisplay();
                  }
                  break;
                }

              case 6:  // -y-x Down-Left
                {
                  display.drawStr(39, 26, "Down-Left");
                  //display.drawBitmap(110, 60, ana6);

                  if (button == "Press a button" && lastbutton == "A") {
                    bdownleftx = N64_status.stick_x;
                    bdownlefty = N64_status.stick_y;
                    test = 7;
                    // reset button
                    lastbutton = "N/A";

                    display.clearDisplay();
                  }
                  break;
                }

              case 7:  // -x Left
                {
                  display.drawStr(51, 26, "Left");
                  //display.drawBitmap(110, 60, ana7);

                  if (button == "Press a button" && lastbutton == "A") {
                    bleftx = N64_status.stick_x;
                    blefty = N64_status.stick_y;
                    test = 8;
                    // reset button
                    lastbutton = "N/A";

                    display.clearDisplay();
                  }
                  break;
                }

              case 8:  // +y+x Up-Left
                {
                  display.drawStr(43, 26, "Up-Left");
                  //display.drawBitmap(110, 60, ana8);

                  if (button == "Press a button" && lastbutton == "A") {
                    bupleftx = N64_status.stick_x;
                    buplefty = N64_status.stick_y;
                    test = 0;
                    // reset button
                    lastbutton = "N/A";

                    display.clearDisplay();
                  }
                  break;
                }
            }
            if (test != 0) {
              display.drawStr(38, 8, "Benchmark");
              display.drawLine(0, 9, 128, 9);
            }
            display.updateDisplay();
            // go to next screen
            nextscreen();
            break;
          }
      }
    }
  }
#   endif /* ENABLE_LCD || ENABLE_OLED */
# endif /* ENABLE_CONTROLLERTEST */

  /******************************************
     N64 Controller Pak Functions
    (connected via Controller)
  *****************************************/
  // Reset the controller
  void resetController()
  {
    uint8_t const command[] = { 0xFF };
    noInterrupts();
    sendJoyBus(BUFFN(command));
    interrupts();
    delay(100);
  }

  // read 3 bytes from controller
  void checkController()
  {
    uint8_t response[8];
    uint8_t const command[] = { 0x00 };

    printHeader();

    // Check if line is HIGH
    if (!N64_QUERY)
      OSCR::UI::fatalError(F("Data line LOW"));

    // don't want interrupts getting in the way
    noInterrupts();
    sendJoyBus(command, sizeof(command));
    recvJoyBus(response, sizeof(response));
    // end of time sensitive code
    interrupts();

    if (response[0] != 0x05)
      OSCR::UI::fatalError(F("Controller not found"));
    if (response[2] != 0x01)
      OSCR::UI::fatalError(F("Controller Pak not found"));
  }

  // read 32bytes from controller pak and calculate CRC
  uint8_t readBlock(byte* output, uint16_t myAddress)
  {
    uint8_t response_crc;
    // Calculate the address CRC
    uint16_t myAddressCRC = addrCRC(myAddress);
    uint8_t const command[] = { 0x02, (byte)(myAddressCRC >> 8), (byte)(myAddressCRC & 0xFF) };
    uint16_t error;

    // don't want interrupts getting in the way
    noInterrupts();
    sendJoyBus(command, sizeof(command));
    error = recvJoyBus(output, 32);
    if (error == 0)
      error = recvJoyBus(&response_crc, 1);
    // end of time sensitive code
    interrupts();

    if (error)
    {
      printHeader();
      OSCR::UI::printLine(F("Controller Pak was"));
      OSCR::UI::printLine(F("not dumped due to a"));
      OSCR::UI::fatalError(F("read timeout"));
    }

    // Compare with computed CRC
    if (response_crc != dataCRC(output))
    {
      printHeader();
      OSCR::UI::printLine(F("Controller Pak was"));
      OSCR::UI::printLine(F("not dumped due to a"));
      OSCR::UI::fatalError(F("protocol CRC error"));
    }

    return response_crc;
  }

  // reads the MPK file to the sd card
  bool readMPK()
  {
    char crcFileName[20];
    uint8_t buf[256];
    bool failed = false;

    cartOn();

    resetController();
    checkController();

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::N64F), FS(OSCR::Strings::Directory::MPK), fileName, FS(OSCR::Strings::FileType::MPK));

    OSCR::Storage::File crcFile;
    snprintf_P(BUFFN(crcFileName), OSCR::Storage::FilenameTemplateP, fileName, OSCR::Strings::FileType::CRCX);
    crcFile.open(crcFileName, O_RDWR | O_CREAT);

    // Dummy write because first write to file takes 1 second and messes up timing
    OSCR::Storage::Shared::sharedFile.write((uint8_t)0xFF);
    OSCR::Storage::Shared::rewind();

    OSCR::UI::setLineRel(2);
    OSCR::UI::ProgressBar::init(0xFFFE);
    OSCR::UI::setLineRel(-2);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
    for (uint16_t pos = 0x0000; pos < 0x8000; pos += 512)
    {
      // Read 32 byte block into buffer
      for (uint16_t currBlock = 0; currBlock < OSCR::Storage::kBufferSize; currBlock += 32)
      {
        // Read one block of the Controller Pak into array myBlock and write CRC of that block to crc file
        crcFile.write(readBlock(&OSCR::Storage::Shared::buffer[currBlock], pos + currBlock));

        // Real N64 has about 627us pause between banks, add a bit extra delay
        if (currBlock < 479)
        {
          delayMicroseconds(800);
        }
      }

      OSCR::Storage::Shared::writeBuffer();

      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    writeErrors = 0;

    OSCR::Storage::Shared::rewind();
    crcFile.rewind();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
    for (uint16_t pos = 0x0000; pos < 0x8000; pos += 512)
    {
      // Read 32 bytes into SD buffer
      OSCR::Storage::Shared::fill();

      // Compare 32 byte block CRC to CRC from file
      for (uint16_t currBlock = 0; currBlock < 512; currBlock += 32)
      {
        // Calculate CRC of block and compare against crc file
        if (dataCRC(&OSCR::Storage::Shared::buffer[currBlock]) != crcFile.read())
          writeErrors++;
      }

      // Update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    if (writeErrors > 0)
    {
      crcFile.close();
      OSCR::Storage::Shared::close();

      OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
      return false;
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

    OSCR::UI::ProgressBar::finish();

    crcFile.close();

    OSCR::Storage::Shared::rewind();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    // Read first 256 byte which contains the header including checksum and reverse checksum and three copies of it
    OSCR::Storage::Shared::sharedFile.read(BUFFN(buf));

    //Check all four header copies
    uint8_t headerErrors = 0;
    uint8_t checkAddresses[] = {
      0x20,
      0x60,
      0x80,
      0xC0,
    };

    for (uint8_t i = 0; i < sizeofarray(checkAddresses); i++)
    {
      headerErrors += checkHeader(&buf[checkAddresses[i]]);
    }

    if (headerErrors > 0)
    {
      failed = true;
    }

    // Check both TOC copies
    writeErrors = 0;

    // Read 2nd and 3rd 256 byte page with TOC info
    for (uint16_t pos = 0x100; pos < 0x300; pos += 256)
    {
      uint8_t sum = 0;

      // Read 256 bytes into SD buffer
      OSCR::Storage::Shared::sharedFile.read(BUFFN(buf));

      // Calculate TOC checksum
      for (uint8_t i = 5; i < 128; i++)
      {
        sum += buf[(i << 1) + 1];
      }

      if (buf[1] != sum)
      {
        writeErrors++;
        failed = true;
      }
    }

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(failed ? OSCR::Strings::Common::FAIL : OSCR::Strings::Common::OK));

    OSCR::UI::printValue(OSCR::Strings::Common::Checksum, 4 - headerErrors, 4);
    OSCR::UI::printValue(PSTR("ToC"), (uint32_t)(2 - writeErrors), (uint32_t)(2));

    return true;
  }

  // Calculates the checksum of the header
  bool checkHeader(byte* buf)
  {
    uint16_t sum = 0;
    uint16_t buf_sum = (buf[28] << 8) + buf[29];

    // first 28 bytes are the header, then comes the checksum(word) followed by the reverse checksum(0xFFF2 - checksum)
    for (uint8_t i = 0; i < 28; i += 2)
    {
      sum += (buf[i] << 8) + buf[i + 1];
    }

    return (sum == buf_sum);
  }

  bool writeMPK()
  {
    // 3 command bytes, 32 data bytes
    uint8_t command[3 + 32] = { 0x03 };

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();

    cartOn();

    resetController();
    checkController();

    OSCR::UI::setLineRel(2);
    OSCR::UI::ProgressBar::init(0xFFFE);
    OSCR::UI::setLineRel(-2);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    for (uint16_t address = 0x0000; address < 0x8000; address += 32)
    {
      OSCR::Storage::Shared::sharedFile.read(command + 3, sizeof(command) - 3);

      uint16_t address_with_crc = addrCRC(address);
      command[1] = (byte)(address_with_crc >> 8);
      command[2] = (byte)(address_with_crc & 0xFF);

      noInterrupts(); // don't want interrupts getting in the way

      sendJoyBus(BUFFN(command));

      interrupts(); // Reenable interrupts

      // Real N64 has about 627us pause between banks, add a bit extra delay
      delayMicroseconds(650);

      // Update progress bar
      OSCR::UI::ProgressBar::advance(32);
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    delay(500);

    uint8_t block[32];

    writeErrors = 0;

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    OSCR::Storage::Shared::rewind();

    // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
    for (uint16_t pos = 0x0000; pos < 0x8000; pos += OSCR::Storage::kBufferSize)
    {
      // Read 512 bytes into SD buffer
      OSCR::Storage::Shared::readBuffer();

      // Compare 32 byte block
      for (uint16_t currBlock = 0; currBlock < OSCR::Storage::kBufferSize; currBlock += 32)
      {
        // Read one block of the Controller Pak
        readBlock(block, pos + currBlock);

        // Check against file on SD card
        for (uint8_t currByte = 0; currByte < 32; currByte++)
        {
          if (OSCR::Storage::Shared::buffer[currBlock + currByte] != block[currByte])
          {
            writeErrors++;
          }
        }

        // Real N64 has about 627us pause between banks, add a bit extra delay
        if (currBlock < 479)
          delayMicroseconds(1500);
      }

      // Update progress bar
      OSCR::UI::ProgressBar::advance(512);
    }

    cartOff();

    OSCR::Storage::Shared::close();

    if (writeErrors > 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::UI::ProgressBar::finish();
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
      return false;
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
    OSCR::UI::ProgressBar::finish();

    return true;
  }

  /******************************************
    N64 Cartridge functions
  *****************************************/
  bool refreshCart()
  {
    do
    {
      getCartInfo();

      if (!cartSize)
      {
        // Display error
        OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CartridgeError));

        OSCR::UI::printValue(OSCR::Strings::Common::Name, fileName);

        OSCR::UI::printValue(OSCR::Strings::Common::ID, cartID);

        OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);
        OSCR::UI::printLine(crc1);

        switch (OSCR::Prompts::abortRetryContinue())
        {
        case OSCR::Prompts::AbortRetryContinue::Abort: return false;
        case OSCR::Prompts::AbortRetryContinue::Retry: continue;
        case OSCR::Prompts::AbortRetryContinue::Continue: break;
        }

        useDefaultName();

        OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));

        cartSize = menuOptionsROM[OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeMB), menuOptionsROM, sizeofarray(menuOptionsROM))];

        return false;
      }

      printHeader();

      OSCR::UI::printLine(romDetail->name);
      OSCR::UI::printValue(OSCR::Strings::Common::ID, cartID);
      OSCR::UI::printValue(OSCR::Strings::Common::Revision, romVersion);
      OSCR::UI::printSize(OSCR::Strings::Common::ROM, cartSize * 1024 * 1024);

      char const * type;

      eepPages = 0;

      switch (saveType)
      {
      case 1:
        type = OSCR::Strings::Common::SRAM;
        break;

      case 2:
        type = PSTR("SRAM 768");
        break;

      case 4:
        type = OSCR::Strings::Common::Flash;
        break;

      case 5:
        type = OSCR::Strings::Common::EEPROM;
        eepPages = 64;
        break;

      case 6:
        type = OSCR::Strings::Common::EEPROM;
        eepPages = 256;
        break;

      default:
        type = OSCR::Strings::Common::None;
        break;
      }

      OSCR::UI::printType_P(OSCR::Strings::Common::Save, type);

      if (eepPages > 0)
      {
        OSCR::UI::printSize(OSCR::Strings::Common::Save, eepPages / 16);
      }

      OSCR::UI::printLabel(OSCR::Strings::Common::Checksum);
      OSCR::UI::printLine(crc1);

      return true;
    }
    while (true);
  }

  bool getCartInfo()
  {
    // cart not in list
    cartSize = 0;
    saveType = 0;

    cartOn();
    idCart();
    cartOff();

    printHeader();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::SearchingDatabase));

    if (OSCR::Databases::Extended::matchCRC(&crc1, 4))
    {
#   if CRDB_DEBUGGING
      OSCR::Serial::print(F("Found ROM: "));
      OSCR::Serial::printLine(romDetail->name);

      romRecord->debug();
#   endif /* CRDB_DEBUGGING */

      cartSize = romDetail->size;
      saveType = romDetail->mapper;

      return true;
    }

    return false;
  }

  // Read rom ID
  void idCart()
  {
    // Set the address
    setAddress(romBase);

    // Read first 64 bytes of rom
    for (uint16_t c = 0; c < 64; c += 2)
    {
      // split word
      uint16_t myWord = readWord();
      uint8_t loByte = myWord & 0xFF;
      uint8_t hiByte = myWord >> 8;

      // write to buffer
      OSCR::Storage::Shared::buffer[c] = hiByte;
      OSCR::Storage::Shared::buffer[c + 1] = loByte;
    }

    // Pull ale_H(PC1) high
    PORTC |= (1 << 1);

    // CRC1
    crc1.b[3] = (uint8_t)(OSCR::Storage::Shared::buffer[0x10]);
    crc1.b[2] = (uint8_t)(OSCR::Storage::Shared::buffer[0x11]);
    crc1.b[1] = (uint8_t)(OSCR::Storage::Shared::buffer[0x12]);
    crc1.b[0] = (uint8_t)(OSCR::Storage::Shared::buffer[0x13]);

    sprintf_P(checksumStr, PSTR("%08X"), (uint32_t)crc1);

    // Get cart id
    cartID[0] = OSCR::Storage::Shared::buffer[0x3B];
    cartID[1] = OSCR::Storage::Shared::buffer[0x3C];
    cartID[2] = OSCR::Storage::Shared::buffer[0x3D];
    cartID[3] = OSCR::Storage::Shared::buffer[0x3E];

    // Get rom version
    romVersion = OSCR::Storage::Shared::buffer[0x3F];

    // If name consists out of all japanese characters use cart id
    if (setOutName((char *)&OSCR::Storage::Shared::buffer[0x20], 20) == 0)
    {
      setOutName(BUFFN(cartID));
    }
  }

  bool readSave()
  {
    switch(saveType)
    {
    case 1:
      readSaveData(32768, false);
      break;

    case 2:
      readSaveData(98304, false);
      break;

    case 4:
      readSaveData(131072, true);
      break;

    case 5:
    case 6:
      readEeprom();
      break;
    }

    return true;
  }

  bool writeSave()
  {
    switch(saveType)
    {
    case 1:
      return writeSRAM(32768);

    case 2:
      return writeSRAM(98304);

    case 4:
      return writeFRAM();

    case 5:
    case 6:
      return writeEeprom();
    }

    return false;
  }

  // Write Eeprom to cartridge
  bool writeEeprom()
  {
    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();

    cartOn();

    resetEeprom();

    // 2 command bytes and 8 data bytes
    uint8_t command[2 + 8];
    command[0] = 0x05;

    // Note: eepPages can be 256, so page must be able to get to 256 for the
    // loop to exit. So it is not possible to use command[1] directly as loop
    // counter.
    for (uint16_t page = 0; page < eepPages; page++)
    {
      command[1] = page;

      // TODO: read 512 bytes in a 512 + 2 bytes buffer, and move the command start 32 bytes at a time
      OSCR::Storage::Shared::sharedFile.read(command + 2, sizeof(command) - 2);

      if (page) delay(50);  // Wait 50ms between pages when writing

      noInterrupts(); // Disable interrupts for more uniform clock pulses

      sendJoyBus(command, sizeof(command));

      interrupts();
    }

    OSCR::UI::printLineSync(FS(OSCR::Strings::Common::DONE));

    delay(600);

    resetEeprom();

    writeErrors = 0;

    OSCR::Storage::Shared::rewind();

    for (uint8_t i = 0; i < eepPages; i += kBufferSize8)
    {
      readEepromPageList(OSCR::Storage::Shared::buffer, i, kBufferSize8);

      // Check buffer content against file on sd card
      for (size_t c = 0; c < OSCR::Storage::kBufferSize; c++)
      {
        if (OSCR::Storage::Shared::sharedFile.read() != OSCR::Storage::Shared::buffer[c])
        {
          writeErrors++;
        }
      }
    }

    OSCR::Storage::Shared::close();

    cartOff();

    if (writeErrors > 0)
    {
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
    }

    return (writeErrors == 0);
  }

  bool readEepromPageList(byte * output, uint8_t page_number, uint8_t page_count)
  {
    uint8_t command[] = { 0x04, page_number };

    // Disable interrupts for more uniform clock pulses
    while (page_count--)
    {
      noInterrupts();

      sendJoyBus(command, sizeof(command));

      // XXX: is it possible to read more than 8 bytes at a time ?
      if (recvJoyBus(output, 8) > 0)
      {
        // If any missing bytes error out
        interrupts();
        return false;
      }

      interrupts();

      if (page_count)
        delayMicroseconds(600);  // wait 600us between pages when reading

      command[1]++;
      output += 8;
    }

    return 1;
  }

  // Reset Eeprom
  void resetEeprom()
  {
    // Pull RESET(PH0) low
    PORTH &= ~(1 << 0);
    delay(100);
    // Pull RESET(PH0) high
    PORTH |= (1 << 0);
    delay(100);
  }

  // Dump Eeprom to SD
  void readEeprom()
  {
    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::N64F), FS(OSCR::Strings::FileType::Save), fileName, FS(OSCR::Strings::FileType::SaveEEPROM));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    cartOn();

    resetEeprom();

    for (uint16_t i = 0; i < eepPages; i += kBufferSize8)
    {
      // If any missing bytes error out
      if (readEepromPageList(OSCR::Storage::Shared::buffer, i, kBufferSize8) == 0)
      {
        cartOff();

        OSCR::Storage::Shared::close();

        OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));

        OSCR::UI::error(FS(OSCR::Strings::Errors::NoDataReceived));
        return;
      }

      // Write 64 pages at once to the SD card
      OSCR::Storage::Shared::writeBuffer();
    }

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  /******************************************
    SRAM functions
  *****************************************/
  // Write sram to cartridge
  bool writeSRAM(uint32_t sramSize)
  {
    constexpr uint16_t const offset = 512, bufferSize = 512;

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    printHeader();

    uint32_t const maxAddress = (kSaveAddress + sramSize);

    for (uint32_t currByte = kSaveAddress; currByte < maxAddress; currByte += offset)
    {
      // Read save from SD into buffer
      OSCR::Storage::Shared::readBuffer();

      // Set the address for the next 512 bytes
      setAddress(currByte);

      for (uint16_t c = 0; c < bufferSize; c += 2)
      {
        // Join bytes to word
        uint16_t myWord = ((OSCR::Storage::Shared::buffer[c] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[c + 1] & 0xFF);

        // Write word
        writeWord(myWord);
      }
    }

    OSCR::UI::printLineSync(FS(OSCR::Strings::Common::DONE));

    return verifySaveData(maxAddress, offset, bufferSize, false);
  }

  // Read save data to the SD card
  bool readSaveData(uint32_t dataSize, bool isFlash)
  {
    uint16_t offset = 512, bufferSize = 512;
    uint8_t flashramType;
    char const * suffix;

    printHeader();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    flashramType = (isFlash) ? getFramType() : 1;

    if (isFlash)
    {
      sendFramCmd(0xF0000000);
    }

    if (flashramType == 2)
    {
      offset = 64;
      bufferSize = 128;
    }

    switch(saveType)
    {
    case 1: suffix = OSCR::Strings::FileType::SaveRA; break;
    case 2: suffix = PSTR("768"); break;
    case 4: suffix = OSCR::Strings::FileType::SaveFlash; break;
    default: suffix = OSCR::Strings::FileType::Save; break;
    }

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::N64F), FS(OSCR::Strings::FileType::Save), fileName, FS(suffix));

    OSCR::UI::clearLine();
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    uint32_t const maxAddress = (kSaveAddress + (dataSize / flashramType));

    for (uint32_t address = kSaveAddress; address < maxAddress; address += offset)
    {
      // Set the address
      setAddress(address);

      for (uint16_t c = 0, myWord = 0; c < bufferSize; c += 2)
      {
        // read, split and write word to buffer
        myWord = readWord();

        OSCR::Storage::Shared::buffer[c] = myWord >> 8;
        OSCR::Storage::Shared::buffer[c + 1] = myWord & 0xFF;
      }

      // Pull ale_H(PC1) high
      PORTC |= (1 << 1);

      OSCR::Storage::Shared::writeBuffer(bufferSize);
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    return true;
  }

  bool verifySaveData(uint32_t maxAddress, uint16_t offset, uint16_t bufferSize, bool isFlash)
  {
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    writeErrors = 0;

    OSCR::Storage::Shared::rewind();

    if (isFlash)
    {
      sendFramCmd(0xF0000000);
    }

    for (uint32_t address = kSaveAddress; address < maxAddress; address += offset)
    {
      // Set the address
      setAddress(address);

      for (uint16_t c = 0; c < bufferSize; c += 2)
      {
        // split word
        uint16_t myWord = readWord();

        // write to buffer
        OSCR::Storage::Shared::buffer[c] = myWord >> 8;
        OSCR::Storage::Shared::buffer[c + 1] = myWord & 0xFF;
      }

      // Check buffer content against file on sd card
      for (uint16_t i = 0; i < bufferSize; i++)
      {
        if (OSCR::Storage::Shared::sharedFile.read() != OSCR::Storage::Shared::buffer[i])
        {
          writeErrors++;
        }
      }
    }

    cartOff();

    OSCR::Storage::Shared::close();

    if (writeErrors > 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::Lang::printErrorVerifyBytes(writeErrors);
      return false;
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

    return true;
  }

  /******************************************
    Flashram functions
  *****************************************/
  // Send a command to the flashram command register
  void sendFramCmd(uint32_t myCommand)
  {
    // Split command into two words
    uint16_t const myComLowOut = myCommand & 0xFFFF;
    uint16_t const myComHighOut = myCommand >> 16;

    // Set address to command register
    setAddress(0x08010000);

    // Send command
    writeWord(myComHighOut);
    writeWord(myComLowOut);

    // Pull ale_H(PC1) high
    PORTC |= (1 << 1);
  }

  // Init fram
  void initFRAM()
  {
    // FRAM_EXECUTE_CMD
    sendFramCmd(0xD2000000);
    delay(10);

    // FRAM_EXECUTE_CMD
    sendFramCmd(0xD2000000);
    delay(10);

    //FRAM_STATUS_MODE_CMD
    sendFramCmd(0xE1000000);
    delay(10);
  }

  bool writeFRAM()
  {
    uint8_t flashramType;
    uint16_t offset = 512, bufferSize = 512;

    printHeader();

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Checking));

    flashramType = getFramType();

    if (flashramType == 2)
    {
      offset = 64;
      bufferSize = 128;
    }

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    initFRAM();

    // Erase fram
    // 0x4B00007F 0x4B0000FF 0x4B00017F 0x4B0001FF 0x4B00027F 0x4B0002FF 0x4B00037F 0x4B0003FF
    for (uint32_t bank = 0x4B00007F; bank < 0x4B00047F; bank += 0x80)
    {
      sendFramCmd(bank);
      delay(10);

      // FRAM_ERASE_MODE_CMD
      sendFramCmd(0x78000000);
      delay(10);

      // FRAM_EXECUTE_CMD
      sendFramCmd(0xD2000000);

      while (waitForFram(flashramType))
      {
        delay(1);
      }
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

    writeErrors = 0;

    // Put flashram into read mode
    // FRAM_READ_MODE_CMD
    sendFramCmd(0xF0000000);

    // Read Flashram
    for (uint32_t currByte = kSaveAddress; currByte < (kSaveAddress + (131072 / flashramType)); currByte += offset)
    {
      // Set the address for the next 512 bytes
      setAddress(currByte);

      for (uint16_t c = 0; c < bufferSize; c += 2)
      {
        // split word
        uint16_t myWord = readWord();
        uint8_t loByte = myWord & 0xFF;
        uint8_t hiByte = myWord >> 8;

        // write to buffer
        OSCR::Storage::Shared::buffer[c] = hiByte;
        OSCR::Storage::Shared::buffer[c + 1] = loByte;
      }

      // Check buffer content against file on sd card
      for (uint16_t i = 0; i < bufferSize; i++)
      {
        if (0xFF != OSCR::Storage::Shared::buffer[i])
        {
          writeErrors++;
        }
      }
    }

    if (writeErrors > 0)
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::UI::error(FS(OSCR::Strings::Common::NotBlank));
      return false;
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Writing));

    // Init fram
    initFRAM();

    // Write all 8 fram banks
    for (uint8_t bank = 0; bank < 8; bank++)
    {
      // Write one bank of 128*128 bytes
      for (uint8_t offset = 0; offset < 128; offset++)
      {
        // Read save from SD into buffer
        OSCR::Storage::Shared::readBuffer(128);

        // FRAM_WRITE_MODE_CMD
        sendFramCmd(0xB4000000);

        delay(1);

        // Set the address for the next 128 bytes
        setAddress(0x08000000);

        // Send 128 bytes, 64 words
        for (uint8_t c = 0; c < 128; c += 2)
        {
          // Join two bytes into one word
          uint16_t myWord = ((OSCR::Storage::Shared::buffer[c] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[c + 1] & 0xFF);

          // Write word
          writeWord(myWord);
        }

        // Delay between each "DMA"
        delay(1);

        //FRAM_WRITE_OFFSET_CMD + offset
        sendFramCmd((0xA5000000 | (((bank * 128) + offset) & 0xFFFF)));

        delay(1);

        // FRAM_EXECUTE_CMD
        sendFramCmd(0xD2000000);

        while (waitForFram(flashramType))
        {
          delay(1);
        }
      }

      // Delay between banks
      delay(20);
    }

    OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));

    uint32_t const maxAddress = (kSaveAddress + (131072 / flashramType));

    return verifySaveData(maxAddress, offset, bufferSize, true);
  }

  // Blankcheck flashram
  uint32_t blankcheck(uint8_t flashramType)
  {
    writeErrors = 0;

    uint16_t offset = 512;
    uint16_t bufferSize = 512;

    if (flashramType == 2)
    {
      offset = 64;
      bufferSize = 128;
    }

    // Put flashram into read mode
    // FRAM_READ_MODE_CMD
    sendFramCmd(0xF0000000);

    // Read Flashram
    for (uint32_t currByte = kSaveAddress; currByte < (kSaveAddress + (131072 / flashramType)); currByte += offset)
    {
      // Set the address for the next 512 bytes
      setAddress(currByte);

      for (uint16_t c = 0; c < bufferSize; c += 2)
      {
        // split word
        uint16_t myWord = readWord();
        uint8_t loByte = myWord & 0xFF;
        uint8_t hiByte = myWord >> 8;

        // write to buffer
        OSCR::Storage::Shared::buffer[c] = hiByte;
        OSCR::Storage::Shared::buffer[c + 1] = loByte;
      }

      // Check buffer content against file on sd card
      for (uint16_t i = 0; i < bufferSize; i++)
      {
        if (0xFF != OSCR::Storage::Shared::buffer[i])
        {
          writeErrors++;
        }
      }
    }

    // Return 0 if verified ok, or number of errors
    return writeErrors;
  }

  // Wait until current operation is done
  uint8_t waitForFram(uint8_t flashramType)
  {
    uint8_t framStatus = 0;
    uint8_t statusMXL1100[] = { 0x11, 0x11, 0x80, 0x01, 0x00, 0xC2, 0x00, 0x1E };
    uint8_t statusMXL1101[] = { 0x11, 0x11, 0x80, 0x01, 0x00, 0xC2, 0x00, 0x1D };
    uint8_t statusMN63F81[] = { 0x11, 0x11, 0x80, 0x01, 0x00, 0x32, 0x00, 0xF1 };

    // FRAM_STATUS_MODE_CMD
    sendFramCmd(0xE1000000);
    delay(1);

    // Set address to Fram status register
    setAddress(0x08000000);

    // Read Status
    for (uint8_t c = 0; c < 8; c += 2) {
      // split word
      uint16_t myWord = readWord();
      uint8_t loByte = myWord & 0xFF;
      uint8_t hiByte = myWord >> 8;

      // write to buffer
      OSCR::Storage::Shared::buffer[c] = hiByte;
      OSCR::Storage::Shared::buffer[c + 1] = loByte;
    }

    if (flashramType == 2) {
      for (uint8_t c = 0; c < 8; c++) {
        if (statusMXL1100[c] != OSCR::Storage::Shared::buffer[c]) {
          framStatus = 1;
        }
      }
    } else if (flashramType == 1) {
      //MX29L1101
      if (MN63F81MPN == false) {
        for (uint8_t c = 0; c < 8; c++) {
          if (statusMXL1101[c] != OSCR::Storage::Shared::buffer[c]) {
            framStatus = 1;
          }
        }
      }
      //MN63F81MPN
      else if (MN63F81MPN == true) {
        for (uint8_t c = 0; c < 8; c++) {
          if (statusMN63F81[c] != OSCR::Storage::Shared::buffer[c]) {
            framStatus = 1;
          }
        }
      }
    }
    // Pull ale_H(PC1) high
    PORTC |= (1 << 1);
    return framStatus;
  }

  // Get flashram type
  uint8_t getFramType()
  {
    uint8_t type = 0;
    char const * name;

    // FRAM_STATUS_MODE_CMD
    sendFramCmd(0xE1000000);
    delay(10);

    // Set address to Fram status register
    setAddress(0x08000000);

    // Read Status
    for (uint8_t c = 0; c < 8; c += 2)
    {
      // split word
      uint16_t myWord = readWord();
      uint8_t loByte = myWord & 0xFF;
      uint8_t hiByte = myWord >> 8;

      // write to buffer
      OSCR::Storage::Shared::buffer[c] = hiByte;
      OSCR::Storage::Shared::buffer[c + 1] = loByte;
    }

    switch (OSCR::Storage::Shared::buffer[7])
    {
    case 0x1E:
      type = 2;
      name = PSTR("MX29L1100");
      break;

    case 0x1D:
      type = 1;
      name = PSTR("MX29L1101");
      MN63F81MPN = false;
      break;

    case 0xF1:
      type = 1;
      name = PSTR("MN63F81MPN");
      MN63F81MPN = true;
      break;

    case 0x8E:
    case 0x84:
      type = 1;
      name = PSTR("29L1100KC-15B0");
      MN63F81MPN = false;
      break;

    default:
      for (uint8_t c = 0; c < 8; c++)
      {
        OSCR::UI::printHex<false>(OSCR::Storage::Shared::buffer[c]);
        OSCR::UI::print(FS(OSCR::Strings::Symbol::Space));
      }

      OSCR::UI::printLine();

      OSCR::UI::error(FS(OSCR::Strings::Errors::UnknownType));
      return 0;
    }

    OSCR::UI::printType(OSCR::Strings::Common::Flash, name);

    // Pull ale_H(PC1) high
    PORTC |= (1 << 1);

    return type;
  }

  /******************************************
    Rom functions
  *****************************************/
  // Read rom and save to the SD card
  void readRom()
  {
    //uint8_t n64buffer[1024];
    uint32_t realSize = (cartSize * 1024 * 1024);
    uint32_t offsetSize = romBase + realSize;

    printHeader();

    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::N64F), FS(OSCR::Strings::Directory::ROM), fileName, FS(OSCR::Strings::FileType::N64));

    cartOn();

    OSCR::UI::ProgressBar::init(realSize, 1);

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Reading));

    OSCR::Time::startMeasure();

    for (uint32_t currByte = romBase; currByte < offsetSize; currByte += 512)
    {
      setAddress(currByte); // Set the address for the first 512 bytes to dump

      NOP; // Wait 62.5ns (safety)

      for (size_t c = 0; c < 512; c += 2)
      {
        // Pull read(PH6) low
        PORTH &= ~(1 << 6);

        // Wait ~310ns
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;

        // data on PINK and PINF is valid now, read into sd card buffer
        OSCR::Storage::Shared::buffer[c] = PINK;      // hiByte
        OSCR::Storage::Shared::buffer[c + 1] = PINF;  // loByte

        // Pull read(PH6) high
        PORTH |= (1 << 6);
      }

      OSCR::Storage::Shared::writeBuffer();
      OSCR::UI::ProgressBar::advance(512);
    }

    OSCR::Time::endMeasure();

    cartOff();

    OSCR::Storage::Shared::close();

    OSCR::UI::printLineSync(FS(OSCR::Strings::Common::DONE));

    OSCR::UI::ProgressBar::finish();

    OSCR::Time::printDifference();

    OSCR::Databases::Extended::matchCRC();
  }

# if HAS_FLASH
  /******************************************
     N64 Repro Flashrom Functions
  *****************************************/
  void flashRepro()
  {
    uint32_t sectorSize = 0;
    uint8_t bufferSize = 0;

    cartOn();

    // Check flashrom ID's
    idFlashrom();

    // If the ID is known continue
    if (cartSize != 0)
    {
      // Print flashrom name
      if ((flashid == 0x227E) && (strcmp_P(cartID, PSTR("2201")) == 0))
      {
        OSCR::UI::print(F("Spansion S29GL256N"));
        if (cartSize == 64)
          OSCR::UI::printLine(F(" x2"));
        else
          OSCR::UI::printLine();
      } else if ((flashid == 0x227E) && (strcmp_P(cartID, PSTR("2101")) == 0)) {
        OSCR::UI::print(F("Spansion S29GL128N"));
      } else if ((flashid == 0x227E) && (strcmp_P(cartID, PSTR("2100")) == 0)) {
        OSCR::UI::print(F("ST M29W128GL"));
      } else if ((flashid == 0x22C9) || (flashid == 0x22CB)) {
        OSCR::UI::print(F("Macronix MX29LV640"));
        if (cartSize == 16)
          OSCR::UI::printLine(F(" x2"));
        else
          OSCR::UI::printLine();
      } else if (flashid == 0x8816)
        OSCR::UI::printLine(F("Intel 4400L0ZDQ0"));
      else if (flashid == 0x7E7E)
        OSCR::UI::printLine(F("Fujitsu MSP55LV100S"));
      else if ((flashid == 0x227E) && (strcmp_P(cartID, PSTR("2301")) == 0))
        OSCR::UI::printLine(F("Fujitsu MSP55LV512"));
      else if ((flashid == 0x227E) && (strcmp_P(cartID, PSTR("3901")) == 0))
        OSCR::UI::printLine(F("Intel 512M29EW"));

      // Print info
      OSCR::UI::printLabel(OSCR::Strings::Common::ID);
      OSCR::UI::printHex(flashid);

      OSCR::UI::printSize(OSCR::Strings::Common::Flash, cartSize * 1024 * 1024);

      OSCR::UI::waitButton();

      if (!OSCR::Prompts::confirmErase())
      {
        cartOff();
        return;
      }
    }
    else
    {
      OSCR::UI::printLine(FS(OSCR::Strings::Common::Unknown));
      OSCR::UI::printValue(OSCR::Strings::Common::Vendor, vendorID);

      OSCR::UI::printLabel(OSCR::Strings::Common::ID);
      OSCR::UI::printHex(flashid);

      OSCR::UI::printLabel(OSCR::Strings::MenuOptions::Cartridge);
      OSCR::UI::printLine(cartID);

      OSCR::UI::wait();

      if (!OSCR::Prompts::confirmErase())
      {
        cartOff();
        return;
      }

      // clear IDs
      sprintf(vendorID, "%s", "CONF");
      flashid = 0;
      sprintf(cartID, "%s", "CONF");

      constexpr uint8_t const BufferSizes[] = {
        0, // no buffer
        32,
        64,
        128,
      };

      constexpr uint8_t const SectorSizes[] = {
        8,
        32,
        64,
        128,
      };

      cartSize = menuOptionsROM[OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectCartSize), FS(OSCR::Strings::Templates::SizeMB), menuOptionsROM, sizeofarray(menuOptionsROM))];

      bufferSize = BufferSizes[OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectBufferSize), FS(OSCR::Strings::Templates::SizeBytes), BufferSizes, sizeofarray(BufferSizes))];

      sectorSize = 1 << SectorSizes[OSCR::UI::menu(FS(OSCR::Strings::Headings::SelectBufferSize), FS(OSCR::Strings::Templates::SizeKB), BufferSizes, sizeofarray(BufferSizes))] * 1024;
    }

    OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

    // Get rom size from file
    fileSize = OSCR::Storage::Shared::getSize();

    OSCR::UI::printSize(OSCR::Strings::Common::ROM, fileSize);

    // Compare file size to flashrom size
    if ((fileSize / 1048576) > cartSize)
    {
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotLargeEnough));
    }

    // Erase needed sectors
    if (flashid == 0x227E)
    {
      // Spansion S29GL256N or Fujitsu MSP55LV512 with 0x20000 sector size and 32 byte buffer
      eraseSector(0x20000);
    }
    else if (flashid == 0x7E7E)
    {
      // Fujitsu MSP55LV100S
      eraseMSP55LV100();
    }
    else if ((flashid == 0x8813) || (flashid == 0x8816))
    {
      // Intel 4400L0ZDQ0
      eraseIntel4400();
      resetIntel4400();
    }
    else if ((flashid == 0x22C9) || (flashid == 0x22CB))
    {
      // Macronix MX29LV640, C9 is top boot and CB is bottom boot block
      eraseSector(0x8000);
    }
    else
    {
      eraseFlashrom();
    }

    // Check if erase was successful
    if (blankcheckFlashrom())
    {
      // Write flashrom
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

      if ((strcmp_P(cartID, PSTR("3901")) == 0) && (flashid == 0x227E))
      {
        // Intel 512M29EW(64MB) with 0x20000 sector size and 128 byte buffer
        writeFlashBuffer(0x20000, 128);
      }
      else if ((strcmp_P(cartID, PSTR("2100")) == 0) && (flashid == 0x227E))
      {
        // ST M29W128GH(16MB) with 0x20000 sector size and 64 byte buffer
        writeFlashBuffer(0x20000, 64);
      }
      else if (flashid == 0x227E)
      {
        // Spansion S29GL128N/S29GL256N or Fujitsu MSP55LV512 with 0x20000 sector size and 32 byte buffer
        writeFlashBuffer(0x20000, 32);
      }
      else if (flashid == 0x7E7E)
      {
        //Fujitsu MSP55LV100S
        writeMSP55LV100(0x20000);
      }
      else if ((flashid == 0x22C9) || (flashid == 0x22CB))
      {
        // Macronix MX29LV640 without buffer and 0x8000 sector size
        writeFlashrom(0x8000);
      }
      else if ((flashid == 0x8813) || (flashid == 0x8816))
      {
        // Intel 4400L0ZDQ0
        writeIntel4400();
        resetIntel4400();
      }
      else if (sectorSize)
      {
        if (bufferSize)
        {
          writeFlashBuffer(sectorSize, bufferSize);
        }
        else
        {
          writeFlashrom(sectorSize);
        }
      }
      else
      {
        OSCR::UI::fatalError(F("sectorSize not set"));
      }

      // Close the file:
      OSCR::Storage::Shared::close();

      // Verify
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Verifying));

      writeErrors = verifyFlashrom();

      cartOff();

      if (writeErrors != 0)
      {
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
      }
    }
    else
    {
      cartOff();

      // Close the file
      OSCR::Storage::Shared::close();

      OSCR::UI::error(FS(OSCR::Strings::Common::FAIL));
    }

    OSCR::UI::waitButton();
  }

  // Reset to read mode
  void resetIntel4400()
  {
    for (uint32_t currPartition = 0; currPartition < (cartSize * 0x100000); currPartition += 0x20000) {
      setAddress(romBase + currPartition);
      writeWord(0xFF);
    }
  }

  // Reset Fujitsu MSP55LV100S
  void resetMSP55LV100(uint32_t flashBase)
  {
    // Send reset Command
    setAddress(flashBase);
    writeWord(0xF0F0);
    delay(100);
  }

  // Common reset command
  void resetFlashrom(uint32_t flashBase)
  {
    // Send reset Command
    setAddress(flashBase);
    writeWord(0xF0);
    delay(100);
  }

  void sendFlashromCommand(uint32_t addr, uint8_t cmd)
  {
    setAddress(addr + (0x555 << 1));
    writeWord(0xAA);
    setAddress(addr + (0x2AA << 1));
    writeWord(0x55);
    setAddress(addr + (0x555 << 1));
    writeWord(cmd);
  }

  void idFlashrom()
  {
    // Set size to 0 if no ID is found
    cartSize = 0;

    // Send flashrom ID command
    sendFlashromCommand(romBase, 0x90);

    // Read 1 byte vendor ID
    setAddress(romBase);
    sprintf(vendorID, "%02X", readWord());
    // Read 2 bytes flashrom ID
    flashid = readWord();
    // Read 2 bytes secondary flashrom ID
    setAddress(romBase + 0x1C);
    sprintf(cartID, "%04X", ((readWord() << 8) | (readWord() & 0xFF)));

    // Spansion S29GL256N(32MB/64MB) with either one or two flashrom chips
    if ((strcmp_P(cartID, PSTR("2201")) == 0) && (flashid == 0x227E)) {
      cartSize = 32;

      // Reset flashrom
      resetFlashrom(romBase);

      // Test for second flashrom chip at 0x2000000 (32MB)
      sendFlashromCommand(romBase + 0x2000000, 0x90);

      char tempID[5];
      setAddress(romBase + 0x2000000);
      // Read manufacturer ID
      readWord();
      // Read flashrom ID
      sprintf(tempID, "%04X", readWord());

      // Check if second flashrom chip is present
      if (strcmp_P(tempID, PSTR("227E")) == 0)
      {
        cartSize = 64;
      }
      resetFlashrom(romBase + 0x2000000);
    }

    // Macronix MX29LV640(8MB/16MB) with either one or two flashrom chips
    else if ((flashid == 0x22C9) || (flashid == 0x22CB)) {
      cartSize = 8;

      resetFlashrom(romBase + 0x800000);

      // Test for second flashrom chip at 0x800000 (8MB)
      sendFlashromCommand(romBase + 0x800000, 0x90);

      char tempID[5];
      setAddress(romBase + 0x800000);
      // Read manufacturer ID
      readWord();
      // Read flashrom ID
      sprintf(tempID, "%04X", readWord());

      // Check if second flashrom chip is present
      if ((strcmp_P(tempID, PSTR("22C9")) == 0) || (strcmp_P(tempID, PSTR("22CB")) == 0))
      {
        cartSize = 16;
      }
      resetFlashrom(romBase + 0x800000);
    }

    // Intel 4400L0ZDQ0 (64MB)
    else if (flashid == 0x8816) {
      // Found first flashrom chip, set to 32MB
      cartSize = 32;
      resetIntel4400();

      // Test if second half of the flashrom might be hidden
      sendFlashromCommand(romBase + 0x2000000, 0x90);

      // Read manufacturer ID
      setAddress(romBase + 0x2000000);
      readWord();
      // Read flashrom ID
      sprintf(cartID, "%04X", readWord());
      if (strcmp_P(cartID, PSTR("8813")) == 0)
      {
        cartSize = 64;
        flashid = 0x8813;
      }
      resetIntel4400();
      // Empty cartID string
      cartID[0] = '\0';
    }

    //Fujitsu MSP55LV512/Spansion S29GL512N (64MB)
    else if ((strcmp_P(cartID, PSTR("2301")) == 0) && (flashid == 0x227E))
    {
      cartSize = 64;
      // Reset flashrom
      resetFlashrom(romBase);
    }

    // Spansion S29GL128N(16MB) with one flashrom chip
    else if ((strcmp_P(cartID, PSTR("2101")) == 0) && (flashid == 0x227E))
    {
      cartSize = 16;
      // Reset flashrom
      resetFlashrom(romBase);
    }

    // ST M29W128GL(16MB) with one flashrom chip
    else if ((strcmp_P(cartID, PSTR("2100")) == 0) && (flashid == 0x227E))
    {
      cartSize = 16;
      // Reset flashrom
      resetFlashrom(romBase);
    }

    // Intel 512M29EW(64MB) with one flashrom chip
    else if ((strcmp_P(cartID, PSTR("3901")) == 0) && (flashid == 0x227E))
    {
      cartSize = 64;
      // Reset flashrom
      resetFlashrom(romBase);
    }

    // Unknown 227E type
    else if (flashid == 0x227E) {
      cartSize = 0;
      // Reset flashrom
      resetFlashrom(romBase);
    }

    //Test for Fujitsu MSP55LV100S (64MB)
    else
    {
      // Send flashrom ID command
      setAddress(romBase + (0x555 << 1));
      writeWord(0xAAAA);
      setAddress(romBase + (0x2AA << 1));
      writeWord(0x5555);
      setAddress(romBase + (0x555 << 1));
      writeWord(0x9090);

      setAddress(romBase);
      // Read 1 byte vendor ID
      readWord();
      // Read 2 bytes flashrom ID
      sprintf(cartID, "%04X", readWord());

      if (strcmp_P(cartID, PSTR("7E7E")) == 0)
      {
        resetMSP55LV100(romBase);
        cartSize = 64;
        flashid = 0x7E7E;
      }
    }
    if ((flashid == 0x1240) && (strcmp_P(cartID, PSTR("1240")) == 0))
    {
      OSCR::UI::fatalError(F("Please reseat cartridge"));
    }
  }

  // Erase Intel flashrom
  void eraseIntel4400()
  {
    uint32_t flashBase = romBase;

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    // If the game is smaller than 32Mbit only erase the needed blocks
    uint32_t lastBlock = 0x1FFFFFF;
    if (fileSize < 0x1FFFFFF)
      lastBlock = fileSize;

    // Erase 4 blocks with 16kwords each
    for (uint32_t currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
      // Unlock block command
      setAddress(flashBase + currBlock);
      writeWord(0x60);
      setAddress(flashBase + currBlock);
      writeWord(0xD0);
      // Erase command
      setAddress(flashBase + currBlock);
      writeWord(0x20);
      setAddress(flashBase + currBlock);
      writeWord(0xD0);

      // Read the status register
      setAddress(flashBase + currBlock);
      uint16_t statusReg = readWord();
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        setAddress(flashBase + currBlock);
        statusReg = readWord();
      }
    }

    // Erase up to 255 blocks with 64kwords each
    for (uint32_t currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF) {
      // Unlock block command
      setAddress(flashBase + currBlock);
      writeWord(0x60);
      setAddress(flashBase + currBlock);
      writeWord(0xD0);
      // Erase command
      setAddress(flashBase + currBlock);
      writeWord(0x20);
      setAddress(flashBase + currBlock);
      writeWord(0xD0);

      // Read the status register
      setAddress(flashBase + currBlock);
      uint16_t statusReg = readWord();
      while ((statusReg | 0xFF7F) != 0xFFFF)
      {
        setAddress(flashBase + currBlock);
        statusReg = readWord();
      }
    }

    // Check if we should erase the second chip too
    if ((cartSize = 64) && (fileSize > 0x2000000)) {
      // Switch base address to second chip
      flashBase = romBase + 0x2000000;

      // 255 blocks with 64kwords each
      for (uint32_t currBlock = 0x0; currBlock < 0x1FDFFFF; currBlock += 0x1FFFF) {
        // Unlock block command
        setAddress(flashBase + currBlock);
        writeWord(0x60);
        setAddress(flashBase + currBlock);
        writeWord(0xD0);
        // Erase command
        setAddress(flashBase + currBlock);
        writeWord(0x20);
        setAddress(flashBase + currBlock);
        writeWord(0xD0);

        // Read the status register
        setAddress(flashBase + currBlock);
        uint16_t statusReg = readWord();
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          setAddress(flashBase + currBlock);
          statusReg = readWord();
        }
      }

      // 4 blocks with 16kword each
      for (uint32_t currBlock = 0x1FE0000; currBlock < 0x1FFFFFF; currBlock += 0x8000) {
        // Unlock block command
        setAddress(flashBase + currBlock);
        writeWord(0x60);
        setAddress(flashBase + currBlock);
        writeWord(0xD0);
        // Erase command
        setAddress(flashBase + currBlock);
        writeWord(0x20);
        setAddress(flashBase + currBlock);
        writeWord(0xD0);

        // Read the status register
        setAddress(flashBase + currBlock);
        uint16_t statusReg = readWord();
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          setAddress(flashBase + currBlock);
          statusReg = readWord();
        }
      }
    }
  }

  // Erase Fujutsu MSP55LV100S
  void eraseMSP55LV100()
  {
    uint32_t flashBase = romBase;
    uint32_t sectorSize = 0x20000;

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    for (uint32_t currSector = 0; currSector < fileSize; currSector += sectorSize)
    {
      // Send Erase Command to first chip
      setAddress(flashBase + (0x555 << 1));
      writeWord(0xAAAA);
      setAddress(flashBase + (0x2AA << 1));
      writeWord(0x5555);
      setAddress(flashBase + (0x555 << 1));
      writeWord(0x8080);
      setAddress(flashBase + (0x555 << 1));
      writeWord(0xAAAA);
      setAddress(flashBase + (0x2AA << 1));
      writeWord(0x5555);
      setAddress(romBase + currSector);
      writeWord(0x3030);

      // Read the status register
      setAddress(romBase + currSector);
      uint16_t statusReg = readWord();
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        setAddress(romBase + currSector);
        statusReg = readWord();
      }

      // Read the status register
      setAddress(romBase + currSector);
      statusReg = readWord();
      while ((statusReg | 0x7FFF) != 0xFFFF) {
        setAddress(romBase + currSector);
        statusReg = readWord();
      }
    }
  }

  // Common chip erase command
  void eraseFlashrom()
  {
    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    // Send Erase Command
    sendFlashromCommand(romBase, 0x80);
    sendFlashromCommand(romBase, 0x10);

    // Read the status register
    setAddress(romBase);

    uint16_t statusReg = readWord();

    while ((statusReg | 0xFF7F) != 0xFFFF)
    {
      setAddress(romBase);
      statusReg = readWord();
      delay(500);
    }
  }

  // Common sector erase command
  void eraseSector(uint32_t sectorSize)
  {
    uint32_t flashBase = romBase;

    OSCR::UI::printSync(FS(OSCR::Strings::Status::Erasing));

    for (uint32_t currSector = 0; currSector < fileSize; currSector += sectorSize)
    {
      // Spansion S29GL256N(32MB/64MB) with two flashrom chips
      if ((currSector == 0x2000000) && (strcmp_P(cartID, PSTR("2201")) == 0) && (flashid == 0x227E))
      {
        // Change to second chip
        flashBase = romBase + 0x2000000;
      }
      // Macronix MX29LV640(8MB/16MB) with two flashrom chips
      else if ((currSector == 0x800000) && ((flashid == 0x22C9) || (flashid == 0x22CB)))
      {
        flashBase = romBase + 0x800000;
      }

      // Send Erase Command
      sendFlashromCommand(flashBase, 0x80);
      setAddress(flashBase + (0x555 << 1));
      writeWord(0xAA);
      setAddress(flashBase + (0x2AA << 1));
      writeWord(0x55);
      setAddress(romBase + currSector);
      writeWord(0x30);


      // Read the status register
      setAddress(romBase + currSector);
      uint16_t statusReg = readWord();
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        setAddress(romBase + currSector);
        statusReg = readWord();
      }
    }
  }

  bool blankcheckFlashrom()
  {
    for (uint32_t currByte = romBase; currByte < romBase + fileSize; currByte += 512)
    {
      // Set the address
      setAddress(currByte);

      for (uint16_t c = 0; c < 512; c += 2)
      {
        if (readWord() != 0xFFFF)
        {
          return 0;
        }
      }
    }
    return 1;
  }

  // Write Intel flashrom
  void writeIntel4400()
  {
    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currSector = 0; currSector < fileSize; currSector += 131072)
    {
      // Write to flashrom
      for (uint32_t pos = 0; pos < 131072; pos += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::readBuffer();

        // Write 32 words at a time
        for (uint16_t currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64)
        {
          // Buffered program command
          setAddress(romBase + currSector + pos + currWriteBuffer);
          writeWord(0xE8);

          // Check Status register
          setAddress(romBase + currSector + pos + currWriteBuffer);
          uint16_t statusReg = readWord();

          while ((statusReg | 0xFF7F) != 0xFFFF)
          {
            setAddress(romBase + currSector + pos + currWriteBuffer);
            statusReg = readWord();
          }

          // Write word count (minus 1)
          setAddress(romBase + currSector + pos + currWriteBuffer);
          writeWord(0x1F);

          // Write buffer
          for (uint8_t currByte = 0; currByte < 64; currByte += 2)
          {
            // Join two bytes into one word
            uint16_t currWord = ((OSCR::Storage::Shared::buffer[currWriteBuffer + currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currWriteBuffer + currByte + 1] & 0xFF);
            setAddress(romBase + currSector + pos + currWriteBuffer + currByte);
            writeWord(currWord);
          }

          // Write Buffer to Flash
          setAddress(romBase + currSector + pos + currWriteBuffer + 62);
          writeWord(0xD0);

          // Read the status register at last written address
          setAddress(romBase + currSector + pos + currWriteBuffer + 62);
          statusReg = readWord();

          while ((statusReg | 0xFF7F) != 0xFFFF)
          {
            setAddress(romBase + currSector + pos + currWriteBuffer + 62);
            statusReg = readWord();
          }
        }
        OSCR::UI::ProgressBar::advance(512);
      }
    }

    OSCR::UI::ProgressBar::finish();
  }

  // Write Fujitsu MSP55LV100S flashrom consisting out of two MSP55LV512 flashroms one used for the high byte the other for the low byte
  void writeMSP55LV100(uint32_t sectorSize)
  {
    uint32_t flashBase = romBase;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currSector = 0; currSector < fileSize; currSector += sectorSize)
    {
      // Write to flashrom
      for (uint32_t pos = 0; pos < sectorSize; pos += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::readBuffer();

        // Write 32 bytes at a time
        for (uint16_t currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 32)
        {
          // 2 unlock commands
          setAddress(flashBase + (0x555 << 1));
          writeWord(0xAAAA);
          setAddress(flashBase + (0x2AA << 1));
          writeWord(0x5555);

          // Write buffer load command at sector address
          setAddress(romBase + currSector + pos + currWriteBuffer);
          writeWord(0x2525);

          // Write word count (minus 1) at sector address
          setAddress(romBase + currSector + pos + currWriteBuffer);
          writeWord(0x0F0F);

          // Define variable before loop so we can use it later when reading the status register
          uint16_t currWord;

          for (uint8_t currByte = 0; currByte < 32; currByte += 2)
          {
            // Join two bytes into one word
            currWord = ((OSCR::Storage::Shared::buffer[currWriteBuffer + currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currWriteBuffer + currByte + 1] & 0xFF);

            // Load Buffer Words
            setAddress(romBase + currSector + pos + currWriteBuffer + currByte);
            writeWord(currWord);
          }

          // Write Buffer to Flash
          setAddress(romBase + currSector + pos + currWriteBuffer + 30);
          writeWord(0x2929);

          // Read the status register at last written address
          setAddress(romBase + currSector + pos + currWriteBuffer + 30);
          uint16_t statusReg = readWord();

          while ((statusReg | 0x7F7F) != (currWord | 0x7F7F))
          {
            setAddress(romBase + currSector + pos + currWriteBuffer + 30);
            statusReg = readWord();
          }
        }
        OSCR::UI::ProgressBar::advance(512);
      }
    }

    OSCR::UI::ProgressBar::finish();
  }

  // Write Spansion S29GL256N flashrom using the 32 byte write buffer
  void writeFlashBuffer(uint32_t sectorSize, uint8_t bufferSize)
  {
    uint32_t flashBase = romBase;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currSector = 0; currSector < fileSize; currSector += sectorSize)
    {
      // Spansion S29GL256N(32MB/64MB) with two flashrom chips
      if ((currSector == 0x2000000) && (strcmp_P(cartID, PSTR("2201")) == 0))
      {
        flashBase = romBase + 0x2000000;
      }

      // Write to flashrom
      for (uint32_t pos = 0; pos < sectorSize; pos += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::readBuffer();

        // Write 32 bytes at a time
        for (uint16_t currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += bufferSize)
        {
          // 2 unlock commands
          setAddress(flashBase + (0x555 << 1));
          writeWord(0xAA);
          setAddress(flashBase + (0x2AA << 1));
          writeWord(0x55);

          // Write buffer load command at sector address
          setAddress(romBase + currSector + pos + currWriteBuffer);
          writeWord(0x25);

          // Write word count (minus 1) at sector address
          setAddress(romBase + currSector + pos + currWriteBuffer);
          writeWord((bufferSize / 2) - 1);

          // Define variable before loop so we can use it later when reading the status register
          uint16_t currWord = 0;

          for (uint8_t currByte = 0; currByte < bufferSize; currByte += 2)
          {
            // Join two bytes into one word
            currWord = ((OSCR::Storage::Shared::buffer[currWriteBuffer + currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currWriteBuffer + currByte + 1] & 0xFF);

            // Load Buffer Words
            setAddress(romBase + currSector + pos + currWriteBuffer + currByte);
            writeWord(currWord);
          }

          // Write Buffer to Flash
          setAddress(romBase + currSector + pos + currWriteBuffer + bufferSize - 2);
          writeWord(0x29);

          // Read the status register at last written address
          setAddress(romBase + currSector + pos + currWriteBuffer + bufferSize - 2);
          uint16_t statusReg = readWord();

          while ((statusReg | 0xFF7F) != (currWord | 0xFF7F))
          {
            setAddress(romBase + currSector + pos + currWriteBuffer + bufferSize - 2);
            statusReg = readWord();
          }
        }

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    OSCR::UI::ProgressBar::finish();
  }

  // Write MX29LV640 flashrom without write buffer
  void writeFlashrom(uint32_t sectorSize)
  {
    uint32_t flashBase = romBase;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currSector = 0; currSector < fileSize; currSector += sectorSize)
    {
      // Macronix MX29LV640(8MB/16MB) with two flashrom chips
      if (currSector == 0x800000)
      {
        flashBase = romBase + 0x800000;
      }

      // Write to flashrom
      for (uint32_t pos = 0; pos < sectorSize; pos += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::fill();

        for (uint16_t currByte = 0; currByte < 512; currByte += 2)
        {
          // Join two bytes into one word
          uint16_t currWord = ((OSCR::Storage::Shared::buffer[currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currByte + 1] & 0xFF);

          // 2 unlock commands
          sendFlashromCommand(flashBase, 0xA0);

          // Write word
          setAddress(romBase + currSector + pos + currByte);
          writeWord(currWord);

          // Read the status register
          setAddress(romBase + currSector + pos + currByte);
          uint16_t statusReg = readWord();

          while ((statusReg | 0xFF7F) != (currWord | 0xFF7F))
          {
            setAddress(romBase + currSector + pos + currByte);
            statusReg = readWord();
          }
        }

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    OSCR::UI::ProgressBar::finish();
  }

  uint32_t verifyFlashrom()
  {
    // Open file on sd card
    OSCR::Storage::Shared::openRO();

    writeErrors = 0;

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currSector = 0; currSector < fileSize; currSector += 131072)
    {
      for (uint32_t pos = 0; pos < 131072; pos += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::readBuffer();

        for (uint16_t currByte = 0; currByte < 512; currByte += 2)
        {
          // Join two bytes into one word
          uint16_t currWord = ((OSCR::Storage::Shared::buffer[currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currByte + 1] & 0xFF);

          // Read flash
          setAddress(romBase + currSector + pos + currByte);

          // Compare both
          if (readWord() != currWord)
          {
            writeErrors++;

            // Abort if too many errors
            if (writeErrors > 20)
            {
              OSCR::UI::print(F("More than "));

              OSCR::Storage::Shared::close();

              return writeErrors;
            }
          }
        }
        OSCR::UI::ProgressBar::advance(512);
      }
    }

    OSCR::UI::ProgressBar::finish();

    // Close the file:
    OSCR::Storage::Shared::close();

    return writeErrors;
  }
# endif /* HAS_FLASH */

  /******************************************
     N64 Gameshark Flash Functions
  *****************************************/
  void sendFlashromGamesharkCommand(uint16_t cmd)
  {
    setAddress(0x1EF0AAA8);
    writeWord(0xAAAA);
    setAddress(0x1EE05554);
    writeWord(0x5555);
    setAddress(0x1EF0AAA8);
    writeWord(cmd);
  }

  bool flashGameshark()
  {
    cartOn();

    // Check flashrom ID's
    unlockGSAddressRanges();
    if (!idGameshark()) return false;

    // Check for SST 29LE010 (0808)/SST 28LF040 (0404)/AMTEL AT29LV010A (3535)/SST 29EE010 (0707)
    // !!!!          This has been confirmed to allow reading of v1.02, v1.04-v1.09, v2.0-2.21, v3.0-3.3     !!!!
    // !!!!          All referenced eeproms/flashroms are confirmed as being writable with this process.     !!!!
    // !!!!                                                                                                  !!!!
    // !!!!                                PROCEED AT YOUR OWN RISK                                          !!!!
    // !!!!                                                                                                  !!!!
    // !!!! SST 29EE010 may have a 5V requirement for writing however dumping works at 3V. As such it is not !!!!
    // !!!!        advised to write to a cart with this chip until further testing can be completed.         !!!!

    if (flashid == 0x0808 || flashid == 0x0404 || flashid == 0x3535 || flashid == 0x0707)
    {
      backupGameshark();

      if (!OSCR::Prompts::confirmErase())
      {
        cartOff();
        return false;
      }

      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

      // Get rom size from file
      fileSize = OSCR::Storage::Shared::getSize();

      printHeader();

      OSCR::UI::printSize(OSCR::Strings::Common::Flash, fileSize);

      // Compare file size to flashrom size for 1 Mbit eeproms
      if (fileSize > 262144 && (flashid == 0x0808 || flashid == 0x3535 || flashid == 0x0707))
      {
        OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotLargeEnough));
      }

      // Compare file size to flashrom size for 1 Mbit eeproms
      if (fileSize > 1048576 && flashid == 0x0404)
      {
        OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotLargeEnough));
      }

      // SST 29LE010, chip erase not needed as this eeprom automaticly erases during the write cycle
      eraseGameshark();
      blankCheck();

      // Write flashrom
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

      writeGameshark();

      // Close the file:
      OSCR::Storage::Shared::close();

      // Verify
      OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

      writeErrors = verifyGameshark();

      cartOff();

      printHeader();

      if (writeErrors == 0)
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
      }
      else
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
      }
    }

    return true;
  }

  void unlockGSAddressRanges()
  {
    // This enables using the 0x1EEx_xxxx, 0x1EFx_xxxx, and 0x1ECx_xxxx address ranges, necessary for writing to all supported chips
    setAddress(0x10400400);
    writeWord(0x1E);
    writeWord(0x1E);
  }

  //Test for SST 29LE010  or SST 28LF040 (0404) or AMTEL AT29LV010A (3535) or SST 29EE010 (0707)
  bool idGameshark()
  {
    flashid = 0x0;
    //Send flashrom ID command
    sendFlashromGamesharkCommand(0x9090);

    setAddress(0x1EC00000);
    // Read 1 byte vendor ID
    readWord();
    // Read 2 bytes flashrom ID
    flashid = readWord();

    if (flashid == 0x0808 || flashid == 0x3535 || flashid == 0x0707)
    {
      flashSize = 262144;
    }
    else if (flashid == 0x0404) {
      //Set SST 28LF040 flashrom size
      flashSize = 1048574;
    }
    else
    {
      OSCR::UI::printLine(F("Check cart connection"));
      OSCR::UI::printLine(F("Unknown Flash ID"));
      return false;
    }

    // Reset flashrom
    resetGameshark();

    return true;
  }

  void resetGameshark()
  {
    if (flashid == 0x0808 || flashid == 0x3535 || flashid == 0x0707) {
      // Send reset command for SST 29LE010 / AMTEL AT29LV010A / SST 29EE010
      sendFlashromGamesharkCommand(0xF0F0);
      delay(100);
    } else if (flashid == 0x0404) {
      // Send reset command for SST 28LF040
      sendFlashromGamesharkCommand(0xFFFF);
      delay(300);
    }
  }

  // Read rom and save to the SD card
  void backupGameshark()
  {
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::N64F), FS(OSCR::Strings::Directory::ROM), "GameShark", FS(OSCR::Strings::FileType::N64));

    for (uint32_t currByte = romBase + 0xEC00000; currByte < (romBase + 0xEC00000 + flashSize); currByte += 512)
    {
      // Set the address for the next 512 bytes
      setAddress(currByte);

      for (uint16_t c = 0; c < 512; c += 2)
      {
        // split word
        uint16_t myWord = readWord();

        // write to buffer
        OSCR::Storage::Shared::buffer[c] = myWord >> 8;
        OSCR::Storage::Shared::buffer[c + 1] = myWord & 0xFF;
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    OSCR::Storage::Shared::close();
  }

  void eraseGameshark()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    // Send chip erase to SST 29LE010 / AMTEL AT29LV010A / SST 29EE010
    if (flashid == 0x0808 || flashid == 0x3535 || flashid == 0x0707) {
      sendFlashromGamesharkCommand(0x8080);
      sendFlashromGamesharkCommand(0x1010);

      delay(20);
    }

    if (flashid == 0x0404) {
      //Unprotect flash
      setAddress(0x1EF03044);
      readWord();
      setAddress(0x1EE03040);
      readWord();
      setAddress(0x1EE03044);
      readWord();
      setAddress(0x1EE00830);
      readWord();
      setAddress(0x1EF00834);
      readWord();
      setAddress(0x1EF00830);
      readWord();
      setAddress(0x1EE00834);
      readWord();
      delay(1000);

      //Erase flash
      sendFlashromGamesharkCommand(0x3030);
      setAddress(0x1EF0AAA8);
      writeWord(0x3030);
      delay(1000);
    }
  }

  void blankCheck()
  {
    // Blankcheck
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Verifying));

    for (uint32_t currSector = 0; currSector < flashSize; currSector += 131072)
    {
      for (uint32_t pos = 0; pos < 131072; pos += 512)
      {
        for (uint16_t currByte = 0; currByte < 512; currByte += 2)
        {
          // Read flash
          setAddress(romBase + 0xEC00000 + currSector + pos + currByte);

          // Compare both
          if (readWord() != 0xFFFF)
          {
            OSCR::UI::fatalError(F("Erase failed"));
          }
        }
      }
    }
  }

  void writeGameshark()
  {
    // Write Gameshark with 2x SST 29LE010 / AMTEL AT29LV010A / SST 29EE010 Eeproms
    if (flashid == 0x0808 || flashid == 0x3535 || flashid == 0x0707)
    {
      // Each 29LE010 has 1024 pages, each 128 bytes in size

      //Initialize progress bar
      OSCR::UI::ProgressBar::init(fileSize);

      OSCR::Storage::Shared::rewind();

      for (uint32_t currPage = 0; currPage < fileSize / 2; currPage += 128)
      {
        // Fill SD buffer with twice the amount since we flash 2 chips
        OSCR::Storage::Shared::readBuffer(256);

        //Send page write command to both flashroms
        sendFlashromGamesharkCommand(0xA0A0);

        // Write 1 page each, one flashrom gets the low byte, the other the high byte.
        for (uint32_t currByte = 0; currByte < 256; currByte += 2)
        {
          // Set address
          setAddress(romBase + 0xEC00000 + (currPage * 2) + currByte);

          // Join two bytes into one word
          uint16_t currWord = ((OSCR::Storage::Shared::buffer[currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currByte + 1] & 0xFF);

          // Send byte data
          writeWord(currWord);
        }

        OSCR::UI::ProgressBar::advance(256);

        delay(30);
      }

      OSCR::UI::ProgressBar::finish();
    }

    // Write Gameshark with 2x SST 28LF040
    if (flashid == 0x0404)
    {
      //Initialize progress bar
      OSCR::UI::ProgressBar::init(fileSize);

      OSCR::Storage::Shared::rewind();

      for (uint32_t currSector = 0; currSector < fileSize; currSector += 16384)
      {
        for (uint32_t pos = 0; pos < 16384; pos += 256)
        {
          OSCR::Storage::Shared::readBuffer(256);

          for (uint32_t currByte = 0; currByte < 256; currByte += 2)
          {
            // Send byte program command
            sendFlashromGamesharkCommand(0x1010);

            // Set address
            setAddress(romBase + 0xEC00000 + currSector + pos + currByte);

            // Join two bytes into one word
            uint16_t currWord = ((OSCR::Storage::Shared::buffer[currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currByte + 1] & 0xFF);

            // Send byte data
            writeWord(currWord);
            delayMicroseconds(60);
          }

          OSCR::UI::ProgressBar::advance(256);
        }
      }

      //Protect flash
      setAddress(0x1EF03044);
      readWord();
      setAddress(0x1EE03040);
      readWord();
      setAddress(0x1EE03044);
      readWord();
      setAddress(0x1EE00830);
      readWord();
      setAddress(0x1EF00834);
      readWord();
      setAddress(0x1EF00830);
      readWord();
      setAddress(0x1EE00814);
      readWord();
      delay(1000);

      OSCR::UI::ProgressBar::finish();
    }
  }

  uint32_t verifyGameshark()
  {
    OSCR::UI::ProgressBar::init(fileSize);

    // Open file on sd card
    OSCR::Storage::Shared::openRO();
    writeErrors = 0;

    for (uint32_t currSector = 0; currSector < fileSize; currSector += 131072)
    {
      for (uint32_t pos = 0; pos < 131072; pos += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::readBuffer();

        for (uint16_t currByte = 0; currByte < 512; currByte += 2)
        {
          // Join two bytes into one word
          uint16_t currWord = ((OSCR::Storage::Shared::buffer[currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currByte + 1] & 0xFF);

          // Read flash
          setAddress(romBase + 0xEC00000 + currSector + pos + currByte);

          // Compare both
          if (readWord() != currWord)
          {
            writeErrors++;
          }
        }

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    OSCR::UI::ProgressBar::finish();

    // Close the file:
    OSCR::Storage::Shared::close();

    return writeErrors;
  }

  /******************************************
     XPLORER 64 Functions
  *****************************************/
  void sendFlashromXplorerCommand(uint16_t cmd)
  {
    oddXPaddrWrite(0x1040AAAA, 0xAAAA);
    evenXPaddrWrite(0x10405555, 0x5555);
    oddXPaddrWrite(0x1040AAAA, cmd);
  }

  void flashXplorer()
  {
    cartOn();

    // Check flashrom ID's
    if (!idXplorer()) return;

    if (flashid == 0x0808)
    {
      backupXplorer();

      if (!OSCR::Prompts::confirmErase()) return;

      OSCR::selectFile(FS(OSCR::Strings::MenuOptions::SelectFile), O_RDONLY);

      // Get rom size from file
      fileSize = OSCR::Storage::Shared::getSize();

      OSCR::UI::printSize(OSCR::Strings::Common::Flash, fileSize);

      // Compare file size to flashrom size
      if (fileSize > 262144)
      {
        OSCR::UI::fatalError(FS(OSCR::Strings::Errors::NotLargeEnough));
      }

      // SST 29LE010, chip erase not needed as this eeprom automaticly erases during the write cycle
      eraseXplorer();
      blankCheck_XP64();

      // Write flashrom

      printHeader();
      OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Writing));

      writeXplorer();

      // Close the file:
      OSCR::Storage::Shared::close();

      // Verify

      OSCR::UI::printSync(FS(OSCR::Strings::Status::Verifying));

      writeErrors = verifyXplorer();

      cartOff();

      printHeader();

      if (writeErrors == 0)
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Common::OK));
        OSCR::UI::waitButton();
      }
      else
      {
        OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
        OSCR::Lang::printErrorVerifyBytes(writeErrors);
      }
    }

  }

  //Test for SST 29LE010
  bool idXplorer()
  {
    flashid = 0x0;
    //Send flashrom ID command
    sendFlashromXplorerCommand(0x9090);

    setAddress(0x10760000);
    readWord();
    setAddress(0x10400D88);
    flashid = readWord();
    setAddress(0x10740000);
    readWord();

    if (flashid != 0x0808)
    {
      OSCR::UI::printLine(F("Check cart connection"));
      OSCR::UI::printLine(F("Unknown Flash ID"));
      OSCR::UI::waitButton();
      return false;
    }

    // Reset flashrom
    resetXplorer();

    return true;
  }

  void resetXplorer()
  {
    // Send reset command for SST 29LE010
    sendFlashromXplorerCommand(0xF0F0);
    delay(100);
  }

  // Read rom and save to the SD card
  void backupXplorer()
  {
    // create a new folder
    OSCR::Storage::Shared::createFile(FS(OSCR::Strings::FileType::N64F), FS(OSCR::Strings::Directory::ROM), "XPLORER64", FS(OSCR::Strings::FileType::N64));

    for (uint32_t currByte = 0x10400000; currByte <= 0x1043FFFF; currByte += 512)
    {
      // Set the address for the next 512 bytes
      setAddress(currByte);

      for (uint16_t c = 0; c < 512; c += 2)
      {
        // split word
        uint16_t myWord = readWord();

        // write to buffer
        OSCR::Storage::Shared::buffer[c] = myWord >> 8;
        OSCR::Storage::Shared::buffer[c + 1] = myWord & 0xFF;
      }

      OSCR::Storage::Shared::writeBuffer();
    }

    OSCR::Storage::Shared::close();

    OSCR::UI::printLine(FS(OSCR::Strings::Common::DONE));
  }

  uint32_t unscramble(uint32_t addr)
  {
    uint32_t result = (((addr >> 4) & 0x001) | ((addr >> 8) & 0x002) | ((~addr >> 9) & 0x004) | ((addr >> 3) & 0x008) | ((addr >> 6) & 0x010) | ((addr >> 2) & 0x020) | ((~addr << 5) & 0x0C0) | ((~addr << 8) & 0x100) | ((~addr << 6) & 0x200) | ((~addr << 2) & 0x400) | ((addr << 6) & 0x800) | (addr & 0x1F000));

    return result;
  }


  uint32_t scramble(uint32_t addr)
  {
    uint32_t result = (((~addr >> 8) & 0x001) | ((~addr >> 5) & 0x006) | ((~addr >> 6) & 0x008) | ((addr << 4) & 0x010) | ((addr >> 6) & 0x020) | ((addr << 3) & 0x040) | ((addr << 2) & 0x080) | ((~addr >> 2) & 0x100) | ((addr << 8) & 0x200) | ((addr << 6) & 0x400) | ((~addr << 9) & 0x800) | (addr & 0x1F000));

    return result;
  }

  void oddXPaddrWrite(uint32_t addr, uint16_t data)
  {
    uint32_t oddAddr = (0x10400000 + ((unscramble((addr & 0xFFFFF) / 2) - 1) * 2));
    setAddress(0x10770000);
    readWord();
    readWord();
    setAddress(oddAddr);
    writeWord(data);
    writeWord(data);
    setAddress(0x10740000);
    readWord();
    readWord();
  }

  void evenXPaddrWrite(uint32_t addr, uint16_t data)
  {
    uint32_t evenAddr = (0x10400000 + (unscramble((addr & 0xFFFFF) / 2) * 2));
    setAddress(0x10760000);
    readWord();
    readWord();
    setAddress(evenAddr);
    writeWord(data);
    writeWord(data);
    setAddress(0x10740000);
    readWord();
    readWord();
  }

  void eraseXplorer()
  {
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    // Send chip erase to SST 29LE010
    sendFlashromXplorerCommand(0x8080);
    sendFlashromXplorerCommand(0x1010);

    delay(20);
  }

  void blankCheck_XP64()
  {
    // Blankcheck
    OSCR::UI::printLineSync(FS(OSCR::Strings::Status::Erasing));

    for (uint32_t currSector = 0; currSector < 262144; currSector += 131072)
    {
      for (uint32_t pos = 0; pos < 131072; pos += 512)
      {
        for (uint16_t currByte = 0; currByte < 512; currByte += 2)
        {
          // Read flash
          setAddress(0x10400000 + currSector + pos + currByte);
          // Compare both
          if (readWord() != 0xFFFF)
          {
            OSCR::UI::fatalError(F("Erase failed"));
          }
        }
      }
    }
  }

  void writeXplorer()
  {
    // Write Xplorer64 with 2x SST 29LE010
    // Each 29LE010 has 1024 pages, each 128 bytes in size

    //Initialize progress bar
    OSCR::UI::ProgressBar::init(fileSize);

    for (uint32_t currPage = 0; currPage < fileSize / 2; currPage += 128)
    {
      // Fill SD buffer with data in the order it will be expected by the CPLD
      for (uint32_t i = 0; i < 256; i += 2)
      {
        uint32_t unscrambled_address = (unscramble(((currPage * 2) + i) / 2) * 2);
        OSCR::Storage::Shared::sharedFile.seekSet(unscrambled_address);
        OSCR::Storage::Shared::sharedFile.read(&OSCR::Storage::Shared::buffer[i], 1);
        OSCR::Storage::Shared::sharedFile.seekSet(unscrambled_address + 1);
        OSCR::Storage::Shared::sharedFile.read(&OSCR::Storage::Shared::buffer[i + 1], 1);
      }

      //Send page write command to both flashroms
      sendFlashromXplorerCommand(0xA0A0);

      // Write 1 page each, one flashrom gets the low byte, the other the high byte.
      for (uint32_t currByte = 0; currByte < 256; currByte += 2)
      {
        // Join two bytes into one word
        uint16_t currWord = ((OSCR::Storage::Shared::buffer[currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currByte + 1] & 0xFF);

        // Set address
        if ((((currByte / 2) >> 4) & 0x1) == 0)
        {
          evenXPaddrWrite(0x10400000 + (currPage * 2) + currByte, currWord);
        }
        else
        {
          oddXPaddrWrite(0x10400000 + (currPage * 2) + currByte, currWord);
        }
      }

      OSCR::UI::ProgressBar::advance(256);
    }

    OSCR::UI::ProgressBar::finish();
  }

  uint32_t verifyXplorer()
  {
    OSCR::UI::ProgressBar::init(262144);

    // Open file on sd card
    OSCR::Storage::Shared::openRO();

    OSCR::Storage::Shared::rewind();
    writeErrors = 0;

    for (uint32_t currSector = 0; currSector < 262144; currSector += 131072)
    {
      for (uint32_t pos = 0; pos < 131072; pos += 512)
      {
        // Fill SD buffer
        OSCR::Storage::Shared::fill();

        for (uint16_t currByte = 0; currByte < 512; currByte += 2)
        {
          // Join two bytes into one word
          uint16_t currWord = ((OSCR::Storage::Shared::buffer[currByte] & 0xFF) << 8) | (OSCR::Storage::Shared::buffer[currByte + 1] & 0xFF);

          // Read flash
          setAddress(romBase + 0x400000 + currSector + pos + currByte);

          // Compare both
          if (readWord() != currWord)
          {
            writeErrors++;
          }
        }

        OSCR::UI::ProgressBar::advance(512);
      }
    }

    OSCR::UI::ProgressBar::finish();

    // Close the file:
    OSCR::Storage::Shared::close();

    return writeErrors;
  }
} /* namespace OSCR::Cores::N64 */

#endif /* ENABLE_N64 */
