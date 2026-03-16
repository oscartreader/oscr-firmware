/**
 * @file
 * @brief Define flags for various features/options.
 *
 * Define the preprocessor constants for various options.
 *
 * @note These are mainly for use in the config file.
 *
 * @warning As the updater uses these values, you should NOT change
 *          them, unless it is absolutely neccessary, such as when
 *          splitting a feature into two or more.
 *
 * @sa core-types.h
 */
#pragma once
#ifndef OSCR_FLAGS_H_
# define OSCR_FLAGS_H_


#pragma region Languages

/**
 * Language IDs
 *
 * When adding new languages, use the ISO-639 2-character code.
 *
 * @sa https://en.wikipedia.org/wiki/List_of_ISO_639_language_codes
 */
#define LANG_EN 1 // English
#define LANG_JA 2 // Japanese (Not implemented yet, just an example -- this ID can change if needed)


/**
 * Range of language IDs.
 *
 * Preprocess define for the IDs of supported languages.
 *
 * When adding/implementing a new language, update `LANG_MAX`.
 *
 * @note  These probably shouldn't be here, but I placed them here so
 *        that adding a language is simple as reasonably possible.
 */
#define LANG_MIN 1 // Lowest Language ID
#define LANG_MAX 1 // Highest Language ID


#pragma region Regions

/**
 * Region IDs
 *
 * The original order of these was decided based on simplifying the
 * preprocessor statements.
 */
#define REGN_AUTO 0 /* Auto/Global */
#define REGN_NA   1 /* North America */
#define REGN_EU   2 /* Europe */
#define REGN_AS   3 /* Asia */
#define REGN_AF   4 /* Africa */
#define REGN_OC   5 /* Oceania */
#define REGN_SA   6 /* South America */
#define REGN_AN   7 /* Antarctica (for completeness) */

/**
 * Range of region IDs.
 *
 * Preprocess define for the IDs of supported regions.
 */
#define REGN_MIN 0 // Lowest Region ID
#define REGN_MAX 7 // Highest Region ID


#pragma region Unique Directory Methods

/**
 * Unique Directory Methods
 */
#define UNQDIR_AUTO       0
#define UNQDIR_INCREMENT  1
#define UNQDIR_RTC        2
#define UNQDIR_BOTH       3


#pragma region Power Saving

/**
 * Power Saving
 */
#define POWERSAVING_DISPLAY_DIM 1
#define POWERSAVING_DISPLAY_OFF 2
#define POWERSAVING_SLOWCLOCK   4
#define POWERSAVING_CARTBUS_OFF 8


#pragma region Performance

/**
 * Performance Optimization Flags
 */
#define PRFOPT_CRC32        1
#define PRFOPT_FILEWR       2
#define PRFOPT_FILERD       4
#define PRFOPT_SHRDFILE     8
#define PRFOPT_FAST64       16
#define PRFOPT_SPEEDORDEATH 32

#pragma region Hardware


/**
 * Performance Optimization Flags
 */
#define RTCOPT_DS3231 1
#define RTCOPT_DS1307 2

#pragma region Output

/**
 * Output Interfaces
 */
#define OUTPUT_SERIAL   0
#define OUTPUT_SSD1306  1
#define OUTPUT_OS12864  2


/**
 * Serial Output Options
 */
#define SERIAL_ASCII    1
#define SERIAL_ANSI     2


/**
 * LCD Options
 */
#define LCD_MKS         1
#define LCD_BTT         2
#define LCD_SSRETRO     2

/**
 * NeoPixel Options
 */
#define NPXL_AUTO       0
#define NPXL_NORMAL     1
#define NPXL_REVERSE    2

/**
 * Voltage Specifier Options
 */
#define VLTSPC_NONE     0
#define VLTSPC_PROMPT   1
#define VLTSPC_TITLE    2
#define VLTSPC_BOTH     3

#pragma region Input

/**
 * Input Interfaces
 */
#define INPUT_SERIAL    0
#define INPUT_1BUTTON   1
#define INPUT_2BUTTON   2
#define INPUT_ROTARY    3

/**
 * [Core] %SMS - SMS Adapter Types
 */
#define SMSOPT_SMS_ADAPTER_ALL        0
#define SMSOPT_SMS_ADAPTER_HW5        1
#define SMSOPT_SMS_ADAPTER_RAPHNET    1
#define SMSOPT_SMS_ADAPTER_RETRODE    2
#define SMSOPT_SMS_ADAPTER_RETRON     3

/**
 * [Core] %SMS - GG Adapter Types
 */
#define SMSOPT_GG_ADAPTER_ALL       0
#define SMSOPT_GG_ADAPTER_RETRODE   1
#define SMSOPT_GG_ADAPTER_HW5       2
#define SMSOPT_GG_ADAPTER_RETRON    2
#define SMSOPT_GG_ADAPTER_STARSHADE 2

/**
 * [Core] %SMS - SG-1000 Adapter Types
 */
#define SMSOPT_GG_ADAPTER_ALL        0
#define SMSOPT_GG_ADAPTER_RAPHNET    1

#endif /* OSCR_FLAGS_H_ */
