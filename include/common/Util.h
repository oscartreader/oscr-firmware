/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_UTIL_H_
#define OSCR_UTIL_H_

# include "config.h"
# include "syslibinc.h"
# include "common/specializations.h"

namespace OSCR
{
  /**
   * @brief Utility methods
   */
  namespace Util
  {
    /**
     * Maximum size of the internal buffer used by functions which copy strings from flash.
     */
    constexpr size_t const flashBufferStrSize = 100;

    template <__TEMPLATE_AUTO_UINT8 base, typename T,
              if_is_integer_t<T> Enable = true,
              enable_if_t<2 == base, bool> IsEnabled = true>
    extern T power(T n);

    template <__TEMPLATE_AUTO_UINT8 base, typename T,
              if_is_integer_t<T> Enable = true,
              enable_if_t<8 == base, bool> IsEnabled = true>
    extern T power(T n);

    template <__TEMPLATE_AUTO_UINT8 base, typename T,
              if_is_integer_t<T> Enable = true,
              enable_if_t<10 == base, bool> IsEnabled = true>
    extern T power(T n);

    template <__TEMPLATE_AUTO_UINT8 base, typename T,
              if_is_integer_t<T> Enable = true,
              enable_if_t<16 == base, bool> IsEnabled = true>
    extern T power(T n);

    template <typename T,
              if_is_integer_t<T> Enable = true>
    extern T power(T b, T n);

    [[noreturn]] static inline void unreachable()
    {
      // Uses compiler specific extensions if possible.
      // Even if no extension is used, undefined behavior is still raised by
      // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
      __assume(false);
#else // GCC, Clang
      __builtin_unreachable();
#endif
    }

    /**
     * Convert a string representation to an integer.
     *
     * @param buffer  The array of characters (string).
     * @param dest    The destination.
     *
     * @returns `true` on success, `false` on failure.
     *
     * @note Supports decimal, hexidecimal (0x...), and octal (0...), as well as ignores
     *       whitespace and commas, even in the middle of the number. When the number is
     *       negative, the first character must be a minus (`-`) symbol.
     */
    bool strToInt(char const buffer[], int32_t & dest);

    /**
     * Convert a string representation to an unsigned integer.
     *
     * @param buffer  The array of characters (string).
     * @param dest    The destination.
     *
     * @returns `true` on success, `false` on failure.
     *
     * @note Supports decimal, hexidecimal (0x...), and octal (0...), as well as ignores
     *       whitespace and commas, even in the middle of the number.
     */
    bool strToInt(char const buffer[], uint32_t & dest);

    bool copyStr(char buffer[], size_t bufferSize, char const * str);

    bool copyStrUpr(char buffer[], size_t bufferSize, char const * str);
    bool copyStrLwr(char buffer[], size_t bufferSize, char const * str);

    //template <bool prfx = true>
    //int toHex(char * buff, size_t len, uint8_t number);

    template <bool prfx = true,
              typename T,
              if_is_any_unsigned_t<T> Enable = true>
    extern int toHex(char * buff, size_t len, T number);

    /**
     * Copy a string of a specific length from PROGMEM to SRAM.
     *
     * @param buffer Pointer to the destination in SRAM.
     * @param str Pointer to the source in PROGMEM.
     * @param strSize Maximum number of characters to be copied from source (usually `sizeof(buffer)`, must be >0).
     *
     * @return `true` if `str` was copied in its entirety, `false` if it was truncated.
     */
    bool copyStr_P(char buffer[], size_t bufferSize, char const * str);
    bool copyStr_P(char buffer[], size_t bufferSize, __FlashStringHelper const * str);

    bool copyStrUpr_P(char buffer[], size_t bufferSize, char const * str);
    bool copyStrUpr_P(char buffer[], size_t bufferSize, __FlashStringHelper const * str);

    bool copyStrLwr_P(char buffer[], size_t bufferSize, char const * str);
    bool copyStrLwr_P(char buffer[], size_t bufferSize, __FlashStringHelper const * str);

