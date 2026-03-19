/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#ifndef OSCR_LANG_UNITS_H_
# define OSCR_LANG_UNITS_H_

# include "ui/l10n.h"

# define OSCR_UNIT_BIT "b"
# define OSCR_UNIT_BYTE "B"

# define OSCR_UNIT_KILO "K"
# define OSCR_UNIT_MEGA "M"
# define OSCR_UNIT_GIGA "G"

# define OSCR_UNIT_KILOBIT OSCR_UNIT_KILO OSCR_UNIT_BIT
# define OSCR_UNIT_MEGABIT OSCR_UNIT_MEGA OSCR_UNIT_BIT
# define OSCR_UNIT_GIGABIT OSCR_UNIT_GIGA OSCR_UNIT_BIT

# define OSCR_UNIT_KILOBYTE OSCR_UNIT_KILO OSCR_UNIT_BYTE
# define OSCR_UNIT_MEGABYTE OSCR_UNIT_MEGA OSCR_UNIT_BYTE
# define OSCR_UNIT_GIGABYTE OSCR_UNIT_GIGA OSCR_UNIT_BYTE

# define OSCR_UNIT_SPACE " "

# define OSCR_UNIT_STRING(NUMSTR, UNITTYPE) NUMSTR OSCR_UNIT_SPACE OSCR_UNIT_ ## UNITTYPE
# define OSCR_UNIT_TEMPLATE(UNITTYPE) "%" PRIu32 OSCR_UNIT_SPACE OSCR_UNIT_ ## UNITTYPE

namespace OSCR::Strings
{
  namespace Units
  {
    constexpr char const PROGMEM Size32B[]    = OSCR_UNIT_STRING("32", BYTE);
    constexpr char const PROGMEM Size64B[]    = OSCR_UNIT_STRING("64", BYTE);
    constexpr char const PROGMEM Size128B[]   = OSCR_UNIT_STRING("128", BYTE);
    constexpr char const PROGMEM Size256B[]   = OSCR_UNIT_STRING("256", BYTE);
    constexpr char const PROGMEM Size512B[]   = OSCR_UNIT_STRING("512", BYTE);
    constexpr char const PROGMEM Size1KB[]    = OSCR_UNIT_STRING("1", KILOBYTE);
    constexpr char const PROGMEM Size2KB[]    = OSCR_UNIT_STRING("2", KILOBYTE);
    constexpr char const PROGMEM Size4KB[]    = OSCR_UNIT_STRING("4", KILOBYTE);
    constexpr char const PROGMEM Size8KB[]    = OSCR_UNIT_STRING("8", KILOBYTE);
    constexpr char const PROGMEM Size16KB[]   = OSCR_UNIT_STRING("16", KILOBYTE);
    constexpr char const PROGMEM Size24KB[]   = OSCR_UNIT_STRING("24", KILOBYTE);
    constexpr char const PROGMEM Size32KB[]   = OSCR_UNIT_STRING("32", KILOBYTE);
    constexpr char const PROGMEM Size64KB[]   = OSCR_UNIT_STRING("64", KILOBYTE);
    constexpr char const PROGMEM Size128KB[]  = OSCR_UNIT_STRING("128", KILOBYTE);
    constexpr char const PROGMEM Size256KB[]  = OSCR_UNIT_STRING("256", KILOBYTE);
    constexpr char const PROGMEM Size512KB[]  = OSCR_UNIT_STRING("512", KILOBYTE);
    constexpr char const PROGMEM Size1MB[]    = OSCR_UNIT_STRING("1", MEGABYTE);
    constexpr char const PROGMEM Size2MB[]    = OSCR_UNIT_STRING("2", MEGABYTE);
    constexpr char const PROGMEM Size4MB[]    = OSCR_UNIT_STRING("4", MEGABYTE);
    constexpr char const PROGMEM Size8MB[]    = OSCR_UNIT_STRING("8", MEGABYTE);
    constexpr char const PROGMEM Size12MB[]   = OSCR_UNIT_STRING("12", MEGABYTE);
    constexpr char const PROGMEM Size16MB[]   = OSCR_UNIT_STRING("16", MEGABYTE);
    constexpr char const PROGMEM Size32MB[]   = OSCR_UNIT_STRING("32", MEGABYTE);
    constexpr char const PROGMEM Size64MB[]   = OSCR_UNIT_STRING("64", MEGABYTE);
    constexpr char const PROGMEM Size128MB[]  = OSCR_UNIT_STRING("128", MEGABYTE);

    constexpr char const PROGMEM b[]  = OSCR_UNIT_BIT;
    constexpr char const PROGMEM K[]  = OSCR_UNIT_KILO;
    constexpr char const PROGMEM M[]  = OSCR_UNIT_MEGA;
    constexpr char const PROGMEM G[]  = OSCR_UNIT_GIGA;

    constexpr char const PROGMEM B[]  = OSCR_UNIT_BYTE;
    constexpr char const PROGMEM KB[] = OSCR_UNIT_KILOBYTE;
    constexpr char const PROGMEM MB[] = OSCR_UNIT_MEGABYTE;
    constexpr char const PROGMEM GB[] = OSCR_UNIT_GIGABYTE;
  } /* namespace Units */

  // (Unit) Templates
  namespace Templates
  {
    constexpr char const PROGMEM PaddedHex2[]         = "%02X";
    constexpr char const PROGMEM SizeBits[]           = OSCR_UNIT_TEMPLATE(BIT);
    constexpr char const PROGMEM SizeK[]              = OSCR_UNIT_TEMPLATE(KILOBIT);
    constexpr char const PROGMEM SizeM[]              = OSCR_UNIT_TEMPLATE(MEGABIT);
    constexpr char const PROGMEM SizeG[]              = OSCR_UNIT_TEMPLATE(GIGABIT);

    constexpr char const PROGMEM SizeBytes[]          = OSCR_UNIT_TEMPLATE(BYTE);
    constexpr char const PROGMEM SizeKB[]             = OSCR_UNIT_TEMPLATE(KILOBYTE);
    constexpr char const PROGMEM SizeMB[]             = OSCR_UNIT_TEMPLATE(MEGABYTE);
    constexpr char const PROGMEM SizeGB[]             = OSCR_UNIT_TEMPLATE(GIGABYTE);
  } /* namespace Templates */
} /* namespace OSCR::Strings */

# undef OSCR_UNIT_BIT
# undef OSCR_UNIT_BYTE
# undef OSCR_UNIT_KILO
# undef OSCR_UNIT_MEGA
# undef OSCR_UNIT_GIGA
# undef OSCR_UNIT_KILOBIT
# undef OSCR_UNIT_MEGABIT
# undef OSCR_UNIT_GIGABIT
# undef OSCR_UNIT_KILOBYTE
# undef OSCR_UNIT_MEGABYTE
# undef OSCR_UNIT_GIGABYTE
# undef OSCR_UNIT_SPACE
# undef OSCR_UNIT_STRING
# undef OSCR_UNIT_TEMPLATE

#endif /* OSCR_LANG_SYMBOLS_H_ */
