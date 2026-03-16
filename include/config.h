/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#ifndef OSCR_CONFIG_H_
# define OSCR_CONFIG_H_

//
// !!!!!        THESE ARE NOT USER-CONFIGURABLE SETTINGS        !!!!!
//
// This file is intended for developers and advanced users only.
//

# include "flags.h"
# include "macros.h"

# if defined(USING_ARDUINO_IDE) || (defined(ARDUINO) && !defined(PLATFORMIO))
#   include "../ArduinoConfig.h"
# endif

#pragma region README

/*==== INTERNAL DEPENDENCIES ======================================*/
/**
 * These `NEEDS_*` flags are for internal features. They are listed
 * here for convenience only -- do NOT enable them here. Create a
 * preprocessor condition below in the Dependencies section that
 * enables them when the appropriate conditions are met.
 */

/****/

/* [ Util: Bitset ------------------------------------------------- ]
 *  These allow you to toggle enabling a template to allow using a
 *  bitset for more than 8 bits.
 */

//#define NEEDS_UTIL_BITSET_TEMPLATE
//#define NEEDS_UTIL_BITSET_TEMPLATE_16
//#define NEEDS_UTIL_BITSET_TEMPLATE_32
//#define NEEDS_UTIL_BITSET_TEMPLATE_64

/****/

/* [ CRDB: Event Hooks -------------------------------------------- ]
 *  These allow you to toggle event hooking for CRDB which allow for
 *  adding core-specific function calls to various events.
 */

//#define NEEDS_CRDB_EVENTS
//#define NEEDS_CRDB_EVENT_POSTMATCHCRC
//#define NEEDS_CRDB_EVENT_POSTRENAMEFILE
//#define NEEDS_CRDB_EVENT_CRCMATCHFAIL
//#define NEEDS_CRDB_EVENT_CRCMATCHSUCCESS

/****/

/* [ CRDB: Quiet Flag --------------------------------------------- ]
 *  This flag enables methods to disable CRDB's standard UI output.
 *
 *  Enables:
 *   * `CRDB::isQuiet()`
 *   * `CRDB::quiet()`
 */
//#define NEEDS_CRDB_ISQUIET

/****/

#pragma Doxygen

#if !defined(DOXYGEN)
# define IS_DOXYGEN false
#endif

#pragma region Cores

/*==== PREPROCESSOR ===============================================*/

#define MAJOR_VERSION 20
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define CORE_ENABLE_auto  2
#define CORE_ENABLE_true  1
#define CORE_ENABLE_false 0

#define CORE_ENABLE_AUTO  2
#define CORE_ENABLE_TRUE  1
#define CORE_ENABLE_FALSE 0

#if (defined(WANT_POWERSAVING) && (WANT_POWERSAVING == true))
#   define ENABLE_POWERSAVING
#endif

#if (defined(WANT_UPDATER) && (WANT_UPDATER == true))
#   define ENABLE_UPDATER
#endif

#if (defined(WANT_NEOPIXEL) && (WANT_NEOPIXEL == true))
#   define ENABLE_NEOPIXEL
#endif

#if (CORE_ARC == CORE_ENABLE_TRUE) || ((CORE_ARC == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_ARC
#endif

#if (CORE_ATARI == CORE_ENABLE_TRUE) || ((CORE_ATARI == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_ATARI
#endif

#if (CORE_COLV == CORE_ENABLE_TRUE) || ((CORE_COLV == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_COLV
#endif

#if (CORE_FAIRCHILD == CORE_ENABLE_TRUE) || ((CORE_FAIRCHILD == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_FAIRCHILD
#endif

#if (CORE_FLASH == CORE_ENABLE_TRUE) || ((CORE_FLASH == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_FLASH
#endif

#if (CORE_FLASH16 == CORE_ENABLE_TRUE) || ((CORE_FLASH16 == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_FLASH16
#endif

#if (CORE_GBX == CORE_ENABLE_TRUE) || (CORE_GBX == CORE_ENABLE_AUTO)
# define ENABLE_GBX
#endif

#if (CORE_INTV == CORE_ENABLE_TRUE) || ((CORE_INTV == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_INTV
#endif

#if (CORE_LOOPY == CORE_ENABLE_TRUE) || ((CORE_LOOPY == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_LOOPY
#endif

#if (CORE_MD == CORE_ENABLE_TRUE) || (CORE_MD == CORE_ENABLE_AUTO)
# define ENABLE_MD
#endif

#if (CORE_MSX == CORE_ENABLE_TRUE) || ((CORE_MSX == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_MSX
#endif

#if (CORE_N64 == CORE_ENABLE_TRUE) || (CORE_N64 == CORE_ENABLE_AUTO)
# define ENABLE_N64
#endif

#if (CORE_NES == CORE_ENABLE_TRUE) || (CORE_NES == CORE_ENABLE_AUTO)
# define ENABLE_NES
#endif

#if (CORE_NGP == CORE_ENABLE_TRUE) || ((CORE_NGP == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_NGP
#endif

#if (CORE_ODY2 == CORE_ENABLE_TRUE) || ((CORE_ODY2 == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_ODY2
#endif

#if (CORE_PCE == CORE_ENABLE_TRUE) || ((CORE_PCE == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_PCE
#endif

#if (CORE_PCW == CORE_ENABLE_TRUE) || ((CORE_PCW == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_PCW
#endif

#if (CORE_POKE == CORE_ENABLE_TRUE) || ((CORE_POKE == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_POKE
#endif