    /**
     * Apply a template of `templateStr` stored in PROGMEM using parameter `num` to `buffer` with a maximum size of `n`.
     *
     * @param buffer Pointer to the destination in SRAM.
     * @param bufferSize Maximum number of characters to be copied from source (usually `sizeof(buffer)`, must be >0).
     * @param templateStr Pointer to the template in PROGMEM. Should contain `%d` (or equivilent).
     * @param num The integer to use.
     *
     * @return `true` if `buffer` fit the templated string, `false` if it was truncated.
     */
    bool applyTemplate_P(char buffer[], size_t bufferSize, char const * templateStr, int32_t num);
    bool applyTemplate_P(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, int32_t num);
    bool applyTemplate_P(char buffer[], size_t bufferSize, char const * templateStr, uint32_t num);
    bool applyTemplate_P(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, uint32_t num);

    /**
     * Apply a template of `templateStr` stored in PROGMEM using parameter `flashStr` to `buffer` with a maximum size of `n`.
     *
     * @param buffer Pointer to the destination in SRAM.
     * @param bufferSize Maximum number of characters to be copied from source (usually `sizeof(buffer)`, must be >0).
     * @param templateStr Pointer to the template in PROGMEM. Should contain `%s` (or equivilent).
     * @param flashStr Pointer to the string parameter in PROGMEM. Maximum size (including the `NUL` terminator) is configured by `flashBufferStrSize`.
     *
     * @return `true` if `buffer` fit the templated string, `false` if it was truncated.
     *
     * @see flashBufferStrSize
     */
    bool applyTemplate_P(char buffer[], size_t bufferSize, char const * templateStr, char const * flashStr);
    bool applyTemplate_P(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, char const * flashStr);
    bool applyTemplate_P(char buffer[], size_t bufferSize, char const * templateStr, __FlashStringHelper const * flashStr);
    bool applyTemplate_P(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, __FlashStringHelper const * flashStr);

    /**
     * Apply a template of `templateStr` stored in PROGMEM using parameter `str` to `buffer` with a maximum size of `n`.
     *
     * @param buffer Pointer to the destination in SRAM.
     * @param bufferSize Maximum number of characters to be copied from source (usually `sizeof(buffer)`, must be >0).
     * @param templateStr Pointer to the template in PROGMEM. Should contain `%s` (or equivilent).
     * @param str Pointer to the string parameter in SRAM.
     *
     * @return `true` if `buffer` fit the templated string, `false` if it was truncated.
     */
    bool applyTemplate(char buffer[], size_t bufferSize, char const * templateStr, char const * str);
    bool applyTemplate(char buffer[], size_t bufferSize, __FlashStringHelper const * templateStr, char const * str);

    extern bool isAlphaNumeric(uint8_t src);

    /**
     * Perform an in-place swap of two variables of the same type.
     *
     * @param a   The first variable.
     * @param b   The second variable.
     *
     * @tparam T  The type of two variables.
     */
    template <typename T>
    extern void swap(T &a, T &b);

    /**
     * Return the smaller of two values.
     *
     * @param a       The first value.
     * @param b       The second value.
     *
     * @note  If the output is unsigned and the inputs are signed,
     *        negative numbers will be set to zero before being
     *        compared
     *
     * @tparam Tout   The type of the output value.
     * @tparam Tin    The type of the input values.
     *
     * @sa minOf_CX()
     */
    template <typename Tout,
              typename Tin,
              if_is_any_number_t<Tout> outEnable = true,
              if_is_any_number_t<Tin> inEnable = true>
    inline Tout minOf(Tin a, Tin b)
    {
      if __if_constexpr ((is_unsigned_v<Tout>) && (is_signed_v<Tin>))
      {
        if (a < 0) a = 0;
        if (b < 0) b = 0;
      }

      if (a > b) return b;
      return a;
    }

