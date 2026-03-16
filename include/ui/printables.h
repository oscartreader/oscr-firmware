#pragma once
#ifndef OSCR_PRINTABLES_H_
# define OSCR_PRINTABLES_H_

# include "common/specializations.h"
# include "common/Types.h"
# include "api/Storage.h"
# include "ui.h"

namespace OSCR
{
  namespace Util
  {
    template <>
    struct is_printable<OSCR::Storage::Path>
    {
      static bool const value = true;
    };

    template <>
    struct is_printable<OSCR::CRC32::crc32_t>
    {
      static bool const value = true;
    };
  }

  namespace UI
  {
    extern void print(OSCR::Storage::Path const * path);
    extern void printLine(OSCR::Storage::Path const * path);
    extern void print(OSCR::Storage::Path const & path);
    extern void printLine(OSCR::Storage::Path const & path);

    extern void printSync(OSCR::Storage::Path const * path);
    extern void printLineSync(OSCR::Storage::Path const * path);
    extern void printSync(OSCR::Storage::Path const & path);
    extern void printLineSync(OSCR::Storage::Path const & path);

    extern void print(OSCR::CRC32::crc32_t const * crc32);
    extern void printLine(OSCR::CRC32::crc32_t const * crc32);
    extern void print(OSCR::CRC32::crc32_t const & crc32);
    extern void printLine(OSCR::CRC32::crc32_t const & crc32);

    extern void printSync(OSCR::CRC32::crc32_t const * crc32);
    extern void printLineSync(OSCR::CRC32::crc32_t const * crc32);
    extern void printSync(OSCR::CRC32::crc32_t const & crc32);
    extern void printLineSync(OSCR::CRC32::crc32_t const & crc32);
  }

  namespace Serial
  {
    extern void print(OSCR::Storage::Path const * path);
    extern void printLine(OSCR::Storage::Path const * path);
    extern void print(OSCR::Storage::Path const & path);
    extern void printLine(OSCR::Storage::Path const & path);

    extern void print(OSCR::CRC32::crc32_t const * crc32);
    extern void printLine(OSCR::CRC32::crc32_t const * crc32);
    extern void print(OSCR::CRC32::crc32_t const & crc32);
    extern void printLine(OSCR::CRC32::crc32_t const & crc32);

    extern void printSync(OSCR::Storage::Path const * path);
    extern void printLineSync(OSCR::Storage::Path const * path);
    extern void printSync(OSCR::Storage::Path const & path);
    extern void printLineSync(OSCR::Storage::Path const & path);

    extern void printSync(OSCR::CRC32::crc32_t const * crc32);
    extern void printLineSync(OSCR::CRC32::crc32_t const * crc32);
    extern void printSync(OSCR::CRC32::crc32_t const & crc32);
    extern void printLineSync(OSCR::CRC32::crc32_t const & crc32);
  }
}

#endif /* OSCR_PRINTABLES_H_ */
