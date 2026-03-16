/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#ifndef OSCR_SPECIALIZATIONS_H_
# define OSCR_SPECIALIZATIONS_H_

# include <stdint.h>
# include <limits.h>
# include "macros.h"
# include "common/Types.h"

#define __spec_def(NAME, ...)   template<__VA_ARGS__> struct NAME
#define __spec_cond(NAME, ...)  template<> struct NAME<__VA_ARGS__>
#define __spec_template(...)    template<__VA_ARGS__> struct
#define __spec_enabled          public enable_spec {}
#define __spec_disabled         public disable_spec {}

#define __spec_v(NAME)          template <class T> inline constexpr bool NAME ## _v = NAME ## <T>::value;

namespace OSCR
{
  //! @cond
  // Note: We don't include template specializations in docs
  //

  namespace Util
  {

    template <bool B,
              class T = void>
    struct enable_if {};

    template <class T>
    struct enable_if<true, T>
    {
      typedef T type;
    };

    template <bool B, class T = void >
    using enable_if_t = typename enable_if<B, T>::type;

    template <typename T>
    T && declval();

    /******************************************************************
     * Helpers
     */

    template <class T, T v>
    struct __spec_result
    {
      static const T value = v;
    };

    typedef __spec_result<bool, true>  enable_spec;
    typedef __spec_result<bool, false> disable_spec;

    template <bool v>
    using spec_result = __spec_result<bool, v>;
    typedef spec_result<true> enable_spec;
    typedef spec_result<false> disable_spec;

    template<class T, T v>
    struct integral_constant
    {
      static constexpr T value = v;
      using value_type = T;
      using type = integral_constant; // using injected-class-name
      constexpr operator value_type() const noexcept { return value; }
      constexpr value_type operator()() const noexcept { return value; } // since c++14
    };

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    namespace detail
    {
      template<class T>
      true_type test(int T::*);

      template<class>
      false_type test(...);
    }

    template<class T>
    struct is_class : decltype(detail::test<T>(nullptr)) {};

    namespace details
    {
      template<typename B>
      true_type test_ptr_conv(const volatile B*);

      template<typename>
      false_type test_ptr_conv(const volatile void*);

      template<typename, typename>
      auto test_is_base_of(...) -> true_type; // private or ambiguous base

      template<typename B, typename D>
      auto test_is_base_of(int) -> decltype(test_ptr_conv<B>(static_cast<D *>(nullptr)));
    }

    template <typename Base, typename Derived>
    struct is_base_of : integral_constant<
      bool,
      is_class<Base>::value &&
      is_class<Derived>::value &&
      decltype(details::test_is_base_of<Base, Derived>(0))::value
    > {};

    template <class Base, class Derived>
    inline constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;



    template <template <typename...> class base, typename derived>
    struct is_base_of_template_impl
    {
        template<typename... Ts>
        static constexpr true_type  test(const base<Ts...> *);
        static constexpr false_type test(...);
        using type = decltype(test(declval<derived*>()));
    };

    template < template <typename...> class base, typename derived>
    using is_base_of_template = typename is_base_of_template_impl<base, derived>::type;

    /******************************************************************
     * is_same<T, U> / is_same_v<T, U>
     */

    template<class T, class U>
    struct is_same: __spec_disabled;

    template<class T>
    struct is_same<T, T>: __spec_enabled;

    template <class T, class U>
    inline constexpr bool is_same_v = is_same<T, U>::value;

    /******************************************************************
     * is_int32<T> / is_int32_v<T>
     */

    TRAIT_DEF(is_int32, typename T);
    TRAIT_COND(is_int32, int32_t);
    TRAIT_HELPERS(is_int32);

    /******************************************************************
     * is_integer<T>
     */

    TRAIT_DEF(is_integer, typename T);
    TRAIT_COND(is_integer, int8_t);
    TRAIT_COND(is_integer, uint8_t);
    TRAIT_COND(is_integer, int16_t);
    TRAIT_COND(is_integer, uint16_t);
    TRAIT_COND(is_integer, int32_t);
    TRAIT_COND(is_integer, uint32_t);
    TRAIT_HELPERS(is_integer);

