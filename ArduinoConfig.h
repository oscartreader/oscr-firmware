/**
 * @file
 * @brief Configuration file for the Arduino IDE.
 *
 * The Arduino IDE is no longer being used for this project, although
 * support for it to compile and update is still being provided for
 * the time being.
 *
 * See `oscr-firmware.ino` for more information.
 *
 * @sa oscr-firmware.ino
 */
#pragma once
#ifndef CONFIG_H_
#define CONFIG_H_

/** FIRMWARE CONFIGURATION ******************************************
 *
 *  Add or remove the "//" in front of items to toggle them.
 *
 *  Disabled:
 *    //#define SOMETHING
 *
 *  Enabled:
 *    #define SOMETHING
 *
 *  Things in ** blocks like this are comments. Changing them doesn't
 *  affect the firmware that is flashed to your OSCR.
 *
 *  If you only get a blank screen or "Press Button" message after
 *  flashing you have enabled too many modules.
 *
 *******************************************************************/

/*==== HARDWARE VERSION ===========================================*/

/* [ Hardware Version --------------------------------------------- ]
 *  Choose your hardware version:
 */

#define HW_VERSION 5

/****/

/* [ Language ----------------------------------------------------- ]
 *  Select your preferred language here.
 *
 *  Available Options:
 *    LANG_EN : English
 */

#define OSCR_LANGUAGE LANG_EN

/****/

/* [ Language ----------------------------------------------------- ]
 *  Select your preferred region here. This only affects the names
 *   of things in UI elements. It does not affect the ability to dump
 *   cartridges in any way.
 *
 *  This currently only has an effect when `OSCR_LANGUAGE` is set to
 *   English, as most other languages will know the region based on
 *   that.
 *
 *  When no region is selected both/all names for an option typically
 *   will be shown.
 *
 *  For example:
 *   - Unset  : Genesis/Mega Drive
 *   - NA     : Genesis
 *   - EU|AS  : Mega Drive
 *
 *  Available Options:
 *   - REGN_AUTO  : Automatic (All for English, or via language)
 *   - REGN_NA    : North America
 *   - REGN_EU    : Europe
 *   - REGN_AS    : Asia (Japan)
 */
#define OSCR_REGION REGN_NA

/****/

/* [ Advanced: Display Type --------------------------------------- ]
 *  Choose a display type:
 *    OUTPUT_SERIAL   : Serial
 *    OUTPUT_SSD1306  : SSD1306, 128x64
 *    OUTPUT_OS12864  : ST7567/OS12864, 128x64 (MINI12864)
 */

//#define HARDWARE_OUTPUT_TYPE OUTPUT_SERIAL

/****/

/* [ Advanced: Input Type ----------------------------------------- ]
 *  Choose an input type:
 *    INPUT_SERIAL  : Serial
 *    INPUT_1BUTTON : 1 push button
 *    INPUT_2BUTTON : 2 push buttons ("HW3 style")
 *    INPUT_ROTARY  : Rotary dial + push button ("HW5 style")
 */

//#define HARDWARE_INPUT_TYPE INPUT_SERIAL

/****/

/* [ Advanced: Serial Output -------------------------------------- ]
 *  Choose the options your serial console supports:
 *    SERIAL_ASCII  : ASCII [default]
 *    SERIAL_ANSI   : ANSI
 *
 *  Hint: Use ASCII if using the Arduino IDE; use ANSI if using a real
 *  terminal program which supports ANSI control codes, such as PuTTY.
 */

//#define OPTION_SERIAL_OUTPUT SERIAL_ANSI

/****/

/*==== HARDWARE MODULES ===========================================*/

/* [ On-board ATmega2560 ------------------------------------------ ]
 *  Enable this if you have an integrated (non-module) ATmega.
 */

#define ENABLE_ONBOARD_ATMEGA

/****/

/* [ Automatic Voltage Selection ---------------------------------- ]
 *  Enable this if you have the VSELECT module.
 */

#define ENABLE_VSELECT

/****/

/* [ Clock Generator ---------------------------------------------- ]
 *  Disable this if you don't have the clock generator module.
 */

#define ENABLE_CLOCKGEN

/****/

/* [ Real Time Clock ---------------------------------------------- ]
 *  Enable this if you have the RTC module. You can configure the
 *  type later in this file.
 */