#if (CORE_SFM == CORE_ENABLE_TRUE) || ((CORE_SFM == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_SFM
#endif

#if (CORE_SMS == CORE_ENABLE_TRUE) || (CORE_SMS == CORE_ENABLE_AUTO)
# define ENABLE_SMS
#endif

#if (CORE_SNES == CORE_ENABLE_TRUE) || (CORE_SNES == CORE_ENABLE_AUTO)
# define ENABLE_SNES
#endif

#if (CORE_SUPERACAN == CORE_ENABLE_TRUE) || ((CORE_SUPERACAN == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_SUPERACAN
#endif

#if (CORE_SV == CORE_ENABLE_TRUE) || ((CORE_SV == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_SV
#endif

#if (CORE_VBOY == CORE_ENABLE_TRUE) || ((CORE_VBOY == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_VBOY
#endif

#if (CORE_WATARASV == CORE_ENABLE_TRUE) || ((CORE_WATARASV == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_WATARASV
#endif

#if (CORE_WONDERSWAN == CORE_ENABLE_TRUE) || ((CORE_WONDERSWAN == CORE_ENABLE_AUTO) && (HW_VERSION > 6))
# define ENABLE_WONDERSWAN
#endif

# if (WANT_VSELECT == true)
#   define ENABLE_VSELECT
#endif

# if (WANT_3V3FIX == true)
#   define ENABLE_3V3FIX
# endif

# if (WANT_RTC == true)
#   define ENABLE_RTC
# endif

# if (WANT_RTC == true)
#   define ENABLE_CONFIG
# endif

# if (WANT_CLOCKGEN == true)
#   define ENABLE_CLOCKGEN
# endif

# if (USING_ONBOARD_ATMEGA == true)
#   define ENABLE_ONBOARD_ATMEGA
# endif

#define VERSION_STRING NUM2STR(MAJOR_VERSION) "." NUM2STR(MINOR_VERSION) "." NUM2STR(PATCH_VERSION)

#pragma region Dependencies

/*==== CORE DEPENDENCIES ==========================================*/

// !! NOTE
//
// Core flags have been extended to help make things easier for
// people to understand. When an `ENABLE` flag is set, any core that
// requires something from another core should set a `NEEDS` flag
// for that core. This will enable the functions but omit it from
// the main menu unless the user specifically enabled it.
//
// This also reduces the amount of program memory used by cores the
// user does not care about even when needed by some other core they
// do care about.
//
// FLAG NAMING CONVENTION
// * `ENABLE_` is used when the user wants the core enabled. It will
//    appear on menus, etc.
// * `NEEDS_` is used when a core needs another core to be enabled,
//    but the user didn't neccessarily explicitly ask for it to be
//    enabled. If only `NEEDS` is defined, the core will not appear
//    on menus.
// * `HAS_` should be set and and can be used when there isn't a
//    difference between the `ENABLE` and `NEEDS` flags.
//
// TLDR:
//  When a core can be required by other cores, make sure `HAS_` is
//  defined below when either `ENABLE_` and/or `NEEDS_` is defined
//  and then within the core's source, check `ENABLE_*` for menus,
//  etc., and check `HAS_` for everything else.
//

/****/

/**
 * CPS3 needs FLASH8/FLASH16
 */
# if defined(ENABLE_CPS3)
#   if !defined(NEEDS_FLASH8)
#     define NEEDS_FLASH8
#   endif
#   if !defined(NEEDS_FLASH16)
#     define NEEDS_FLASH16
#   endif
# endif

/****/

/**
 * SFM, SV, ST, and GPC need SNES
 */
# if defined(ENABLE_SFM) || defined(ENABLE_SV) || defined(ENABLE_ST) || defined(ENABLE_GPC)
#   if !defined(NEEDS_FLASH8)
#     define NEEDS_SNES
#   endif
# endif

/****/

/**
 * SNES + FLASH also needs FLASH8
 */
# if defined(ENABLE_SNES) && defined(ENABLE_FLASH)
#   if !defined(NEEDS_FLASH8)
#     define NEEDS_FLASH8
#   endif
# endif

/****/

/**
 * (ENABLED_/NEEDS_)FLASH16 implies (ENABLED_/NEEDS_)FLASH8
 */

#if defined(ENABLE_FLASH16) && !defined(ENABLED_FLASH8)
# define ENABLED_FLASH8
#endif

#if defined(NEEDS_FLASH16) && !defined(NEEDS_FLASH8)
# define NEEDS_FLASH8
#endif

/****/

/*==== FINAL PROCESSING & PRESETS =================================*/

#pragma region HAS Flags

/* [ Cores: HAS Flags --------------------------------------------- ]
 *  `HAS_*` is a intended to be a quick way to check if one or both
 *  of `ENABLE_*` and `NEEDS_*` is set.
 */

# if defined(ENABLE_GBX) || defined(NEEDS_GBX)
#   define HAS_GBX 1
# else
#   define HAS_GBX 0
# endif /* ENABLE_GBX || NEEDS_GBX */

# if defined(ENABLE_N64) || defined(NEEDS_N64)
#   define HAS_N64 1
# else
#   define HAS_N64 0
# endif /* ENABLE_N64 || NEEDS_N64 */

# if defined(ENABLE_SNES) || defined(NEEDS_SNES)
#   define HAS_SNES 1
# else
#   define HAS_SNES 0
# endif /* ENABLE_SNES || NEEDS_SNES */

# if defined(ENABLE_SFM) || defined(NEEDS_SFM)
#   define HAS_SFM 1
# else
#   define HAS_SFM 0
# endif /* ENABLE_SFM || NEEDS_SFM */