    /******************************************************************
     * is_signed<T>
     */

    // is_signed
    TRAIT_DEF(is_signed, typename T);
    TRAIT_COND(is_signed, int8_t);
    TRAIT_COND(is_signed, int16_t);
    TRAIT_COND(is_signed, int32_t);
    TRAIT_HELPERS(is_signed);

    // is_unsigned
    TRAIT_DEF(is_unsigned, typename T);
    TRAIT_COND(is_unsigned, uint8_t);
    TRAIT_COND(is_unsigned, uint16_t);
    TRAIT_COND(is_unsigned, uint32_t);
    TRAIT_HELPERS(is_unsigned);

    // is_number
    TRAIT_DEF(is_number, typename T);
    TRAIT_COND(is_number, int8_t);
    TRAIT_COND(is_number, uint8_t);
    TRAIT_COND(is_number, int16_t);
    TRAIT_COND(is_number, uint16_t);
    TRAIT_COND(is_number, int32_t);
    TRAIT_COND(is_number, uint32_t);
    TRAIT_COND(is_number, double);
    TRAIT_HELPERS(is_number);

    // is_any_number
    TRAIT_DEF(is_any_number, typename T);
    TRAIT_COND(is_any_number, int8_t);
    TRAIT_COND(is_any_number, uint8_t);
    TRAIT_COND(is_any_number, int16_t);
    TRAIT_COND(is_any_number, uint16_t);
    TRAIT_COND(is_any_number, int32_t);
    TRAIT_COND(is_any_number, uint32_t);
    TRAIT_COND(is_any_number, int64_t);
    TRAIT_COND(is_any_number, uint64_t);
    TRAIT_COND(is_any_number, double);
    TRAIT_HELPERS(is_any_number);

    // is_signed
    TRAIT_DEF(is_any_signed, typename T);
    TRAIT_COND(is_any_signed, int8_t);
    TRAIT_COND(is_any_signed, int16_t);
    TRAIT_COND(is_any_signed, int32_t);
    TRAIT_COND(is_any_signed, int64_t);
    TRAIT_HELPERS(is_any_signed);

    // is_unsigned
    TRAIT_DEF(is_any_unsigned, typename T);
    TRAIT_COND(is_any_unsigned, uint8_t);
    TRAIT_COND(is_any_unsigned, uint16_t);
    TRAIT_COND(is_any_unsigned, uint32_t);
    TRAIT_COND(is_any_unsigned, uint64_t);
    TRAIT_HELPERS(is_any_unsigned);

    // make_signed
    CONV_DEF(make_signed, if_is_any_number_t<T> Enable = true);
    CONV_TYPE(make_signed, int8_t, uint8_t, true);
    CONV_TYPE(make_signed, int16_t, uint16_t, true);
    CONV_TYPE(make_signed, int32_t, uint32_t, true);
    CONV_TYPE(make_signed, int64_t, uint64_t, true);
    CONV_HELPER(make_signed);

    // make_unsigned
    CONV_DEF(make_unsigned, if_is_any_number_t<T> Enable = true);
    CONV_TYPE(make_unsigned, uint8_t, int8_t, true);
    CONV_TYPE(make_unsigned, uint16_t, int16_t, true);
    CONV_TYPE(make_unsigned, uint32_t, int32_t, true);
    CONV_TYPE(make_unsigned, uint64_t, int64_t, true);
    CONV_HELPER(make_unsigned);

    template<typename T>
    struct is_format
    {
      static bool const value = false;
    };

    template<>
    struct is_format<OSCR::Serial::Style>
    {
      static bool const value = true;
    };

    template<>
    struct is_format<OSCR::Serial::Foreground>
    {
      static bool const value = true;
    };

    template<>
    struct is_format<OSCR::Serial::Background>
    {
      static bool const value = true;
    };

    template<typename T>
    struct is_printable
    {
      static bool const value = false;
    };
  }

  //
  // End of template specializations
  //! @endcond
}

#endif /* OSCR_SPECIALIZATIONS_H_ */