#define ENABLE_RTC

/****/

/*==== GAME SYSTEM CORES ==========================================*/

/* [ Atari 2600 --------------------------------------------------- ]
 */

//#define ENABLE_2600

/****/

/* [ Atari 5200 --------------------------------------------------- ]
 */

//#define ENABLE_5200

/****/

/* [ Atari 7800 --------------------------------------------------- ]
 */

//#define ENABLE_7800

/****/

/* [ Atari Jaguar ------------------------------------------------- ]
 */
//#define ENABLE_JAGUAR

/****/

/* [ Atari LYNX --------------------------------------------------- ]
 */

//#define ENABLE_LYNX

/****/

/* [ Atari 8-bit -------------------------------------------------- ]
 */

//#define ENABLE_ATARI8

/****/

/* [ Bally Astrocade ---------------------------------------------- ]
 */

//#define ENABLE_BALLY

/****/

/* [ Bandai Little Jammer ----------------------------------------- ]
 */

//#define ENABLE_LJ

/****/

/* [ Bandai Little Jammer Pro ------------------------------------- ]
 */

//#define ENABLE_LJPRO

/****/

/* [ Benesse Pocket Challenge W ----------------------------------- ]
 */

//#define ENABLE_PCW

/****/

/* [ Casio PV-1000 ------------------------------------------------ ]
 */

//#define ENABLE_PV1000

/****/

/* [ ColecoVision ------------------------------------------------- ]
 */

//#define ENABLE_COLV

/****/

/* [ Commodore 64 ------------------------------------------------- ]
 */

//#define ENABLE_C64

/****/

/* [ Commodore VIC-20 --------------------------------------------- ]
 */

//#define ENABLE_VIC20

/****/

/* [ Emerson Arcadia 2001 ----------------------------------------- ]
 */

//#define ENABLE_ARC

/****/

/* [ Fairchild Channel F ------------------------------------------ ]
 */

//#define ENABLE_FAIRCHILD

/****/

/* [ Flash -------------------------------------------------------- ]
 *  Enable flashing most supported repros.
 */

//#define ENABLE_FLASH

/****/

/* [ Flash ICs ---------------------------------------------------- ]
 *   Enable flashing 8-/16-bit ICs directly.
 */

//#define ENABLE_FLASH8
//#define ENABLE_FLASH16

/****/

/* [ Game Boy (Color) and Advance --------------------------------- ]
 */

#define ENABLE_GBX

/****/

/* [ Intellivision ------------------------------------------------ ]
 */

//#define ENABLE_INTV

/****/

/* [ LeapFrog Leapster -------------------------------------------- ]
 */

//#define ENABLE_LEAP

/****/

/* [ Neo Geo Pocket ----------------------------------------------- ]
 */

//#define ENABLE_NGP

/****/

/* [ Nintendo 64 -------------------------------------------------- ]
 */

#define ENABLE_N64
//#define ENABLE_CONTROLLERTEST

/****/

/* [ Nintendo Entertainment System/Family Computer ---------------- ]
 */

#define ENABLE_NES

/****/

/* [ Magnavox Odyssey 2 ------------------------------------------- ]
 */

//#define ENABLE_ODY2

/****/

/* [ MSX ---------------------------------------------------------- ]
 */

//#define ENABLE_MSX

/****/

/* [ PC Engine/TurboGrafx 16 -------------------------------------- ]
 */

//#define ENABLE_PCE

/****/

/* [ Pokemon Mini ------------------------------------------------- ]
 */

//#define ENABLE_POKE

/****/

/* [ RCA Studio II ------------------------------------------------ ]
 */

//#define ENABLE_RCA

/****/

/* [ Sega Master System/Mark III/Game Gear/SG-1000 ---------------- ]
 */

#define ENABLE_SMS

/****/

/* [ Sega Mega Drive/Genesis -------------------------------------- ]
 */

#define ENABLE_MD

/****/

/* [ Super Famicom SF Memory Cassette ----------------------------- ]
 */

//#define ENABLE_SFM

/****/

/* [ Super Famicom Satellaview ------------------------------------ ]
 */

//#define ENABLE_SV

/****/

/* [ Super Famicom Sufami Turbo ----------------------------------- ]
 */

//#define ENABLE_ST

/****/

/* [ Super Famicom Game Processor RAM Cassette -------------------- ]
 */