    /**
     * Return the larger of two values.
     *
     * @param a       The first value.
     * @param b       The second value.
     *
     * @note  If the output is unsigned and the inputs are signed,
     *        negative numbers will be set to zero before being
     *        compared
     *
     * @tparam Tout   The type of the output value.
     * @tparam Tin    The type of the input values.
     *
     * @sa maxOf_CX
     */
    template <typename Tout,
              typename Tin,
              if_is_any_number_t<Tout> outEnable = true,
              if_is_any_number_t<Tin> inEnable = true>
    inline Tout maxOf(Tin a, Tin b)
    {
      if __if_constexpr ((is_unsigned_v<Tout>) && (is_signed_v<Tin>))
      {
        if (a < 0) a = 0;
        if (b < 0) b = 0;
      }

      if (a < b) return b;
      return a;
    }

    /**
     * Return the smaller of two values of different signedness.
     *
     * @param a       The first value.
     * @param b       The second value.
     *
     * @tparam Tout   The type of the output value.
     * @tparam Tin1   The type of the first value.
     * @tparam Tin2   The type of the second value.
     */
    template <typename Tout,
              typename Tin1,
              typename Tin2,
              if_is_any_number_t<Tout> outEnable = true,
              if_is_any_number_t<Tin1> in1Enable = true,
              if_is_any_number_t<Tin2> in2Enable = true>
    inline Tout minOf(Tin1 a, Tin2 b)
    {
      // If output is unsigned
      if __if_constexpr (is_unsigned_v<Tout>)
      {
        // If `a` is signed+negative, return 0
        if __if_constexpr (is_signed_v<Tin1>)
        {
          if (a < 0) return 0;
        }

        // If `b` is signed+negative, return 0
        if __if_constexpr (is_signed_v<Tin2>)
        {
          if (b < 0) return 0;
        }
      }

      // signed `a`, unsigned `b`
      if __if_constexpr ((is_signed_v<Tin1>) && (is_unsigned_v<Tin2>))
      {
        // since `b` is unsigned, it can't be lower than zero
        if (a < 0) return a;

        // `a` is not negative, so it is safe to cast to unsigned
        if (((make_unsigned_t<Tin1>)a) > b) return b;
        return a;
      }
      else if __if_constexpr ((is_unsigned_v<Tin1>) && (is_signed_v<Tin2>)) // unsigned `a`, signed `b`
      {
        // since `a` is unsigned, it can't be lower than zero
        if (b < 0) return b;

        // `b` is not negative, so it is safe to cast to unsigned
        if (a > ((make_unsigned_t<Tin2>)b)) return b;

        return a;
      }
      else // same signedness
      {
        if (a > b) return b;
        return a;
      }
    }

    /**
     * Return the larger of two values of different signedness.
     *
     * @param a       The first value.
     * @param b       The second value.
     *
     * @tparam Tout   The type of the output value.
     * @tparam Tin1   The type of the first value.
     * @tparam Tin2   The type of the second value.
     */
    template <typename Tout,
              typename Tin1,
              typename Tin2,
              if_is_any_number_t<Tout> outEnable = true,
              if_is_any_number_t<Tin1> in1Enable = true,
              if_is_any_number_t<Tin2> in2Enable = true>
    inline Tout maxOf(Tin1 a, Tin2 b)
    {
      // If `Tin1`(`a`) is signed and `Tin2` (`b`) is unsigned
      if __if_constexpr ((is_signed_v<Tin1>) && (is_unsigned_v<Tin2>))
      {
        // since `b` is unsigned, it can't be lower than zero
        if (a < 0) return b;

        // Cast `a` to signed before comparing
        if (((make_unsigned_t<Tin1>)a) > b) return b;

        return a;
      }
      else if __if_constexpr ((is_unsigned_v<Tin1>) && (is_signed_v<Tin2>)) // If `Tin1`(`a`) is unsigned and `Tin2` (`b`) is signed
      {
        // since `a` is unsigned, it can't be lower than zero
        if (b < 0) return a;

        // Cast `b` to signed before comparing
        if (a > (make_unsigned_t<Tin2>(b))) return b;

        return a;
      }
      else
      {
        if (a < b) return b;
        return a;
      }
    }

