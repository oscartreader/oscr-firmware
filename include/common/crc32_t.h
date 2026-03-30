#pragma once
#ifndef OSCR_CRC32_T_H_
# define OSCR_CRC32_T_H_

# include "config.h"
# include "syslibinc.h"

namespace OSCR::CRC32
{
# if OPTION_CRC32_LUT
  extern uint32_t const crc_32_tab[];
# else
  extern uint32_t const PROGMEM crc_32_tab[];
# endif

  /**
   * @brief Interface for handling CRC32 values.
   */
  typedef union crc32_t
  {
    // Accessors
    uint32_t value; //!< The CRC32 (binary, host endian)
    uint8_t b[4]; //!< The 4 bytes that make up the CRC32

    // Constructors

    /**
     * @brief Create an empty CRC32 interface
     */
    crc32_t();

    /**
     * @brief Create a CRC32 interface using the provided value.
     *
     * @param v   The binary value of the CRC32
     */
    explicit crc32_t(uint32_t v);

    //! @cond

    // Implicit Casts
    operator uint32_t() const;

    bool operator==(crc32_t const & rhs) const __hot;

    crc32_t & operator=(uint32_t v) __hot;

# if (OPTION_OPTIMIZE_CRC32)
    __optimize_crc32
    crc32_t & operator+=(uint8_t data)
    {
      update(value, data);
      return *this;
    }

    __optimize_crc32
    crc32_t & operator+=(uint16_t data)
    {
      update(value, (uint8_t)(( data >> (  8 ) ) & ( 0xFF )));
      update(value, (uint8_t)(( data           ) & ( 0xFF )));
      return *this;
    }

    __optimize_crc32
    crc32_t & operator+=(uint32_t data)
    {
      update(value, (uint8_t)(( data >> ( 24 ) ) & ( 0xFF )));
      update(value, (uint8_t)(( data >> ( 16 ) ) & ( 0xFF )));
      update(value, (uint8_t)(( data >> (  8 ) ) & ( 0xFF )));
      update(value, (uint8_t)(( data           ) & ( 0xFF )));
      return *this;
    }

    __optimize_crc32
    void next(uint8_t const * data)
    {
      update(value, *data);
    }

    __optimize_crc32
    void next(uint8_t data)
    {
      update(value, data);
    }
# else
    crc32_t & operator+=(uint8_t data);
    crc32_t & operator+=(uint16_t data);
    crc32_t & operator+=(uint32_t data);

    void next(uint8_t const * data);
    void next(uint8_t data);
# endif
    // Operators
    uint8_t & operator[](size_t idx) __hot;
    uint8_t const & operator[](size_t idx) const __hot;

    //! @endcond

    // Methods

    void reset() __hot;
    void done() __hot;

    /**
     * Copy a human readable CRC32 string into a buffer.
     *
     * @param buffer  The pointer to the destination
     * @param sizeOf  The size of the destination (default: 9)
     */
    void human(char * buffer, uint_fast8_t sizeOf = 9) const;

# if (OPTION_OPTIMIZE_CRC32)
    private:
#   if OPTION_CRC32_LUT
      __optimize_crc32 static
      void update(uint32_t & crcValue, uint8_t const & data)
      {
        uint8_t idx = ((crcValue) ^ (data)) & 0xFF;
        uint32_t tab_value = OSCR::CRC32::crc_32_tab[idx];
        crcValue = (uint32_t)(tab_value ^ ((crcValue) >> 8));
      }
#   else
      __optimize_crc32 static
      void update(uint32_t & crcValue, uint8_t const & data)
      {
        uint8_t idx = ((crcValue) ^ (data)) & 0xFF;
        uint32_t tab_value = pgm_read_dword(OSCR::CRC32::crc_32_tab + idx);
        crcValue = (uint32_t)(tab_value ^ ((crcValue) >> 8));
      }
#   endif
# endif
  } crc32_t;
}

#endif /* OSCR_CRC32_T_H_ */