//#define ENABLE_GPC

/****/

/* [ Super Nintendo ----------------------------------------------- ]
 */

#define ENABLE_SNES

/****/

/* [ Texas Instruments TI-99 -------------------------------------- ]
 */

//#define ENABLE_TI99

/****/

/* [ Tomy Pyuuta -------------------------------------------------- ]
 */

//#define ENABLE_PYUUTA

/****/

/* [ TRS-80 Color Computer ---------------------------------------- ]
 */

//#define ENABLE_TRS80

/****/

/* [ Vectrex ------------------------------------------------------ ]
 */

//#define ENABLE_VECTREX

/****/

/* [ Virtual Boy -------------------------------------------------- ]
 */

//#define ENABLE_VBOY

/****/

/* [ Vtech V.Smile ------------------------------------------------ ]
 */

//#define ENABLE_VSMILE

/****/

/* [ Watara Supervision ------------------------------------------- ]
 */

//#define ENABLE_WSV

/****/

/* [ WonderSwan and Benesse Pocket Challenge v2 ------------------- ]
 */

//#define ENABLE_WS

/****/

/* [ Super A'can -------------------------------------------------- ]
 */

//#define ENABLE_SUPRACAN

/****/

/* [ Casio Loopy -------------------------------------------------- ]
 */

//#define ENABLE_LOOPY

/****/

/* [ CP System III ------------------------------------------------ ]
 */

//#define ENABLE_CPS3

/****/

/*==== FIRMWARE OPTIONS ===========================================*/

/* [ Voltage Specifier -------------------------------------------- ]
 *  Choose how voltage changes are displayed:
 *    0: None
 *    1: Prompt [default]
 *    2: Title (when 3V)
 *    3: Both
 *
 *  This controls how cores communicate the voltage they need. This
 *  setting is ignored if VSELECT is enabled.
 */

//#define OPTION_VOLTAGE_SPECIFIER VLTSPC_PROMPT

/****/

/* [ Config File -------------------------------------------------- ]
 *  Allow changing some configuration values via a config file. You
 *  generally can only use the config to set options or disable
 *  certain featuress. It cannot be used to toggle firmware options
 *  on, only off.
 *
 *  Note For Developers: See OSCR.* for info.
 *
 *  Filename: config.txt
 */

#define ENABLE_CONFIG

/****/

/* [ Performance Optimizations ------------------------------------ ]
 *  This option allows for configuring several often-called methods
 *   to always be inlined, resulting in better performance at the
 *   cost of program size.
 *
 *   - PRFOPT_CRC32
 *   - PRFOPT_FILEWR
 *   - PRFOPT_FILERD
 *   - PRFOPT_SHRDFILE
 *   - PRFOPT_FAST64
 *   - PRFOPT_SPEEDORDEATH
 */

//#define OPTION_PERFORMANCE_FLAGS (0)
//#define OPTION_PERFORMANCE_FLAGS (PRFOPT_CRC32)

/****/

/* [ Power Saving ------------------------------------------------- ]
 *  Enable power saving.
 */
#define ENABLE_POWERSAVING

/* [ Power Saving Method ------------------------------------------ ]
 *  Choose the power saving methods used via bitflag.
 *
 *  Options:
 *   - POWERSAVING_DISPLAY_DIM  : Dim the display (and LEDs).
 *   - POWERSAVING_DISPLAY_OFF  : Turn off the display (and LEDs).
 *   - POWERSAVING_SLOWCLOCK    : Reduce the MCU's clock speed.
 *
 *  All of these are pretty unobstrusive so by default they are all
 *    enabled here.
 */
#define OPTION_POWERSAVING_METHOD (POWERSAVING_DISPLAY_DIM | POWERSAVING_DISPLAY_OFF | POWERSAVING_SLOWCLOCK)

/* [ Power Saving Dim Timeout ------------------------------------- ]
 *  Idle time before dimming the display.
 */
#define OPTION_POWERSAVING_IDLE_DIM   30000

/* [ Power Saving Sleep Timeout ----------------------------------- ]
 *  Idle time before fully sleeping.
 */
#define OPTION_POWERSAVING_IDLE_SLEEP 60000

/****/

