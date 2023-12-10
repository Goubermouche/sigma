#pragma once

#include "diagnostics/console.h"

#ifdef _WIN32
    #include <intrin.h>
    #define DEBUG_BREAK() __debugbreak()
#elif __linux__
    #include <signal.h>
    #define DEBUG_BREAK() raise(SIGTRAP)
#else
    #error "Unsupported platform!"
#endif

#define LANG_VERIFY(cond, msg)                    \
    do {                                          \
        if (!(cond)) {                            \
            utility::console::out << msg << '\n'; \
			DEBUG_BREAK();                        \
		}                                         \
    } while (false)

/**
 * \brief Basic assertion macro, asserts whenever \a cond evaluates to false and prints \a message.
 * \param cond Condition to evaluate
 * \param mesg Assertion notification message
 */
#define ASSERT(cond, mesg) LANG_VERIFY(cond, mesg)

#define CONCATENATE(x, y) _CONCATENATE(x, y)
#define _CONCATENATE(x, y) x ## y

#define OUTCOME_TRY_1(__function)                                                      \
    do {                                                                               \
		auto CONCATENATE(result, __LINE__) = __function;                               \
		if (CONCATENATE(result, __LINE__).has_error())                                 \
		return utility::outcome::failure((CONCATENATE(result, __LINE__)).get_error()); \
    }                                                                                  \
	while(0)

#define OUTCOME_TRY_2(__success, __function)                                            \
     auto CONCATENATE(result, __LINE__) = __function;                                   \
     if(CONCATENATE(result, __LINE__).has_error())                                      \
         return utility::outcome::failure((CONCATENATE(result, __LINE__)).get_error()); \
     __success = CONCATENATE(result, __LINE__).get_value()

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
    typedef typename enum_flag_integer_for_size<sizeof(type)>::type type;
};

// source: <winnt.h>

/**
 * \brief Declares the given \a __enum to have various binary operands useful
 * with flag enums.
 * \param __enum Enum to declare as a flag enum
 */
#define FLAG_ENUM(__enum) \
inline constexpr __enum operator | (__enum a, __enum b) noexcept { return __enum(((enum_flag_sized_integer<__enum>::type)a) | ((enum_flag_sized_integer<__enum>::type)b)); } \
inline __enum &operator |= (__enum &a, __enum b) noexcept { return (__enum &)(((enum_flag_sized_integer<__enum>::type &)a) |= ((enum_flag_sized_integer<__enum>::type)b)); } \
inline constexpr __enum operator & (__enum a, __enum b) noexcept { return __enum(((enum_flag_sized_integer<__enum>::type)a) & ((enum_flag_sized_integer<__enum>::type)b)); } \
inline __enum &operator &= (__enum &a, __enum b) noexcept { return (__enum &)(((enum_flag_sized_integer<__enum>::type &)a) &= ((enum_flag_sized_integer<__enum>::type)b)); } \
inline constexpr __enum operator ~ (__enum a) noexcept { return __enum(~((enum_flag_sized_integer<__enum>::type)a)); }                                                       \
inline constexpr __enum operator ^ (__enum a, __enum b) noexcept { return __enum(((enum_flag_sized_integer<__enum>::type)a) ^ ((enum_flag_sized_integer<__enum>::type)b)); } \
inline __enum &operator ^= (__enum &a, __enum b) noexcept { return (__enum &)(((enum_flag_sized_integer<__enum>::type &)a) ^= ((enum_flag_sized_integer<__enum>::type)b)); } \

#if defined(_MSC_VER)
#define aligned_malloc _aligned_malloc
#define aligned_free _aligned_free
#else
#define aligned_malloc std::aligned_alloc
#define aligned_free std::free
#endif