#pragma once

#include "utility/diagnostics/console.h"

#define LANG_VERIFY(cond, mesg)                                     \
	do {                                                            \
		if (cond) { /* contextually convertible to bool paranoia */ \
		} else {                                                    \
			console::out << mesg << '\n';                           \
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

#define CONCATENATE(x, y) _CONCATENATE(x, y)
#define _CONCATENATE(x, y) x ## y

#define OUTCOME_TRY_1(__function)                                             \
    do {                                                                      \
		auto CONCATENATE(result, __LINE__) = __function;                      \
		if (CONCATENATE(result, __LINE__).has_error())                        \
		return outcome::failure((CONCATENATE(result, __LINE__)).get_error()); \
    }                                                                         \
	while(0)

#define OUTCOME_TRY_2(__success, __function)                                   \
     auto CONCATENATE(result, __LINE__) = __function;                          \
     if(CONCATENATE(result, __LINE__).has_error())                             \
         return outcome::failure((CONCATENATE(result, __LINE__)).get_error()); \
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