    /**
     * `constexpr` implementation of `minOf()`.
     *
     * @param a       The first value.
     * @param b       The second value.
     *
     * @tparam Tout   The type of the output value.
     * @tparam Tin    The type of the input values.
     *
     * @sa minOf()
     */
    template <typename Tout,
              typename Tin,
              if_is_any_number_t<Tout> outEnable = true,
              if_is_any_number_t<Tin> inEnable = true>
    inline constexpr Tout minOf_CX(Tin a, Tin b)
    {
      if ((is_unsigned_v<Tout>) && (is_signed_v<Tin>))
      {
        if (a < 0) a = 0;
        if (b < 0) b = 0;
      }

      if (a > b) return b;
      return a;
    }

    /**
     * `constexpr` implementation of `maxOf()`.
     *
     * @param a       The first value.
     * @param b       The second value.
     *
     * @tparam Tout   The type of the output value.
     * @tparam Tin    The type of the input values.
     *
     * @sa maxOf()
     */
    template <typename Tout,
              typename Tin,
              if_is_any_number_t<Tout> outEnable = true,
              if_is_any_number_t<Tin> inEnable = true>
    inline constexpr Tout maxOf_CX(Tin a, Tin b)
    {
      if ((is_unsigned_v<Tout>) && (is_signed_v<Tin>))
      {
        if (a < 0) a = 0;
        if (b < 0) b = 0;
      }

      if (a < b) return b;
      return a;
    }

    /**
     * Clamp the input to a minimum and maximum value.
     *
     * @note
     * For the sake of program size, cast parameter values to consistent
     * types where possible.
     *
     * @param input   The number to clamp.
     * @param minimum The minimum value.
     * @param maximum The maximum value.
     *
     * @return The clamped value (Tout or same type as input type if unspecified).
     *
     * @tparam Tin    The type of the input value.
     * @tparam Tmin   The type of the minimum value.
     * @tparam Tmax   The type of the maximum value.
     * @tparam Tout   The type of the output value (default: Tin).
     */
    template <typename Tin,
              typename Tmin,
              typename Tmax,
              typename Tout = Tin,
              if_is_any_number_t<Tin> inEnable = true,
              if_is_any_number_t<Tmin> minEnable = true,
              if_is_any_number_t<Tmax> maxEnable = true,
              if_is_any_number_t<Tout> outEnable = true>
    inline Tout clamp(Tin input, Tmin minimum, Tmax maximum)
    {
      return minOf<Tout, Tin, Tmax>(maxOf<Tin, Tin, Tmin>(input, minimum), maximum);
    }

    /**
     * Clamp the input to a minimum and maximum value.
     *
     * @param input   The number to clamp.
     * @param minimum The minimum value.
     * @param maximum The maximum value.
     *
     * @return The clamped value
     *
     * @tparam Tout   The type of the output, min, and max values.
     * @tparam Tin    The type of the input value.
     */
    template <typename Tout,
              typename Tin,
              if_is_any_number_t<Tout> outEnable = true,
              if_is_any_number_t<Tin> inEnable = true>
    inline Tout clamp(Tin input, Tout minimum, Tout maximum)
    {
      if __if_constexpr ((is_unsigned_v<Tout>) && (is_signed_v<Tin>))
      {
        if (input < 0) return minimum;
      }

      if (input < minimum) return minimum;
      if (input > maximum) return maximum;
      return input;
    }

