#pragma once

#include "utility/console.h"

//#define LANG_REPORT_BASE(...) (void) ((1 != _CrtDbgReport(__VA_ARGS__)) || (__debugbreak(), 0))
//#define LANG_REPORT_F0(rptno, msg) LANG_REPORT_BASE(rptno, __FILE__, __LINE__, NULL, "%s", msg)
//
//#define LANG__CRT_SECURE_INVALID_PARAMETER(expr) ::_invalid_parameter(_CRT_WIDE(#expr), L"", __FILEW__, __LINE__, 0)
//
//#define LANG_REPORT_ERROR(mesg)                   \
//do {                                              \
//        LANG_REPORT_F0(_CRT_ASSERT, mesg);        \
//        LANG__CRT_SECURE_INVALID_PARAMETER(mesg); \
//} while (false)

#define LANG_VERIFY(cond, mesg)                                     \
    do {                                                            \
        if (cond) { /* contextually convertible to bool paranoia */ \
        } else {                                                    \
            std::cout << mesg << '\n';                              \
            __debugbreak();                                         \
        }                                                           \
    } while (false)

/**
 * \brief Basic assertion macro, asserts whenever \a cond evaluates to false and prints \a message.
 * \param cond Condition to evaluate
 * \param mesg Assertion notification message
 */
#define ASSERT(cond, mesg) LANG_VERIFY(cond, mesg)

// #define ENABLE_NODE_NAME_LOGS
#ifdef ENABLE_NODE_NAME_LOGS
#define LOG_NODE_NAME(node) std::cout << #node << '\n'
#else
#define LOG_NODE_NAME(node)
#endif