# if defined(ENABLE_FLASH) || defined(NEEDS_FLASH)
#   define HAS_FLASH 1
# else
#   define HAS_FLASH 0
# endif /* ENABLE_FLASH || NEEDS_FLASH */

# if defined(ENABLE_FLASH8) || defined(NEEDS_FLASH8)
#   define HAS_FLASH8 1
# else
#   define HAS_FLASH8 0
# endif /* ENABLE_FLASH8 || NEEDS_FLASH8 */

# if defined(ENABLE_FLASH16) || defined(NEEDS_FLASH16)
#   define HAS_FLASH16 1
# else
#   define HAS_FLASH16 0
# endif /* ENABLE_FLASH16 || NEEDS_FLASH16 */

# if defined(ENABLE_MD) || defined(NEEDS_MD)
#   define HAS_MD 1
# else
#   define HAS_MD 0
# endif /* ENABLE_MD || NEEDS_MD */

# if defined(ENABLE_PCE) || defined(NEEDS_PCE)
#   define HAS_PCE 1
# else
#   define HAS_PCE 0
# endif /* ENABLE_PCE || NEEDS_PCE */

# if defined(ENABLE_SV) || defined(NEEDS_SV)
#   define HAS_SV 1
# else
#   define HAS_SV 0
# endif /* ENABLE_SV || NEEDS_SV */

# if defined(ENABLE_NES) || defined(NEEDS_NES)
#   define HAS_NES 1
# else
#   define HAS_NES 0
# endif /* ENABLE_NES || NEEDS_NES */

# if defined(ENABLE_SMS) || defined(NEEDS_SMS)
#   define HAS_SMS 1
# else
#   define HAS_SMS 0
# endif /* ENABLE_SMS || NEEDS_SMS */

# if defined(ENABLE_WS) || defined(NEEDS_WS)
#   define HAS_WS 1
# else
#   define HAS_WS 0
# endif /* ENABLE_WS || NEEDS_WS */

# if defined(ENABLE_NGP) || defined(NEEDS_NGP)
#   define HAS_NGP 1
# else
#   define HAS_NGP 0
# endif /* ENABLE_NGP || NEEDS_NGP */

# if defined(ENABLE_INTV) || defined(NEEDS_INTV)
#   define HAS_INTV 1
# else
#   define HAS_INTV 0
# endif /* ENABLE_INTV || NEEDS_INTV */

# if defined(ENABLE_COLV) || defined(NEEDS_COLV)
#   define HAS_COLV 1
# else
#   define HAS_COLV 0
# endif /* ENABLE_COLV || NEEDS_COLV */

# if defined(ENABLE_VBOY) || defined(NEEDS_VBOY)
#   define HAS_VBOY 1
# else
#   define HAS_VBOY 0
# endif /* ENABLE_VBOY || NEEDS_VBOY */

# if defined(ENABLE_WSV) || defined(NEEDS_WSV)
#   define HAS_WSV 1
# else
#   define HAS_WSV 0
# endif /* ENABLE_WSV || NEEDS_WSV */

# if defined(ENABLE_PCW) || defined(NEEDS_PCW)
#   define HAS_PCW 1
# else
#   define HAS_PCW 0
# endif /* ENABLE_PCW || NEEDS_PCW */

# if defined(ENABLE_ODY2) || defined(NEEDS_ODY2)
#   define HAS_ODY2 1
# else
#   define HAS_ODY2 0
# endif /* ENABLE_ODY2 || NEEDS_ODY2 */

# if defined(ENABLE_ARC) || defined(NEEDS_ARC)
#   define HAS_ARC 1
# else
#   define HAS_ARC 0
# endif /* ENABLE_ARC || NEEDS_ARC */

# if defined(ENABLE_FAIRCHILD) || defined(NEEDS_FAIRCHILD)
#   define HAS_FAIRCHILD 1
# else
#   define HAS_FAIRCHILD 0
# endif /* ENABLE_FAIRCHILD || NEEDS_FAIRCHILD */

# if defined(ENABLE_SUPRACAN) || defined(NEEDS_SUPRACAN)
#   define HAS_SUPRACAN 1
# else
#   define HAS_SUPRACAN 0
# endif /* ENABLE_SUPRACAN || NEEDS_SUPRACAN */

# if defined(ENABLE_MSX) || defined(NEEDS_MSX)
#   define HAS_MSX 1
# else
#   define HAS_MSX 0
# endif /* ENABLE_MSX || NEEDS_MSX */

# if defined(ENABLE_POKE) || defined(NEEDS_POKE)
#   define HAS_POKE 1
# else
#   define HAS_POKE 0
# endif /* ENABLE_POKE || NEEDS_POKE */

# if defined(ENABLE_LOOPY) || defined(NEEDS_LOOPY)
#   define HAS_LOOPY 1
# else
#   define HAS_LOOPY 0
# endif /* ENABLE_LOOPY || NEEDS_LOOPY */

# if defined(ENABLE_C64) || defined(NEEDS_C64)
#   define HAS_C64 1
# else
#   define HAS_C64 0
# endif /* ENABLE_C64 || NEEDS_C64 */

# if defined(ENABLE_2600) || defined(NEEDS_2600)
#   define HAS_2600 1
# else
#   define HAS_2600 0
# endif /* ENABLE_2600 || NEEDS_2600 */

# if defined(ENABLE_5200) || defined(NEEDS_5200)
#   define HAS_5200 1
# else
#   define HAS_5200 0
# endif /* ENABLE_5200 || NEEDS_5200 */

