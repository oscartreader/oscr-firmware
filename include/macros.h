/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#ifndef OSCR_MACROS_H_
# define OSCR_MACROS_H_

#pragma region Helpers

# if !defined(DEC)
#   define DEC 10
# endif /* DEC */

# if !defined(HEX)
#   define HEX 16
# endif /* HEX */

# if !defined(OCT)
#   define OCT 8
# endif /* OCT */

# if !defined(BIN)
#   define BIN 2
# endif /* BIN */

# if !defined(STR)
#   define STR(x) #x
# endif

# if !defined(NUM2STR)
#   define NUM2STR(x) STR(x)
# endif

# if !defined(XSTR)
#define XSTR(x) STR(x)
# endif

# if !defined(MSG)
#define MSG(x) _Pragma (STR(message (x)))
# endif

# if !defined(DISPLAY_GCC_VERSION)
#   define DISPLAY_GCC_VERSION MSG("GCC version: " XSTR(__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__))
# endif

/**
 * @def __constinit
 * @brief Expands to `constinit` if supported, or nothing if not.
 *
 * The intention is to flag candidates for `constinit` to help with
 * evaluating any possible improvements C++20 could make.
 *
 * @see https://en.cppreference.com/w/cpp/language/constinit.html
 */


/**
 * Feature checks.
 */
#if defined(__has_cpp_attribute)

  // constexpr (C++)
# if defined(__cpp_constexpr)
    // relaxed constexpr (C++14)
#   if __cpp_constexpr < 201304L
#     define COMPILER_OUTDATED
#   else
#     define __relaxed_constexpr constexpr
#   endif
# else
#   define COMPILER_TOO_OLD
# endif

  // template non-type parameter are allowed to be `auto` (C++17)
# if defined(__cpp_nontype_template_parameter_auto)
#   define __TEMPLATE_AUTO_UINT8 auto
# else
#   define COMPILER_DEPRECATED
# endif

# if defined(__cpp_if_constexpr)
#   define __if_constexpr constexpr
#else
#   define COMPILER_DEPRECATED
# endif

  // variadic template arguments (C++11)
# if !defined(__cpp_variadic_templates)
#   define COMPILER_DEPRECATED
# endif

  // constinit (C++20)
# if defined(__cpp_constinit)
#   define __constinit constinit
#   define __constinitexpr constinit
# endif

  // consteval (C++20)
# if defined(__cpp_consteval)
#   define __CONSTEVAL consteval
# endif

#endif /* __has_cpp_attribute */

// "Safe" defaults are defined here in case `__has_cpp_attribute` wasn't defined

#if !defined(__if_constexpr)
#   define __if_constexpr
#endif

// uint8_t when `auto` is unsupported
#if !defined(__TEMPLATE_AUTO_UINT8)
# define __TEMPLATE_AUTO_UINT8 uint8_t
#endif

// Empty when relaxed `constexpr`s are unsupported
#if !defined(__relaxed_constexpr)
# define __relaxed_constexpr
#endif

// Empty when `constinit` is unsupported
#if !defined(__constinit)
# define __constinit
#endif

// `constexpr` when `consteval` is unsupported
#if !defined(__CONSTEVAL)
# define __CONSTEVAL constexpr
#endif

#if !defined(__constinitexpr)
/**
 * @def __constinitexpr
 * @brief Expands to `constinit` if supported, otherwise `constexpr`.
 *
 * This macro should be used where `constexpr` would currently be
 * valid but `constinit` would be preferred due to a possible future
 * need for mutability.
 *
 * The intention is to flag candidates for `constinit` to help with
 * evaluating any possible improvements C++20 could make.
 *
 * @see https://en.cppreference.com/w/cpp/language/constinit.html
 */
# define __constinitexpr constexpr
#endif

// hot
#if !defined(__hot)
# define __hot __attribute__((hot))
#endif

#if defined(COMPILER_TOO_OLD) || defined(COMPILER_OUTDATED)
# pragma message "Outdated compiler or dialect detected!"
# pragma message "The recommended minimum dialect is gnu++17 (or gnu++1z)."
# pragma message "See: https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html"
#endif

#if defined(COMPILER_TOO_OLD)
# error "Your compiler is too old or you are compiling with the wrong C++ dialect."
#endif

