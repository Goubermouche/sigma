#pragma once
#include "utility/types.h"

#include <format>
#include <list>

#include "macros.h"

namespace utility {
	class console {
	public:
		static void print(const std::string& message) {
			std::cout << message;
		}

		template<typename... arguments>
		static void print(std::format_string<arguments...> fmt, arguments&&... args) {
			std::cout << format_str(std::move(fmt), std::forward<arguments>(args)...);
		}

		static void println(const std::string& message) {
			std::cout << message << '\n';
		}

		template<typename... arguments>
		static void println(std::format_string<arguments...> fmt, arguments&&... args) {
			std::cout << format_str(std::move(fmt), std::forward<arguments>(args)...) << '\n';
		}

		static void println() {
			std::cout << '\n';
		}

		static void flush() {
			std::cout << std::flush;
		}
	private:
		template<typename... arguments>
		static auto format_str(std::format_string<arguments...> fmt, arguments&&... args) -> std::string {
			return std::format(fmt, std::forward<arguments>(args)...);
		}
	};

	class error {
	public:
		enum class code {
			// filesystem
			EXPECTED_FILE,
			FILE_DOES_NOT_EXIST,
			INVALID_FILE_EXTENSION,
			CANNOT_READ_FILE,

			// tokenizer
			INVALID_STRING_TERMINATOR,
			NUMERICAL_LITERAL_FP_WITHOUT_DOT,
			NUMERICAL_LITERAL_UNSIGNED_WITH_DOT,
			NUMERICAL_LITERAL_MORE_THAN_ONE_DOT,

			// parser
			INVALID_TYPE_TOKEN,
			UNEXPECTED_NON_NUMERICAL,
			UNEXPECTED_TOKEN,
			UNEXPECTED_TOKEN_WITH_EXPECTED,

			// type checker
			UNKNOWN_FUNCTION,
			INVALID_FUNCTION_PARAMETER_COUNT,
			VOID_RETURN,
			UNEXPECTED_TYPE,
			UNKNOWN_VARIABLE,
		};

		error(const std::string& message) : m_message(message) {}

		template<typename... arguments>
		static auto create(code code, arguments&&... args) -> error {
			return error(std::vformat(m_errors.find(code)->second, std::make_format_args(args...)));
		}

		auto get_message() const -> const std::string& {
			return m_message;
		}
	private:
		std::string m_message;

		const static inline std::unordered_map<code, std::string> m_errors = {
			// filesystem
			{ code::EXPECTED_FILE, "expected a file but got a directory ('{}')" },
			{ code::FILE_DOES_NOT_EXIST, "specified file does not exist ('{}')" },
			{ code::INVALID_FILE_EXTENSION, "specified file has an invalid file extension ('{}' - expected '{}')" },
			{ code::CANNOT_READ_FILE, "unable to read the specified file ('{}')" },

			// tokenizer
			{ code::INVALID_STRING_TERMINATOR, "invalid string literal terminator detected" },
			{ code::NUMERICAL_LITERAL_FP_WITHOUT_DOT, "numerical floating-point literal without '.' character detected" },
			{ code::NUMERICAL_LITERAL_UNSIGNED_WITH_DOT, "unsigned numerical literal with '.' character detected" },
			{ code::NUMERICAL_LITERAL_MORE_THAN_ONE_DOT, "numerical literal with more than one '.' character detected" },

			// parser
			{ code::INVALID_TYPE_TOKEN, "invalid type token received ('{}')" },
			{ code::UNEXPECTED_NON_NUMERICAL, "unexpected non-numerical literal token received ('{}')" },
			{ code::UNEXPECTED_TOKEN, "unexpected token received ('{}')" },
			{ code::UNEXPECTED_TOKEN_WITH_EXPECTED, "unexpected token received (expected '{}', but got '{}' instead)" },

			// type checker
			{ code::UNKNOWN_FUNCTION, "attempting to call an unknown function ('{}')" },
			{ code::INVALID_FUNCTION_PARAMETER_COUNT, "attempting to call a function with an invalid number of parameters (expected '{}', but got '{}' instead)" },
			{ code::VOID_RETURN, "invalid void return statement (expected 'ret;', but got 'ret {};' instead)" },
			{ code::UNEXPECTED_TYPE, "encountered an unexpected type (expected '{}', but got '{}' instead" },
			{ code::UNKNOWN_VARIABLE, "unknown variable '{}' referenced" },
		};
	};