    /**
     * `constexpr` implementation of `clamp()`
     *
     * @param input   The number to clamp.
     * @param minimum The minimum value.
     * @param maximum The maximum value.
     *
     * @return The clamped value
     *
     * @tparam Tout   The type of the output, min, and max values.
     * @tparam Tin    The type of the input value.
     */
    template <typename Tout,
              typename Tin,
              if_is_any_number_t<Tout> outEnable = true,
              if_is_any_number_t<Tin> inEnable = true>
    inline constexpr Tout clamp_CX(Tin input, Tout minimum, Tout maximum)
    {
      if ((is_unsigned_v<Tout>) && (is_signed_v<Tin>))
      {
        if (input < 0) return minimum;
      }

      if (input < minimum) return minimum;
      if (input > maximum) return maximum;

      return input;
    }

    /**
     * @class clamped_value
     * @brief Wrapper for clamping variable values.
     *
     * A wrapper for clamping class values and preventing attempts
     * to set a value outside of a specified range.
     *
     * @note
     * This wrapper is not a replacement for sanity checks as it will
     * not attempt to manipulate the values beyond keeping them within
     * the specified bounds. It is only intended to help with keeping
     * the program from accessing invalid memory.
     */
    template <typename Value, Value LowerLimit, Value UpperLimit,
              if_is_integer_t<Value> = true>
    class clamped_value
    {
      public:
        clamped_value(Value&& value) : value_{clamp(value, LowerLimit, UpperLimit)} {}
        clamped_value(int&& value) : value_{clamp<int, Value, Value, Value>(value, LowerLimit, UpperLimit)} {}
        clamped_value(unsigned int&& value) : value_{clamp<unsigned int, Value, Value, Value>(value, LowerLimit, UpperLimit)} {}

        constexpr operator Value&() { return value_; }
        constexpr operator Value() const { return value_; }

      private:
        Value value_;
    };

# if defined(NEEDS_UTIL_BITSET_TEMPLATE)
    template <typename BitsetType = uint8_t,
              if_is_integer_t<BitsetType> = true>
# else /* !NEEDS_UTIL_BITSET_TEMPLATE */
    typedef uint8_t BitsetType;