#if defined(COMPILER_DEPRECATED) && !defined(COMPILER_TOO_OLD) && !defined(COMPILER_OUTDATED)
# pragma message "Deprecated compiler or dialect detected!"
# pragma message "Your compiler will stop working in the future. The minimum dialect is gnu++17 (or gnu++1z)."
# pragma message "See: https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html"
#endif

#if defined(ARDUINO_ARCH_AVR)

# define LitStr(s) F(s)
# define LitStr_P(s) PSTR(s)

# define __AVR__ALWAYS_INLINE__ __attribute__((alwaysinline))
# define __AVR__NEVER_INLINE__ __attribute__((noinline))

// ARDUINO_AVR_MEGA2560
// ARDUINO_AVR_UNO

#else

# define LitStr(s) s
# define LitStr_P(s) s

# define __AVR__NEVER_INLINE__

#define PINA  fakePin
#define PORTA fakePort
#define DDRA  fakeDDR

#define PINB  fakePin
#define PORTB fakePort
#define DDRB  fakeDDR

#define PINC  fakePin
#define PORTC fakePort
#define DDRC  fakeDDR

#define PIND  fakePin
#define PORTD fakePort
#define DDRD  fakeDDR

#define PINE  fakePin
#define PORTE fakePort
#define DDRE  fakeDDR

#define PINF  fakePin
#define PORTF fakePort
#define DDRF  fakeDDR

#define PING  fakePin
#define PORTG fakePort
#define DDRG  fakeDDR

#define PINH  fakePin
#define PORTH fakePort
#define DDRH  fakeDDR

#define PINJ  fakePin
#define PORTJ fakePort
#define DDRJ  fakeDDR

#define PINK  fakePin
#define PORTK fakePort
#define DDRK  fakeDDR

#define PINL  fakePin
#define PORTL fakePort
#define DDRL  fakeDDR

#endif

#define STRINGIFY(x) #x
#define STRINGY2(x) STRINGIFY(x)

#define IF_EN3(X, Y) (STRINGIFY(X) == STRINGIFY(Y))
#define IF_EN2(X, Y) (STRINGIFY(X) == STRINGIFY(Y))
#define IF_EN(X, Y) (X==Y)

#if !defined(CONCAT)
# define CONCAT(X, Y) X ## Y
#endif

#define GET_HW_VER(X, Y) ((Y == 0) ? X Custom : CONCAT(X V, Y))
#define OSCR_HARDWARE_VERSION GET_HW_VER(HW::, HW_VERSION)
#define OSCR_FEATURE_FLAG(EN, FEAT) (EN ? (1 << static_cast<uint32_t>(FEAT)) : 0)

#define _GET_MACRO_0_1(_0, _1, NAME, ...) NAME
#define _GET_MACRO_0_2(_0, _1, _2, NAME, ...) NAME
#define _GET_MACRO_0_3(_0, _1, _2, _3, NAME, ...) NAME
#define _GET_MACRO_0_4(_0, _1, _2, _3, _4, NAME, ...) NAME
#define _GET_MACRO_0_5(_0, _1, _2, _3, _4, _5, NAME, ...) NAME

#define GET_MACRO_0_1(MNAME, ...) _GET_MACRO_0_1(_0, ##__VA_ARGS__, MNAME ## 1, MNAME ## 0)(__VA_ARGS__)
#define GET_MACRO_0_2(MNAME, ...) _GET_MACRO_0_2(_0, ##__VA_ARGS__, MNAME ## 2, MNAME ## 1, MNAME ## 0)(__VA_ARGS__)
#define GET_MACRO_0_3(MNAME, ...) _GET_MACRO_0_3(_0, ##__VA_ARGS__, MNAME ## 3, MNAME ## 2, MNAME ## 1, MNAME ## 0)(__VA_ARGS__)
#define GET_MACRO_0_4(MNAME, ...) _GET_MACRO_0_4(_0, ##__VA_ARGS__, MNAME ## 4, MNAME ## 3, MNAME ## 2, MNAME ## 1, MNAME ## 0)(__VA_ARGS__)
#define GET_MACRO_0_5(MNAME, ...) _GET_MACRO_0_5(_0, ##__VA_ARGS__, MNAME ## 5, MNAME ## 4, MNAME ## 3, MNAME ## 2, MNAME ## 1, MNAME ## 0)(__VA_ARGS__)