	/**
	 * \brief Rudimentary replacement for std::expected. Either contains an error (string message)
	 * or a value of type \b type. \b The \b contained \b value \b is \b move \b constructed! For
	 * more info see the \b PRINT_ERROR, \b TRY, and \b SUCCESS macros.
	 * \tparam type Type of the contained value
	 */
	template<typename type>
	class result {
	public:
		// NOTE: prefer move constructing over passing const references, since the value is a result of
		//       various functions we can just take the value and move it out, instead of having to
		//       copy construct it again

		template <class current = type>
		requires (!std::is_same_v<std::remove_cvref_t<current>, std::in_place_t> && !std::is_same_v<std::remove_cv_t<type>, bool>&& std::is_constructible_v<type, current>)
		constexpr explicit(!std::is_convertible_v<current, type>)
		result(current value) noexcept(std::is_nothrow_constructible_v<type, current>) : m_success(std::move(value)) {}

		result(const error& failure) noexcept : m_error_message(failure.get_message()) {}
		result() {}

		bool has_error() const {
			return !m_error_message.empty();
		}

		bool has_value() const {
			return !has_error();
		}

		auto get_value() const -> const type& {
			return m_success;
		}

		auto get_value() -> type {
			return  std::move(m_success);
		}

		auto get_error_message() const -> const std::string& {
			return m_error_message;
		}
	private:
		type m_success;
		std::string m_error_message;
	};

	template<>
	class result<void> {
	public:
		result() noexcept = default;
		result(const error& failure) noexcept : m_error_message(failure.get_message()) {}

		bool has_error() const {
			return !m_error_message.empty();
		}

		bool has_value() const {
			return !has_error();
		}

		auto get_error_message() const -> const std::string& {
			return m_error_message;
		}
	private:
		std::string m_error_message;
	};
} // namespace utility

#define CONCATENATE(x, y) _CONCATENATE(x, y)
#define _CONCATENATE(x, y) x ## y

/**
 * \brief Accepts a utility::result and prints the contained error, if there is one.
 * \param __result utility::result to print
 */
#define PRINT_ERROR(__result)                                                       \
	do {                                                                              \
		auto CONCATENATE(result, __LINE__) = __result;                                  \
		if (CONCATENATE(result, __LINE__).has_error()) {                                \
			utility::console::println(CONCATENATE(result, __LINE__).get_error_message()); \
		}                                                                               \
} while (false)

#define TRY_1(__result)                                                         \
do {                                                                            \
	auto CONCATENATE(result, __LINE__) = __result;                                \
	if (CONCATENATE(result, __LINE__).has_error()) {                              \
    return utility::error((CONCATENATE(result, __LINE__)).get_error_message()); \
	}                                                                             \
} while(false)

#define TRY_2(__success, __result)                                            \
auto CONCATENATE(result, __LINE__) = (__result);                              \
if(CONCATENATE(result, __LINE__).has_error()) {                               \
  return utility::error((CONCATENATE(result, __LINE__)).get_error_message()); \
}                                                                             \
__success = CONCATENATE(result, __LINE__).get_value()

#define EXPAND(x) x
#define GET_MACRO(_1, _2, NAME, ...) NAME

/**
 * \brief Rudimentary try macro, accepts a utility::result as the first argument. Immediately returns
 * if the result contains an error and propagates the error further. Two variants: the first variant
 * only returns, whilst the second one handles the 'value' contained in the result and stores it.
 */
#define TRY(...) EXPAND(GET_MACRO(__VA_ARGS__, TRY_2, TRY_1)(__VA_ARGS__))

/**
 * \brief Used as a more streamlined way of handling success states of utility::result<void> (instead
 * of having to write stuff like utility::result<void>() or {} we can just use this to hopefully
 * reduce confusion related to this already relatively confusing system.
 */
#define SUCCESS {}