    class bitset
# endif /* NEEDS_UTIL_BITSET_TEMPLATE */
    {
    public:
      class reference
      {
        friend class bitset;

        bitset * __bitset;
        size_t pos;

      public:
        constexpr reference(reference const &) = default;
        constexpr reference(bitset & __b, size_t __pos): __bitset(&__b), pos(__pos)
        {
          // ...
        }

        __relaxed_constexpr
        reference & operator=(bool value) noexcept              // for b[i] = x;
        {
          __bitset->set(this->pos, value);
          return *this;
        }

        __relaxed_constexpr
        reference & operator=(reference const & ref) noexcept // for b[i] = b[j];
        {
          __bitset->set(this->pos, ref.__bitset->test(ref.pos));
          return *this;
        }

        constexpr bool operator~() const noexcept                        // flips the bit
        {
          return !__bitset->test(this->pos);
        }

        constexpr operator bool() const noexcept                         // for x = b[i];
        {
          return __bitset->test(this->pos);
        }

        __relaxed_constexpr
        reference & flip() noexcept                             // for b[i].flip();
        {
          __bitset->set(this->pos, !__bitset->test(this->pos));
          return *this;
        }
      };

      friend class reference;

    protected:
      uint8_t width = 0;
      BitsetType bits = 0;

    public:
      // constructors
      constexpr bitset(uint8_t size) noexcept: width(size)
      {
        // ...
      }

      constexpr bitset(uint8_t size, BitsetType val) noexcept: width(size), bits(val)
      {
        // ...
      }

      // bitset operations
      __relaxed_constexpr
      bitset & operator&=(bitset const & rhs) noexcept
      {
        this->bits &= rhs.bits;
        return *this;
      }

      __relaxed_constexpr
      bitset & operator|=(bitset const & rhs) noexcept
      {
        this->bits |= rhs.bits;
        return *this;
      }

      __relaxed_constexpr
      bitset & operator^=(bitset const & rhs) noexcept
      {
        this->bits ^= rhs.bits;
        return *this;
      }

      __relaxed_constexpr
      bitset & operator<<=(size_t shift) noexcept
      {
        this->bits <<= shift;
        return *this;
      }

      __relaxed_constexpr
      bitset & operator>>=(size_t shift) noexcept
      {
        this->bits >>= shift;
        return *this;
      }

      __relaxed_constexpr
      bitset   operator<<(size_t shift) const noexcept
      {
        return bitset(*this) <<= shift;
      }

      __relaxed_constexpr
      bitset   operator>>(size_t shift) const noexcept
      {
        return bitset(*this) >>= shift;
      }

      __relaxed_constexpr
      bitset & set() noexcept
      {
        for (size_t __position = 0; __position < this->width; __position++)
        {
          this->set(__position, true);
        }

        return *this;
      }

      __relaxed_constexpr
      bitset & set(size_t __position, bool val = true)
      {
        if (__position > this->width) return *this;

        if (val) bits |= (1 << __position);
        else bits &= ~(1 << __position);

        return *this;
      }

      __relaxed_constexpr
      bitset & reset() noexcept
      {
        this->bits = 0;
        return *this;
      }

      __relaxed_constexpr
      bitset & reset(size_t __position)
      {
        this->set(__position, 0);
        return *this;
      }
      constexpr bitset   operator~() const noexcept;

      __relaxed_constexpr
      bitset & flip() noexcept
      {
        for (size_t __position = 0; __position < this->width; __position++)
        {
          this->flip(__position);
        }

        return *this;
      }

      __relaxed_constexpr
      bitset & flip(size_t __position)
      {
        this->set(__position, !this->test(__position));
        return *this;
      }

      // element access
      constexpr bool operator[](size_t __position) const
      {
        return !!(bits & (1 << __position));
      }

      __relaxed_constexpr
      reference operator[](size_t __position)
      {
        return reference(*this, __position);
      }

      // observers
      __relaxed_constexpr
      size_t count() const noexcept
      {
        size_t bitcount = 0;

        for (size_t i = 0; i < this->width; i++)
        {
          bitcount += this->test(i);
        }

        return bitcount;
      }

      constexpr size_t size() const noexcept
      {
        return this->width;
      }

      constexpr bool operator==(bitset const & rhs) const noexcept
      {
        return ((this->width == rhs.width) && (this->bits == rhs.bits));
      }

      constexpr bool test(size_t __position) const
      {
        return !!(bits & (1 << __position));
      }

      __relaxed_constexpr
      bool all() const noexcept
      {
        size_t const allbits = ~0U >> ((8 * sizeof(BitsetType)) - this->width);
        return !!(bits & allbits);
      }

      constexpr bool any() const noexcept
      {
        return !!bits;
      }

      constexpr bool none() const noexcept
      {
        return !bits;
      }
    };
  }

# if defined(NEEDS_UTIL_BITSET_TEMPLATE)
  //! @cond

  template class bitset<uint8_t>;

#   if defined(NEEDS_UTIL_BITSET_TEMPLATE_16) || NEEDS_UTIL_BITSET_TEMPLATE >= 16
  template class bitset<uint16_t>;
#   endif /* NEEDS_UTIL_BITSET_TEMPLATE_16 || NEEDS_UTIL_BITSET_TEMPLATE >= 16 */

#   if defined(NEEDS_UTIL_BITSET_TEMPLATE_32) || NEEDS_UTIL_BITSET_TEMPLATE >= 32
  template class bitset<uint32_t>;
#   endif /* NEEDS_UTIL_BITSET_TEMPLATE_32 || NEEDS_UTIL_BITSET_TEMPLATE >= 32 */

#   if defined(NEEDS_UTIL_BITSET_TEMPLATE_64) || NEEDS_UTIL_BITSET_TEMPLATE >= 64
  template class bitset<uint64_t>;
#   endif /* NEEDS_UTIL_BITSET_TEMPLATE_64 || NEEDS_UTIL_BITSET_TEMPLATE >= 64 */

  //! @endcond
# endif
}

#endif /* !OSCR_UTIL_H_ */