/* [ Directory Naming --------------------------------------------- ]
 *  How should directory names be kept unique?
 *
 *  Options:
 *   - UNQDIR_AUTO      : Autoselect [default]
 *   - UNQDIR_INCREMENT : Use incrementing value stored in EEPROM
 *   - UNQDIR_RTC       : Use a datetimestamp (requires RTC)
 *   - UNQDIR_BOTH      : Use both (requires RTC)
 *
 *  Autoselect: If RTC is enabled, it will be `UNQDIR_RTC`, otherwise
 *    it will be `UNQDIR_INCREMENT`.
 */

#define OPTION_UNIQUE_DIRECTORY_METHOD UNQDIR_AUTO

/****/

/* [ Pin Control -------------------------------------------------- ]
 *  Enable the new experimental PinControl method of reading/writing
 *  to carts.
 *
 *  This only affects systems which have been ported to use it.
 *
 *  Note: If successful, then eventually this won't be optional. So
 *    if this config option disappears just know that's why. Also,
 *    some systems may now or eventually use PinControl even if this
 *    option is not specifically enabled.
 */

//#define ENABLE_PINCONTROL

/****/

/* [ Debugging ---------------------------------------------------- ]
 *  Enable via serial. Cannot be used with ANSI serial.
 */

//#define ENABLE_DEBUG

/****/

/* [ CRDB Debugging ----------------------------------------------- ]
 *  Enable debugging the CRDB format. This will output additional
 *  information via %Serial.
 */

//#define ENABLE_CRDB_DEBUG

/****/

/* [ LCD: Background Color ---------------------------------------- ]
 *  Set the backlight color of the LCD if you have one.
 *
 *  Can be set using config:
 *    lcd.confColor=1
 *    lcd.red=0
 *    lcd.green=0
 *    lcd.blue=0
 */

#define OPTION_LCD_BG_RED 100
#define OPTION_LCD_BG_GREEN 0
#define OPTION_LCD_BG_BLUE 100

/****/

/* [ LCD: Notification Color -------------------------------------- ]
 *  Set the color used for notifications
 *
 *  Can be set using config:
 *    lcd.confNotifColor=1
 *    lcd.notifRed=0
 *    lcd.notifGreen=0
 *    lcd.notifBlue=0
 */

#define OPTION_LCD_NOTIF_RED 100
#define OPTION_LCD_NOTIF_GREEN 100
#define OPTION_LCD_NOTIF_BLUE 0

/****/

/* [ LCD: Error Color --------------------------------------------- ]
 *  Set the color used for notifications
 *
 *  Can be set using config:
 *    lcd.confErrorColor=1
 *    lcd.errorRed=0
 *    lcd.errorGreen=0
 *    lcd.errorBlue=0
 */

#define OPTION_LCD_ERROR_RED 100
#define OPTION_LCD_ERROR_GREEN 0
#define OPTION_LCD_ERROR_BLUE 0

/****/

/* [ LCD: RGB Behavior -------------------------------------------- ]
 *  Decide the behavior of the color of the LCD's rotary knob.
 *
 *  Options:
 *    1: Use notif/error colors, otherwise BG color. [default]
 *    2: Always BG color
 */

#define OPTION_LCD_RGB 1

/****/

/* [ LCD: Model --------------------------------------------------- ]
 *  Choose the type of LCD you have.
 *
 *  Options:
 *    LCD_MKS:      MKS MINI 12864 [default]
 *    LCD_BTT:      BTT MINI 12864
 *    LCD_SSRETRO:  StarshadeRETRO OSCR 12864 (internally the same as LCD_BTT)
 */

#define OPTION_LCD_TYPE LCD_SSRETRO

/****/

/* [ LCD: NeoPixel Order ------------------------------------------ ]
 *  The order of the colors for the LCD module.
 *
 *  Options:
 *    NPXL_AUTO     : Autoselect based on LCD type
 *    NPXL_NORMAL   : Original order
 *    NPXL_REVERSE  : Reverse order
 */

//#define OPTION_NEOPIXEL_ORDER NPXL_AUTO

/****/

/* [ Voltage Monitoring ------------------------------------------- ]
 *  Which method should be used to monitor voltage?
 *
 *  Options:
 *    0: Don't monitor voltages [default]
 *    1: Use voltage monitor IC on VCC
 *    2: Use VSELECT STAT pin (requires VSELECT)
 *    3: Use both
 *
 *  Note: Requires on-board ATmega2560
 */