# if defined(ENABLE_7800) || defined(NEEDS_7800)
#   define HAS_7800 1
# else
#   define HAS_7800 0
# endif /* ENABLE_7800 || NEEDS_7800 */

# if defined(ENABLE_JAGUAR) || defined(NEEDS_JAGUAR)
#   define HAS_JAGUAR 1
# else
#   define HAS_JAGUAR 0
# endif /* ENABLE_JAGUAR || NEEDS_JAGUAR */

# if defined(ENABLE_LYNX) || defined(NEEDS_LYNX)
#   define HAS_LYNX 1
# else
#   define HAS_LYNX 0
# endif /* ENABLE_LYNX || NEEDS_LYNX */

# if defined(ENABLE_VECTREX) || defined(NEEDS_VECTREX)
#   define HAS_VECTREX 1
# else
#   define HAS_VECTREX 0
# endif /* ENABLE_VECTREX || NEEDS_VECTREX */

# if defined(ENABLE_ST) || defined(NEEDS_ST)
#   define HAS_ST 1
# else
#   define HAS_ST 0
# endif /* ENABLE_ST || NEEDS_ST */

# if defined(ENABLE_GPC) || defined(NEEDS_GPC)
#   define HAS_GPC 1
# else
#   define HAS_GPC 0
# endif /* ENABLE_GPC || NEEDS_GPC */

# if defined(ENABLE_ATARI8) || defined(NEEDS_ATARI8)
#   define HAS_ATARI8 1
# else
#   define HAS_ATARI8 0
# endif /* ENABLE_ATARI8 || NEEDS_ATARI8 */

# if defined(ENABLE_BALLY) || defined(NEEDS_BALLY)
#   define HAS_BALLY 1
# else
#   define HAS_BALLY 0
# endif /* ENABLE_BALLY || NEEDS_BALLY */

# if defined(ENABLE_LJ) || defined(NEEDS_LJ)
#   define HAS_LJ 1
# else
#   define HAS_LJ 0
# endif /* ENABLE_LJ || NEEDS_LJ */

# if defined(ENABLE_LJPRO) || defined(NEEDS_LJPRO)
#   define HAS_LJPRO 1
# else
#   define HAS_LJPRO 0
# endif /* ENABLE_LJPRO || NEEDS_LJPRO */

# if defined(ENABLE_PV1000) || defined(NEEDS_PV1000)
#   define HAS_PV1000 1
# else
#   define HAS_PV1000 0
# endif /* ENABLE_PV1000 || NEEDS_PV1000 */

# if defined(ENABLE_VIC20) || defined(NEEDS_VIC20)
#   define HAS_VIC20 1
# else
#   define HAS_VIC20 0
# endif /* ENABLE_VIC20 || NEEDS_VIC20 */

# if defined(ENABLE_LEAP) || defined(NEEDS_LEAP)
#   define HAS_LEAP 1
# else
#   define HAS_LEAP 0
# endif /* ENABLE_LEAP || NEEDS_LEAP */

# if defined(ENABLE_RCA) || defined(NEEDS_RCA)
#   define HAS_RCA 1
# else
#   define HAS_RCA 0
# endif /* ENABLE_RCA || NEEDS_RCA */

# if defined(ENABLE_TI99) || defined(NEEDS_TI99)
#   define HAS_TI99 1
# else
#   define HAS_TI99 0
# endif /* ENABLE_TI99 || NEEDS_TI99 */

# if defined(ENABLE_PYUUTA) || defined(NEEDS_PYUUTA)
#   define HAS_PYUUTA 1
# else
#   define HAS_PYUUTA 0
# endif /* ENABLE_PYUUTA || NEEDS_PYUUTA */

# if defined(ENABLE_TRS80) || defined(NEEDS_TRS80)
#   define HAS_TRS80 1
# else
#   define HAS_TRS80 0
# endif /* ENABLE_TRS80 || NEEDS_TRS80 */

# if defined(ENABLE_VSMILE) || defined(NEEDS_VSMILE)
#   define HAS_VSMILE 1
# else
#   define HAS_VSMILE 0
# endif /* ENABLE_VSMILE || NEEDS_VSMILE */

# if defined(ENABLE_CPS3) || defined(NEEDS_CPS3)
#   define HAS_CPS3 1
# else
#   define HAS_CPS3 0
# endif /* ENABLE_CPS3 || NEEDS_CPS3 */

# if defined(ENABLE_SELFTEST) || defined(NEEDS_SELFTEST)
#   define HAS_SELFTEST 1
# else
#   define HAS_SELFTEST 0
# endif /* ENABLE_ATARI8 || NEEDS_ATARI8 */

# if defined(NEEDS_UTIL_BITSET_TEMPLATE)
#   define HAS_UTIL_BITSET_TEMPLATE 1
# endif

/* [ Feature Flags ------------------------------------------------ ]
 *  For features, `HAS_*` falgs are booleans and `HW_VERSION`
 *  contains the numerical value of the hardware version.
 */

# if defined(ENABLE_UPDATER)
#   define HAS_UPDATER 1
# else
#   define HAS_UPDATER 0
# endif

# if defined(ENABLE_CONFIG)
#   define HAS_CONFIG 1
# else
#   define HAS_CONFIG 0
# endif

# if defined(ENABLE_CLOCKGEN)
#   define HAS_CLOCKGEN 1
# else
#   define HAS_CLOCKGEN 0
# endif

# if defined(ENABLE_VSELECT)
#   define HAS_VSELECT 1
# else
#   define HAS_VSELECT 0
# endif