#define GET_MACRO_1_2(MNAME, ...) _GET_MACRO_0_1(##__VA_ARGS__, MNAME ## 2, MNAME ## 1)(__VA_ARGS__)
#define GET_MACRO_1_3(MNAME, ...) _GET_MACRO_0_2(##__VA_ARGS__, MNAME ## 3, MNAME ## 2, MNAME ## 1)(__VA_ARGS__)
#define GET_MACRO_1_4(MNAME, ...) _GET_MACRO_0_3(##__VA_ARGS__, MNAME ## 4, MNAME ## 3, MNAME ## 2, MNAME ## 1)(__VA_ARGS__)
#define GET_MACRO_1_5(MNAME, ...) _GET_MACRO_0_4(##__VA_ARGS__, MNAME ## 5, MNAME ## 4, MNAME ## 3, MNAME ## 2, MNAME ## 1)(__VA_ARGS__)

#define GET_MACRO_2_3(MNAME, ...) _GET_MACRO_0_2(##__VA_ARGS__, MNAME ## 3, MNAME ## 2)(__VA_ARGS__)
#define GET_MACRO_2_4(MNAME, ...) _GET_MACRO_0_3(##__VA_ARGS__, MNAME ## 4, MNAME ## 3, MNAME ## 2)(__VA_ARGS__)
#define GET_MACRO_2_5(MNAME, ...) _GET_MACRO_0_4(##__VA_ARGS__, MNAME ## 5, MNAME ## 4, MNAME ## 3, MNAME ## 2)(__VA_ARGS__)

#define GET_MACRO_3_4(MNAME, ...) _GET_MACRO_0_3(##__VA_ARGS__, MNAME ## 4, MNAME ## 3)(__VA_ARGS__)
#define GET_MACRO_3_5(MNAME, ...) _GET_MACRO_0_4(##__VA_ARGS__, MNAME ## 5, MNAME ## 4, MNAME ## 3)(__VA_ARGS__)

#define GET_MACRO_4_5(MNAME, ...) _GET_MACRO_0_4(##__VA_ARGS__, MNAME ## 5, MNAME ## 4)(__VA_ARGS__)

#define _ISEMPTY(x) (((x ## 0) == 0) && ((1 ## x) == 1))
#define ISEMPTY(x) _ISEMPTY(x)

#define assertIsNumber(x, msg)      static_assert(OSCR::Util::is_integer<decltype(x)>::value, msg);

/**
 * Defines a type trait
 */
#define TRAIT_DEF(NAME, ...)        template<__VA_ARGS__> struct NAME             : __spec_disabled

/**
 * Enables a type trait for the specified condition
 */
#define TRAIT_COND(NAME, ...)       template<           > struct NAME<__VA_ARGS__>: __spec_enabled

/**
 * Creates a `NAME_v` helper for a type trait.
 */
#define TRAIT_HELPER_VAL(NAME)      template <class T> inline constexpr bool NAME ## _v = NAME<T>::value
#define TRAIT_HELPER_TYPE(NAME)     template <class T> using if_ ## NAME ## _t = enable_if_t<NAME<T>::value, bool>
#define TRAIT_HELPERS(NAME)         TRAIT_HELPER_VAL(NAME); TRAIT_HELPER_TYPE(NAME)

#define CONV_DEF(NAME, ...)         template <class T, __VA_ARGS__> struct NAME { typedef T type; }
#define CONV_TYPE(NAME, TYPE, ...)  template <> struct NAME <__VA_ARGS__> { typedef TYPE type; }
#define CONV_HELPER(NAME)           template <class T> using NAME ## _t   = typename NAME<T>::type

#define sizeofarray(a) sizeof(a)/sizeof(a[0])
#define BUFFN(buff) buff, (size_t)sizeof(buff)

#define BANNEDF(FUNC) sorry_##FUNC##_is_a_banned_function
#define BANNEDV(VAR) sorry_##VAR##_is_a_banned_variable

/**
 * Arduino Mbed uses `CRC` as a system macro. Currently, it is suggested Use `CRCSum` instead.
 */
#if !defined(CRC)
  #define CRC BANNEDV(CRC);
#endif

/**
 * Arduino Mbed uses `RTC` as a system macro.
 */
#if !defined(RTC)
  #define RTC BANNEDV(RTC);
#endif

#include "arch/macros.h"

#endif /* OSCR_MACROS_H_ */