//#define OPTION_VOLTAGE_MONITOR_METHOD 2

/****/

/* [ 3.3V Stability Fix (3V3FIX) ---------------------------------- ]
 *  Enable this if you are having stability issues when using 3.3V,
 *  works best with VSELECT.
 *
 *  If not using VSELECT, always turn the cart reader on with the
 *  voltage switch set to 5V and switch to 5V before selecting a
 *  cartridge from the menu.
 */

//#define ENABLE_3V3FIX

/****/

/* [ Updater ------------------------------------------------------ ]
 *  Disable this if you don't plan to/want to use the firmware
 *  updater utility. This setting is ignored on hardware versions
 *  other than HW5 and HW3.
 */

#define ENABLE_UPDATER

/****/

/* [ Self Test ---------------------------------------------------- ]
 *  Tests for shorts and other issues in your OSCR build.
 */

//#define ENABLE_SELFTEST

/****/

/* [ Logging ------------------------------------------------------ ]
 *  Write all info to OSCR_LOG.txt in root dir
 *
 *  Can be toggled off using config:
 *    oscr.logging=0
 */

//#define ENABLE_GLOBAL_LOG

/****/

/* [ RTC: IC Type ------------------------------------------------- ]
 *  When the RTC module is installed, choose the type here. This
 *  setting is ignored if the RTC option is not enabled.
 *
 *  Options:
 *    RTCOPT_DS3231: Using the DS3231
 *    RTCOPT_DS1307: Using the DS1307
 */

#define RTC_TYPE RTCOPT_DS3231

/****/

/* [ SNES Core/CLOCKGEN: Read Clock Generator Calibration Data ---- ]
 *  Toggle clock calibration menu and/or whether or not to use the
 *  calibration data from snes_clk.txt.
 */

//#define OPTION_CLOCKGEN_CALIBRATION
//#define OPTION_CLOCKGEN_USE_CALIBRATION

/****/

/* [ MegaDrive/Genesis Core: Compatibility Settings --------------- ]
 *  Configure how the MD core saves are formatted.
 *
 *  Can be set using config (if enabled):
 *    md.saveType=0
 *
 *  If config is enabled, this option does nothing -- use the config.
 *
 *  Options:
 *    0: Output each byte once. Not supported by emulators. [default]
 *    1: Duplicate each byte. Usable by Kega Fusion.
 *    2: Same as 1 + pad with 0xFF so that the file size is 64KB.
 */

//#define OPTION_MD_DEFAULT_SAVE_TYPE 0

/****/

/* [ SMS/Game Gear Core: Adapter Types ---------------------------- ]
 *  Change which adapters are shown on the menu as well as adjust
 *  the names of the menu options to match. If not set, all adapters
 *  will be shown with their original names.
 *
 *  When choosing an option that displays only one adapter, the name
 *  of the system (SMS/GG/etc) will be used instead of the adapter's
 *  type/name.
 */

// == SMS Adapters
// - SMSOPT_SMS_ADAPTER_ALL     : Show all SMS adapters [default for HW <= 3]
// - SMSOPT_SMS_ADAPTER_RAPHNET : Reaphnet adapter [default for HW >= 4]
// - SMSOPT_SMS_ADAPTER_HW5     : OSCR six-slot top-PCB. Internally the same as RAPHNET.
// - SMSOPT_SMS_ADAPTER_RETRODE : Retrode adapter
// - SMSOPT_SMS_ADAPTER_RETRON  : Retron3in1 adapter
//#define OPTION_SMS_ADAPTER SMSOPT_SMS_ADAPTER_ALL

// == GG Adapters
// - SMSOPT_GG_ADAPTER_ALL        : Show all GG adapters [default for HW < 5]
// - SMSOPT_GG_ADAPTER_RETRODE    : Retrode adapter
// - SMSOPT_GG_ADAPTER_RETRON     : Retron3in1 adapter [default for HW 5]
// - SMSOPT_GG_ADAPTER_HW5        : OSCR seven-slot top-PCB. Internally the same as RETRON.
// - SMSOPT_GG_ADAPTER_STARSHADE  : StarshadeRETRO SMS-to-GG adapter. Internally the same as RETRON.
//#define OPTION_GG_ADAPTER SMSOPT_GG_ADAPTER_ALL