# if defined(ENABLE_RTC)
#   define HAS_RTC 1
# else
#   define HAS_RTC 0
# endif

# if defined(ENABLE_ONBOARD_ATMEGA)
#   define HAS_ONBOARD_ATMEGA 1
# else
#   define HAS_ONBOARD_ATMEGA 0
# endif

# if defined(ENABLE_3V3FIX)
#   define HAS_STABILITYFIX 1
# else
#   define HAS_STABILITYFIX 0
# endif

# if defined(ENABLE_POWERSAVING)
#   define HAS_POWERSAVING 1
# else
#   define HAS_POWERSAVING 0
# endif

# if defined(OPTION_CLOCKGEN_CALIBRATION)
#   define HAS_CLOCKGEN_CALIBRATION 1
# else
#   define HAS_CLOCKGEN_CALIBRATION 0
# endif

# if defined(OPTION_CLOCKGEN_USE_CALIBRATION)
#   define HAS_CLOCKGEN_CALIBRATED 1
# else
#   define HAS_CLOCKGEN_CALIBRATED 0
# endif

/* [ Util: Flags -------------------------------------------------- ]
 */

#pragma region Presets

# if !defined(OPTION_SERIAL_OUTPUT)
#   define OPTION_SERIAL_OUTPUT SERIAL_ASCII
# endif

# if !defined(OPTION_LCD_RGB)
#   define OPTION_LCD_RGB 1
# endif

#define OPTION_LCD_BG_RED 100
#define OPTION_LCD_BG_GREEN 0
#define OPTION_LCD_BG_BLUE 100

#define OPTION_LCD_NOTIF_RED 100
#define OPTION_LCD_NOTIF_GREEN 100
#define OPTION_LCD_NOTIF_BLUE 0

#define OPTION_LCD_ERROR_RED 100
#define OPTION_LCD_ERROR_GREEN 0
#define OPTION_LCD_ERROR_BLUE 0

#pragma region Core Defaults

/**
 * [SMS] Default Adapters
 */

// If adding a new adapter, increase the max here
# define OPTION_SMS_ADAPTER_MAX 3
# define OPTION_GG_ADAPTER_MAX 2
# define OPTION_SG1000_ADAPTER_MAX 1

/****/

#pragma region Defaults

# if defined(ENABLE_CONFIG)
#   define CONFIG_FILE "config.txt"
// Max length of the key=value pairs
// Do your best not to have to increase these.
#   define CONFIG_KEY_MAX 32
#   define CONFIG_VALUE_MAX 32
# endif

/****/

# if !defined(OSCR_REGION)
#   define OSCR_REGION REGN_AUTO
# endif

/****/

// Unique Directory Method

# if !defined(OPTION_UNIQUE_DIRECTORY_METHOD)
#   define OPTION_UNIQUE_DIRECTORY_METHOD UNQDIR_AUTO
# elif ISEMPTY(OPTION_UNIQUE_DIRECTORY_METHOD)
#   undef OPTION_UNIQUE_DIRECTORY_METHOD
#   define OPTION_UNIQUE_DIRECTORY_METHOD UNQDIR_AUTO
# endif

# if (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_AUTO)
#   undef OPTION_UNIQUE_DIRECTORY_METHOD
#   if defined(ENABLE_RTC)
#     define OPTION_UNIQUE_DIRECTORY_METHOD UNQDIR_RTC
#   else
#     define OPTION_UNIQUE_DIRECTORY_METHOD UNQDIR_INCREMENT
#   endif
# endif

/****/

// Power Saving

# if defined(ENABLE_POWERSAVING)

#   if WANT_POWERSAVING_DISPLAY_DIM == true
#     define HAS_POWERSAVING_DISPLAY_DIM POWERSAVING_DISPLAY_DIM
#   else
#     define HAS_POWERSAVING_DISPLAY_DIM 0
#   endif

#   if WANT_POWERSAVING_DISPLAY_OFF == true
#     define HAS_POWERSAVING_DISPLAY_OFF POWERSAVING_DISPLAY_OFF
#   else
#     define HAS_POWERSAVING_DISPLAY_OFF 0
#   endif

#   if WANT_POWERSAVING_SLOWCLOCK == true
#     define HAS_POWERSAVING_SLOWCLOCK POWERSAVING_SLOWCLOCK
#   else
#     define HAS_POWERSAVING_SLOWCLOCK 0
#   endif

#   if ISEMPTY(ENABLE_POWERSAVING)
#     undef  ENABLE_POWERSAVING
#     define ENABLE_POWERSAVING 1
#   endif

#   if !defined(OPTION_POWERSAVING_METHOD)
#     define OPTION_POWERSAVING_METHOD (HAS_POWERSAVING_DISPLAY_DIM | HAS_POWERSAVING_DISPLAY_OFF | HAS_POWERSAVING_SLOWCLOCK)
#   endif

#   if !defined(OPTION_POWERSAVING_IDLE_DIM) || ISEMPTY(OPTION_POWERSAVING_IDLE_DIM)
#     define OPTION_POWERSAVING_IDLE_DIM   30000
#   elif (ISEMPTY(OPTION_POWERSAVING_IDLE_DIM) || OPTION_POWERSAVING_IDLE_DIM < 1000)
#     undef  OPTION_POWERSAVING_IDLE_DIM
#     define OPTION_POWERSAVING_IDLE_DIM   30000
#   endif

