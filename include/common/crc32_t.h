#pragma once
#ifndef OSCR_CRC32_T_H_
# define OSCR_CRC32_T_H_

# include "config.h"
# include "syslibinc.h"

namespace OSCR::CRC32
{
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

    bool operator==(crc32_t const & rhs) const;

    crc32_t & operator=(uint32_t v);

    crc32_t & operator+=(uint8_t data);
    crc32_t & operator+=(uint16_t data);
    crc32_t & operator+=(uint32_t data);

    // Operators
    uint8_t & operator[](size_t idx);
    uint8_t const & operator[](size_t idx) const;

    //! @endcond

    // Methods

    void reset();
    void next(uint8_t const * data);
    void next(uint8_t data);
    void done();

    /**
     * Copy a human readable CRC32 string into a buffer.
     *
     * @param buffer  The pointer to the destination
     * @param sizeOf  The size of the destination (default: 9)
     */
    void human(char * buffer, uint_fast8_t sizeOf = 9) const;
  } crc32_t;
}

#endif /* OSCR_CRC32_T_H_ */