// == SG-1000 Adapters
// -  0     Show all SG-1000 adapters [default]
// -  1     Raphnet
//#define OPTION_SG1000_ADAPTER SMSOPT_SG1000_ADAPTER_ALL

/****/

/* [ UI: File Browser --------------------------------------------- ]
 *  Settings for the new file browser.
 */

// Max filename length to display when browsing files.
#define UI_FILE_BROWSER_FILENAME_MAX 20

// Maximum number of files/folders in a directory
#define UI_FILE_BROWSER_FILES_MAX 50

// Maximum length of a filename to consider when sorting
//
//   Note: Cannot be higher than UI_FILE_BROWSER_FILENAME_MAX
//
#define UI_FILE_BROWSER_FILE_SORT_LEN 10 // 0 to disable

// Maximum directory depth
#define PATH_MAX_DEPTH 5

/****/

/* [ Filebrowser: Sort direction ---------------------------------- ]
 *  Enable to sort files/folders from newest to oldest
 */

//#define OPTION_REVERSE_SORT

/****/

//
//
// !!!!!           END OF USER-CONFIGURABLE SETTINGS           !!!!!
//
//

#define UPD_BAUD 9600

#if (HW_VERSION == 5)

# if !defined(HARDWARE_OUTPUT_TYPE)
#   define HARDWARE_OUTPUT_TYPE OUTPUT_OS12864
# endif /* HARDWARE_OUTPUT_TYPE */

# if !defined(HARDWARE_INPUT_TYPE)
#   define HARDWARE_INPUT_TYPE INPUT_ROTARY
# endif /* HARDWARE_INPUT_TYPE */

# if !defined(OPTION_SMS_ADAPTER)
#   define OPTION_SMS_ADAPTER SMSOPT_SMS_ADAPTER_HW5
# endif

# if !defined(OPTION_GG_ADAPTER)
#   define OPTION_GG_ADAPTER SMSOPT_GG_ADAPTER_HW5
# endif

#elif (HW_VERSION == 4)

# if !defined(HARDWARE_OUTPUT_TYPE)
#   define HARDWARE_OUTPUT_TYPE OUTPUT_OS12864
# endif /* HARDWARE_OUTPUT_TYPE */

# if !defined(HARDWARE_INPUT_TYPE)
#   define HARDWARE_INPUT_TYPE INPUT_ROTARY
# endif /* HARDWARE_INPUT_TYPE */

#elif (HW_VERSION == 3)

# if !defined(HARDWARE_OUTPUT_TYPE)
#   define HARDWARE_OUTPUT_TYPE OUTPUT_SSD1306
# endif /* HARDWARE_OUTPUT_TYPE */

# if !defined(HARDWARE_INPUT_TYPE)
#   define HARDWARE_INPUT_TYPE INPUT_2BUTTON
# endif /* HARDWARE_INPUT_TYPE */

#elif (HW_VERSION == 2)

# if !defined(HARDWARE_OUTPUT_TYPE)
#   define HARDWARE_OUTPUT_TYPE OUTPUT_SSD1306
# endif /* HARDWARE_OUTPUT_TYPE */

# if !defined(HARDWARE_INPUT_TYPE)
#   define HARDWARE_INPUT_TYPE INPUT_2BUTTON
# endif /* HARDWARE_INPUT_TYPE */

#elif (HW_VERSION == 1)

# if !defined(HARDWARE_OUTPUT_TYPE)
#   define HARDWARE_OUTPUT_TYPE OUTPUT_SSD1306
# endif /* HARDWARE_OUTPUT_TYPE */

# if !defined(HARDWARE_INPUT_TYPE)
#   define HARDWARE_INPUT_TYPE INPUT_1BUTTON
# endif /* HARDWARE_INPUT_TYPE */

#endif /* HW_VERSION */

# if !defined(ENABLE_NEOPIXEL) && !defined(DISABLE_NEOPIXEL) && (HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864)
#   define ENABLE_NEOPIXEL
# endif /* ENABLE_NEOPIXEL && !DISABLE_NEOPIXEL && (=OUTPUT_OS12864) */

# if !defined(OPTION_SERIAL_OUTPUT)
#   define OPTION_SERIAL_OUTPUT SERIAL_ASCII
# endif

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

#endif /* CONFIG_H_ */