#   if !defined(OPTION_POWERSAVING_IDLE_SLEEP)
#     define OPTION_POWERSAVING_IDLE_SLEEP 60000
#   elif (ISEMPTY(OPTION_POWERSAVING_IDLE_SLEEP) || OPTION_POWERSAVING_IDLE_SLEEP < 1000)
#     undef  OPTION_POWERSAVING_IDLE_SLEEP
#     define OPTION_POWERSAVING_IDLE_SLEEP 60000
#   endif

# elif defined(OPTION_POWERSAVING_METHOD)
#   undef  OPTION_POWERSAVING_METHOD
#   define OPTION_POWERSAVING_METHOD (0)
# else
#   define OPTION_POWERSAVING_METHOD (0)
# endif

# if !defined(OPTION_VOLTAGE_SPECIFIER)
#   define OPTION_VOLTAGE_SPECIFIER VLTSPC_PROMPT
# endif /* !OPTION_VOLTAGE_SPECIFIER */

/****/

/**
 * [LCD] Defaults
 */
# if HARDWARE_OUTPUT_TYPE == OUTPUT_OS12864
/**
 * [UI Page Entry] Maximum Lines
 * The maximum number of lines on a page, usually the line height of the display.
 */
#   define UI_PAGE_SIZE ((uint8_t)7)

/**
 * [UI Page Entry] Maximum Length
 * The maximum length of a page, usually the line length of the display plus 1 for the string terminator.
 */
#   define UI_PAGE_ENTRY_LENTH_MAX ((uint8_t)30)

/**
 * [UI Page Entry] Minimum Length
 * The minimum length of a page. Don't change this.
 */
#   define UI_PAGE_ENTRY_LENTH_MIN ((uint8_t)2)

#   if defined(OPTION_LCD_TYPE) && OPTION_LCD_TYPE == 0
#     undef OPTION_LCD_TYPE
#   endif

#   if !defined(OPTION_LCD_TYPE) || OPTION_LCD_TYPE == 0
#     define OPTION_LCD_TYPE LCD_MKS
#   endif

#   if defined(OPTION_NEOPIXEL_ORDER) && OPTION_NEOPIXEL_ORDER == NPXL_AUTO
#     undef OPTION_NEOPIXEL_ORDER
#   endif

#   if OPTION_LCD_TYPE == 1 /* Makerbase MINI 12864*/
#     if !defined(OPTION_NEOPIXEL_ORDER)
#       define OPTION_NEOPIXEL_ORDER NPXL_NORMAL
#     endif
#   elif OPTION_LCD_TYPE == 2 /* StarshadeRETRO OSCR 12864 or BigTreeTech MINI 12864*/
#     if !defined(OPTION_NEOPIXEL_ORDER)
#       define OPTION_NEOPIXEL_ORDER NPXL_REVERSE
#     endif
#     if !defined(NO_OPTION_BTN_PULLUP) && !defined(OPTION_BTN_PULLUP_DISABLE)
#       define OPTION_BTN_PULLUP
#     endif
#   else  /* OPTION_LCD_TYPE = ? */
#     error !!! INVALID VALUE FOR OPTION_LCD_TYPE !!!
#   endif /* OPTION_LCD_TYPE */

# elif HARDWARE_OUTPUT_TYPE == OUTPUT_SSD1306
/**
 * [OLED] Defaults
 */

/**
 * [UI Page Entry] Maximum Lines
 * The maximum number of lines on a page, usually the line height of the display.
 */
#   define UI_PAGE_SIZE ((uint8_t)7)

/**
 * [UI Page Entry] Maximum Length
 * The maximum length of a page, usually the line length of the display plus 1 for the string terminator.
 */
#   define UI_PAGE_ENTRY_LENTH_MAX ((uint8_t)20)

/**
 * [UI Page Entry] Minimum Length
 * The minimum length of a page. Don't change this.
 */
#   define UI_PAGE_ENTRY_LENTH_MIN ((uint8_t)2)

# elif HARDWARE_OUTPUT_TYPE == OUTPUT_SERIAL
/**
 * [Serial] Defaults
 */

/**
 * [UI Page Entry] Maximum Lines
 * The maximum number of lines on a page, usually the line height of the display.
 */
#   define UI_PAGE_SIZE ((uint8_t)32)

/**
 * [UI Page Entry] Maximum Length
 * The maximum length of a page, usually the line length of the display plus 1 for the string terminator.
 */
#   define UI_PAGE_ENTRY_LENTH_MAX ((uint8_t)31)

/**
 * [UI Page Entry] Minimum Length
 * The minimum length of a page. Don't change this.
 */
#   define UI_PAGE_ENTRY_LENTH_MIN ((uint8_t)2)

/**
 * [UI Input] Next
 * A single character for choosing "next" (`UserInput::kUserInputNext`)
 */
#   define UI_INPUT_SERIAL_NEXT           'n'

/**
 * [UI Input] Back
 * A single character for choosing "back" (`UserInput::kUserInputBack`)
 */
#   define UI_INPUT_SERIAL_BACK           'b'

/**
 * [UI Input] Confirm
 * A single character for choosing "confirm" (`UserInput::kUserInputConfirm`)
 */
#   define UI_INPUT_SERIAL_CONFIRM        'c'

/**
 * [UI Input] Confirm (Short)
 * A single character for choosing "confirm short" (`UserInput::kUserInputConfirmShort`)
 */
#   define UI_INPUT_SERIAL_CONFIRM_SHORT  'x'

/**
 * [UI Input] Confirm Long
 * A single character for choosing "confirm long" (`UserInput::kUserInputConfirmLong`)
 */
#   define UI_INPUT_SERIAL_CONFIRM_LONG   'z'

