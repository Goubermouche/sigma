#pragma once
#include "diagnostics.h"

#ifdef _WIN32
#include <intrin.h>

#define DEBUG_BREAK() __debugbreak()
#define SYSTEM_WINDOWS
#elif __linux__
#include <signal.h>

#define DEBUG_BREAK() raise(SIGTRAP)
#define SYSTEM_LINUX
#else
#error "Unsupported platform!"
#endif

#ifdef DEBUG
#define ASSERT(__condition, __fmt, ...)                                              \
  do {                                                                               \
    if(!(__condition)) {                                                             \
      utility::console::printerr("ASSERTION FAILED: ({}:{}): ", __FILE__, __LINE__); \
      utility::console::printerr((__fmt), ##__VA_ARGS__);                            \
      utility::console::printerr("\n");                                              \
      utility::console::flush();                                                     \
      DEBUG_BREAK();                                                                 \
    }                                                                                \
  } while(false)

#define NOT_IMPLEMENTED()                                                                           \
  do {                                                                                              \
    utility::console::printerr("ASSERTION FAILED: ({}:{}): NOT IMPLEMENTED\n", __FILE__, __LINE__); \
    utility::console::errflush();                                                                   \
    DEBUG_BREAK();                                                                                  \
  } while(false)

#define PANIC( __fmt, ...)                                                         \
  do {                                                                             \
    utility::console::printerr("ASSERTION FAILED: ({}:{}): ", __FILE__, __LINE__); \
    utility::console::printerr((__fmt), ##__VA_ARGS__);                            \
    utility::console::printerr("\n");                                              \
    utility::console::errflush();                                                  \
    DEBUG_BREAK();                                                                 \
  } while(false)
#else
#define ASSERT(__condition, __fmt, ...)
#define NOT_IMPLEMENTED()
#define PANIC(__fmt, ...)
#endif

#define SUPPRESS_C4100(__statement) (void)__statement

/**
 * \brief Declares the given \a __enum to have various binary operands useful
 * with flag enums.
 * \param __enum Enum to declare as a flag enum
 */
#define FLAG_ENUM(__enum) \
inline constexpr __enum operator | (__enum a, __enum b) noexcept { return __enum(((std::underlying_type_t<__enum>)a) | ((std::underlying_type_t<__enum>)b)); } \
inline __enum &operator |= (__enum &a, __enum b) noexcept { return (__enum &)(((std::underlying_type_t<__enum> &)a) |= ((std::underlying_type_t<__enum>)b)); } \
inline constexpr __enum operator & (__enum a, __enum b) noexcept { return __enum(((std::underlying_type_t<__enum>)a) & ((std::underlying_type_t<__enum>)b)); } \
inline __enum &operator &= (__enum &a, __enum b) noexcept { return (__enum &)(((std::underlying_type_t<__enum> &)a) &= ((std::underlying_type_t<__enum>)b)); } \
inline constexpr __enum operator ~ (__enum a) noexcept { return __enum(~((std::underlying_type_t<__enum>)a)); }                                                \
inline constexpr __enum operator ^ (__enum a, __enum b) noexcept { return __enum(((std::underlying_type_t<__enum>)a) ^ ((std::underlying_type_t<__enum>)b)); } \
inline __enum &operator ^= (__enum &a, __enum b) noexcept { return (__enum &)(((std::underlying_type_t<__enum> &)a) ^= ((std::underlying_type_t<__enum>)b)); } \

#define INTEGRAL_ENUM(__enum) \
template<typename other_enum_type> \
inline constexpr std::underlying_type_t<__enum> operator | (__enum enum_a, other_enum_type enum_b) { return (std::underlying_type_t<__enum>)enum_a | (std::underlying_type_t<other_enum_type>)enum_b; } \
inline constexpr std::underlying_type_t<__enum> operator | (__enum e, std::underlying_type_t<__enum> value) { return (std::underlying_type_t<__enum>)e | value; } \
inline constexpr std::underlying_type_t<__enum> operator | (std::underlying_type_t<__enum> value, __enum e) { return value | (std::underlying_type_t<__enum>)e; } \
inline constexpr std::underlying_type_t<__enum> operator & (__enum e, std::underlying_type_t<__enum> value) { return (std::underlying_type_t<__enum>)e & value; } \
inline constexpr std::underlying_type_t<__enum> operator & (std::underlying_type_t<__enum> value, __enum e) { return value & (std::underlying_type_t<__enum>)e; } \
inline constexpr std::underlying_type_t<__enum> operator << (std::underlying_type_t<__enum> value, __enum e) { return value << (std::underlying_type_t<__enum>)e; } \
inline constexpr std::underlying_type_t<__enum> operator + (__enum e, std::underlying_type_t<__enum> value) { return (std::underlying_type_t<__enum>)e + value; } \
inline constexpr std::underlying_type_t<__enum> operator + (std::underlying_type_t<__enum> value, __enum e) { return value + (std::underlying_type_t<__enum>)e; } \
inline constexpr std::underlying_type_t<__enum> operator ^ (__enum e, std::underlying_type_t<__enum> value) { return (std::underlying_type_t<__enum>)e ^ value; } \
inline constexpr std::underlying_type_t<__enum> operator ^ (std::underlying_type_t<__enum> value, __enum e) { return value ^ (std::underlying_type_t<__enum>)e; } \
inline constexpr bool operator == (std::underlying_type_t<__enum> value, __enum e) { return value == (std::underlying_type_t<__enum>)e; } \
