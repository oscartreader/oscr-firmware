#pragma once
#ifndef OSCR_CRC32_H_
# define OSCR_CRC32_H_

# include "config.h"
# include "syslibinc.h"
# include "common/Types.h"
# include "api/Storage.h"

#define UPDATE_CRC(crc, ch) \
  do { \
    uint8_t idx = ((crc) ^ (ch)) & 0xFF; \
    uint32_t tab_value = pgm_read_dword(OSCR::CRC32::crc_32_tab + idx); \
    (crc) = tab_value ^ ((crc) >> 8); \
  } while (0)

namespace OSCR
{
  namespace CRC32
  {
#if OPTION_CRC32_LUT
    extern uint32_t const crc_32_tab[];
#else
    extern uint32_t const PROGMEM crc_32_tab[];
#endif

    extern crc32_t current;

    extern void reset();
    extern void next(uint8_t const * data);
    extern void next(uint8_t data);
    extern void done();

    extern uint32_t calculateCRC(uint8_t const * buffer, size_t length);
    extern uint32_t calculateCRC(OSCR::Storage::File & infile);
    extern uint32_t calculateCRC(char const * fileName, char const * folder, uint32_t offset);
  }

  namespace Cores
  {
    using OSCR::CRC32::crc32_t;
  }

  namespace Databases
  {
    using OSCR::CRC32::crc32_t;
  }

  namespace CRDB
  {
    using OSCR::CRC32::crc32_t;
  }
}

#endif /* OSCR_CRC32_H_ */