/**
 * [UI Input] ANSI Next
 * The escape sequence for choosing "next" (`UserInput::kUserInputNext`)
 */
#   define UI_INPUT_SERIAL_ANSI_NEXT      0x42

/**
 * [UI Input] ANSI Back
 * The escape sequence for choosing "back" (`UserInput::kUserInputBack`)
 */
#   define UI_INPUT_SERIAL_ANSI_BACK      0x41

/**
 * [UI Input] ANSI Confirm
 * The escape sequence for choosing "confirm" (`UserInput::kUserInputConfirm`)
 */
#   define UI_INPUT_SERIAL_ANSI_CONFIRM   0x43

#   if (OPTION_SERIAL_OUTPUT == 1)
#     define ENABLE_SERIAL_OUTPUT
#     define ENABLE_SERIAL_ASCII
#   elif (OPTION_SERIAL_OUTPUT == 2)
#     define ENABLE_SERIAL_OUTPUT
#     define ENABLE_SERIAL_ANSI
#   endif

# endif /* ENABLE_LCD || ENABLE_OLED*/

/**
 * [Bankset] Minimum supported banks
 * The minimum that all targets must support. For the board-specific
 * number of targets available:
 *  - Preprocessor: `BANKSET_SUPPORTED_BANKS`
 *  - Code: `OSCR::Hardware::kBanksetSupportedBanks`
 */
# define BANKSET_MIN_SUPPORTED_BANKS 4

#pragma region Sanity Checks

/*==== SANITY CHECKS ==============================================*/

// Error if no hardware version is enabled
# if (HW_VERSION < 1) || (HW_VERSION > 5)
#   error !!! PLEASE CHOOSE HARDWARE VERSION IN CONFIG.H !!!
# endif

/****/

// Let user know unsafe configs are allowed
# if defined(ALLOW_UNSAFE_CONFIG)
// Error if defined during GitHub CI tests. This should not happen unless someone accidentally committed their Config.h
#   if defined(GITHUB_CI)
#     error !! UNSAFE CONFIGURATIONS ARE ALLOWED !! -- This should not be enabled on the repository!
#   else /* !defined(GITHUB_CI) */
#     warning !! UNSAFE CONFIGURATIONS ARE ALLOWED !! -- Unless you are a developer this probably is not something you want set.
#   endif /* GITHUB_CI */
# endif /* ALLOW_UNSAFE_CONFIG */

/****/

# if ((OPTION_UNIQUE_DIRECTORY_METHOD < UNQDIR_INCREMENT) || (OPTION_UNIQUE_DIRECTORY_METHOD > UNQDIR_BOTH))
#   error Unique directory method is invalid.
# elif ((OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_RTC) || (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_BOTH))
#   if (!defined(ENABLE_RTC))
#     error Unique directory method is set to use the datetime, but RTC is not enabled.
#   endif
# endif

/****/

# if !defined(OSCR_LANGUAGE)
#   error !!! PLEASE CHOOSE A LANGUAGE !!!
# endif

# if ((OSCR_LANGUAGE < LANG_MIN) || (OSCR_LANGUAGE > LANG_MAX))
#   error The language specified in Config.h is invalid.
# endif

/****/

# if defined(OSCR_REGION)
#   if ((OSCR_REGION < REGN_MIN) || (OSCR_REGION > REGN_MAX))
#     error The region specified in Config.h is invalid.
#   endif
# endif

/****/

# if defined(ENABLE_POWERSAVING)
#   if (((OPTION_POWERSAVING_METHOD) & (POWERSAVING_DISPLAY_DIM | POWERSAVING_DISPLAY_OFF)) && (OPTION_POWERSAVING_IDLE_SLEEP <= OPTION_POWERSAVING_IDLE_DIM))
#     error Power save sleep timeout must be greater than the dim timeout.
#   endif
# endif

/****/

# if defined(ENABLE_3V3FIX) && !defined(ENABLE_VSELECT)
#   warning Using 3V3FIX is best with VSELECT.
# endif

/****/

// Voltage Monitor
# if defined(OPTION_VOLTAGE_MONITOR_METHOD)

// Error if invalid value
#   if !(OPTION_VOLTAGE_MONITOR_METHOD >= 0 && OPTION_VOLTAGE_MONITOR_METHOD <= 3)
#     error !!! VALUE OF OPTION_VOLTAGE_MONITOR_METHOD IS INVALID !!!
#   endif /* OPTION_VOLTAGE_MONITOR_METHOD <0|>2 */

# endif /* OPTION_VOLTAGE_MONITOR_METHOD */

/****/

// VSELECT
# if defined(ENABLE_VSELECT)

#   if ISEMPTY(ENABLE_VSELECT)
#     undef ENABLE_VSELECT
#     define ENABLE_VSELECT 1
#   endif

// Error if not a supported hardware version
#   if (HW_VERSION < 4)
#     if defined(ALLOW_UNSAFE_CONFIG)
#       define IS_UNSAFE_CONFIG
#       warning Using VSELECT with hardware revisions other than 4 or 5 is not supported.
#     else /* !defined(ALLOW_UNSAFE_CONFIG) */
#       error Using VSELECT with hardware revisions other than 4 or 5 is not supported. \
              If you understand what you are doing you can define ALLOW_UNSAFE_CONFIG in Config.h to allow compiling.
#     endif /* ALLOW_UNSAFE_CONFIG */
#   endif /* !(HW4 | HW5 | SERIAL_MONITOR) */

