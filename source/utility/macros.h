#pragma once

#include "utility/diagnostics.h"

#ifdef _WIN32
  #include <intrin.h>
  #define DEBUG_BREAK() __debugbreak()
#elif __linux__
  #include <signal.h>
  #define DEBUG_BREAK() raise(SIGTRAP)
#else
  #error "Unsupported platform!"
#endif

#ifdef DEBUG
#define ASSERT(__condition, __fmt, ...)                                           \
	do {                                                                            \
		if(!(__condition)) {                                                          \
			utility::console::print("ASSERTION FAILED: ({}:{}): ", __FILE__, __LINE__); \
			utility::console::println((__fmt), ##__VA_ARGS__);                          \
			utility::console::flush();                                                  \
			DEBUG_BREAK();                                                              \
		}                                                                             \
	} while(false)

#define NOT_IMPLEMENTED()                                                                        \
	do {                                                                                           \
		utility::console::println("ASSERTION FAILED: ({}:{}): NOT IMPLEMENTED", __FILE__, __LINE__); \
		utility::console::flush();                                                                   \
		DEBUG_BREAK();                                                                               \
	} while(false)

#define PANIC( __fmt, ...)                                                      \
	do {                                                                          \
		utility::console::print("ASSERTION FAILED: ({}:{}): ", __FILE__, __LINE__); \
		utility::console::println((__fmt), ##__VA_ARGS__);                          \
		utility::console::flush();                                                  \
		DEBUG_BREAK();                                                              \
	} while(false)
#else
#define ASSERT(__condition, __fmt, ...)
#define NOT_IMPLEMENTED()
#define PANIC(__fmt, ...)
#endif

#define CONCATENATE(x, y) _CONCATENATE(x, y)
#define _CONCATENATE(x, y) x ## y

#define EXPAND(x) x
#define GET_MACRO(_1, _2, NAME, ...) NAME

/**
 * \brief Attempts to call the given \a __function, if the outcome of the
 * function call is erroneous immediately returns from the parent function/method.
 * \param __success Variable declaration used for storing the successful result
 * \param __function Function to execute
 */
#define OUTCOME_TRY(...) EXPAND(GET_MACRO(__VA_ARGS__, OUTCOME_TRY_2, OUTCOME_TRY_1)(__VA_ARGS__))

#define SUPPRESS_C4100(_statement) (void)_statement

#define LANG_FILE_EXTENSION ".s"


template <size_t size>
struct enum_flag_integer_for_size;

template <>
struct enum_flag_integer_for_size<1> {
  typedef utility::i8 type;
};

template <>
struct enum_flag_integer_for_size<2> {
  typedef utility::i16 type;
};

template <>
struct enum_flag_integer_for_size<4> {
  typedef utility::i32 type;
};

template <>
struct enum_flag_integer_for_size<8> {
  typedef utility::i64 type;
};

// used as an approximation of std::underlying_type<T>
template <class type>
struct enum_flag_sized_integer {
  typedef typename enum_flag_integer_for_size<sizeof(type)>::type size_type;
};

// source: <winnt.h>

/**
 * \brief Declares the given \a __enum to have various binary operands useful
 * with flag enums.
 * \param __enum Enum to declare as a flag enum
 */
#define FLAG_ENUM(__enum) \
inline constexpr __enum operator | (__enum a, __enum b) noexcept { return __enum(((enum_flag_sized_integer<__enum>::size_type)a) | ((enum_flag_sized_integer<__enum>::size_type)b)); } \
inline __enum &operator |= (__enum &a, __enum b) noexcept { return (__enum &)(((enum_flag_sized_integer<__enum>::size_type &)a) |= ((enum_flag_sized_integer<__enum>::size_type)b)); } \
inline constexpr __enum operator & (__enum a, __enum b) noexcept { return __enum(((enum_flag_sized_integer<__enum>::size_type)a) & ((enum_flag_sized_integer<__enum>::size_type)b)); } \
inline __enum &operator &= (__enum &a, __enum b) noexcept { return (__enum &)(((enum_flag_sized_integer<__enum>::size_type &)a) &= ((enum_flag_sized_integer<__enum>::size_type)b)); } \
inline constexpr __enum operator ~ (__enum a) noexcept { return __enum(~((enum_flag_sized_integer<__enum>::size_type)a)); }                                                       \
inline constexpr __enum operator ^ (__enum a, __enum b) noexcept { return __enum(((enum_flag_sized_integer<__enum>::size_type)a) ^ ((enum_flag_sized_integer<__enum>::size_type)b)); } \
inline __enum &operator ^= (__enum &a, __enum b) noexcept { return (__enum &)(((enum_flag_sized_integer<__enum>::size_type &)a) ^= ((enum_flag_sized_integer<__enum>::size_type)b)); } \

#if defined(_MSC_VER)
#define aligned_malloc _aligned_malloc
#define aligned_free _aligned_free
#else
#define aligned_malloc std::aligned_alloc
#define aligned_free std::free
#endif