// HW4 might work but needs tested. Make sure they know it's untested.
#   if HW_VERSION == 4
#     if defined(ALLOW_UNSAFE_CONFIG)
#       define IS_UNSAFE_CONFIG
#       warning Using VSELECT with HW4 is untested. Verification with a multimeter before use is strongly recommended. Please remember to report back with your findings.
#     else /* !defined(ALLOW_UNSAFE_CONFIG) */
#       error Using VSELECT with HW4 is untested. Verification with a multimeter before use is strongly recommended. \
              Define ALLOW_UNSAFE_CONFIG in Config.h to allow compiling. Please report back with your findings after testing!
#     endif /* ALLOW_UNSAFE_CONFIG */
#   endif /* HW4 */

# endif /* ENABLE_VSELECT */

/****/

# if !defined(NEEDS_UTIL_BITSET_TEMPLATE) && (defined(NEEDS_UTIL_BITSET_TEMPLATE_16) || defined(NEEDS_UTIL_BITSET_TEMPLATE_32) || defined(NEEDS_UTIL_BITSET_TEMPLATE_64))
#   define NEEDS_UTIL_BITSET_TEMPLATE
# endif /* !NEEDS_UTIL_BITSET_TEMPLATE && (NEEDS_UTIL_BITSET_TEMPLATE_16 || NEEDS_UTIL_BITSET_TEMPLATE_32 || NEEDS_UTIL_BITSET_TEMPLATE_64)*/

/****/

# if defined(ENABLE_NEOPIXEL)
#   if !defined(OPTION_NEOPIXEL_ORDER)
#     error !!! A VALUE FOR OPTION_NEOPIXEL_ORDER IS REQUIRED !!!
#   endif

#   if (!((OPTION_NEOPIXEL_ORDER >= NPXL_AUTO) && (OPTION_NEOPIXEL_ORDER <= NPXL_REVERSE)))
#     error !!! INVALID VALUE FOR OPTION_NEOPIXEL_ORDER !!!
#   endif /* OPTION_NEOPIXEL_ORDER */

# endif /* ENABLE_NEOPIXEL */

/****/

/* Firmware updater only works with HW3 and HW5 */
# if !(HW_VERSION == 5 || HW_VERSION == 3)
#   undef ENABLE_UPDATER
# endif

/****/

# if defined(OPTION_VOLTAGE_SPECIFIER)
#   if defined(ENABLE_VSELECT)
#     undef OPTION_VOLTAGE_SPECIFIER
#     define OPTION_VOLTAGE_SPECIFIER VLTSPC_NONE
#   elif ISEMPTY(OPTION_VOLTAGE_SPECIFIER)
#     error !!! OPTION_VOLTAGE_SPECIFIER CANNOT BE EMPTY !!!
#   elif ((OPTION_VOLTAGE_SPECIFIER < 0) || (OPTION_VOLTAGE_SPECIFIER > 3))
#     error !!! INVALID VALUE FOR OPTION_VOLTAGE_SPECIFIER !!!
#   endif
# endif

/****/

#pragma region Core Sanity Checks

// SMS
# if defined(ENABLE_SMS)

#   if !defined(OPTION_SMS_ADAPTER)
#     define OPTION_SMS_ADAPTER 0
#   elif !(OPTION_SMS_ADAPTER >= 0 && OPTION_SMS_ADAPTER <= OPTION_SMS_ADAPTER_MAX)
#     error !!! INVALID VALUE FOR OPTION_SMS_ADAPTER !!!
#   endif

#   if !defined(OPTION_GG_ADAPTER)
#     define OPTION_GG_ADAPTER 0
#   elif !(OPTION_GG_ADAPTER >= 0 && OPTION_GG_ADAPTER <= OPTION_GG_ADAPTER_MAX)
#     error !!! INVALID VALUE FOR OPTION_GG_ADAPTER !!!
#   endif

#   if !defined(OPTION_SG1000_ADAPTER)
#     define OPTION_SG1000_ADAPTER 0
#   elif !(OPTION_SG1000_ADAPTER >= 0 && OPTION_SG1000_ADAPTER <= OPTION_SG1000_ADAPTER_MAX)
#     error !!! INVALID VALUE FOR OPTION_SG1000_ADAPTER !!!
#   endif

# endif /* ENABLE_SMS */

/****/

# if defined(OPTION_PERFORMANCE_FLAGS)
#   if (OPTION_PERFORMANCE_FLAGS & PRFOPT_CRC32)
#     define __optimize_crc32 __attribute__((always_inline))
#   endif
#   if (OPTION_PERFORMANCE_FLAGS & PRFOPT_FILEWR)
#     define __optimize_file_write __attribute__((always_inline))
#   endif
#   if (OPTION_PERFORMANCE_FLAGS & PRFOPT_FILERD)
#     define __optimize_file_read __attribute__((always_inline))
#   endif
#   if (OPTION_PERFORMANCE_FLAGS & PRFOPT_SHRDFILE)
#     define __optimize_shared_file __attribute__((always_inline))
#   endif
#   if (OPTION_PERFORMANCE_FLAGS & PRFOPT_SPEEDORDEATH)
#     define OPTION_CRC32_LUT 1
#   endif
# endif

# if !defined(__optimize_crc32)
#   define __optimize_crc32
# endif
# if !defined(__optimize_file_read)
#   define __optimize_file_read
# endif
# if !defined(__optimize_file_write)
#   define __optimize_file_write
# endif
# if !defined(__optimize_shared_file)
#   define __optimize_shared_file
# endif
# if !defined(OPTION_CRC32_LUT)
#   define OPTION_CRC32_LUT 0
# endif

#endif /* OSCR_CONFIG_H_ */
