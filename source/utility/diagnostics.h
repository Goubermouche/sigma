#pragma once
#include "utility/types.h"

#include <format>
#include <list>

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

		static void printerr(const std::string& message) {
			std::cerr << message;
		}

		template<typename... arguments>
		static void printerr(std::format_string<arguments...> fmt, arguments&&... args) {
			std::cerr << format_str(std::move(fmt), std::forward<arguments>(args)...);
		}

		static void flush() {
			std::cout << std::flush;
		}

		static void errflush() {
			std::cerr << std::flush;
		}
	private:
		template<typename... arguments>
		static auto format_str(std::format_string<arguments...> fmt, arguments&&... args) -> std::string {
			return std::format(fmt, std::forward<arguments>(args)...);
		}
	};

	class error {
	public:
		error(const std::string& message) : m_message(message) {}

		auto get_message() const -> const std::string& {
			return m_message;
		}
	private:
		std::string m_message;
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
		requires (!std::is_same_v<std::remove_cvref_t<current>, std::in_place_t> && std::is_constructible_v<type, current>)
		constexpr explicit(!std::is_convertible_v<current, type>)
		result(current value) noexcept(std::is_nothrow_constructible_v<type, current>)
			: m_value(std::move(type(std::move(value)))) {}

		result(error failure) noexcept : m_value(std::move(failure)) {}

		bool has_error() const {
			return std::holds_alternative<error>(m_value);
		}

		bool has_value() const {
			return !has_error();
		}

		auto get_value() const -> const type& {
			return std::get<type>(m_value);
		}

		auto get_value() -> type {
			return std::move(std::get<type>(m_value));
		}

		auto get_error() const -> const error& {
			return std::get<error>(m_value);
		}
	private:
		std::variant<type, error> m_value;
	};

	template<>
	class result<void> {
	public:
		result() noexcept = default;
		result(error failure) noexcept : m_error_message(failure.get_message()) {}

		bool has_error() const {
			return !m_error_message.empty();
		}

		bool has_value() const {
			return !has_error();
		}

		auto get_error() const -> error {
			return error(m_error_message);
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
    return utility::error((CONCATENATE(result, __LINE__)).get_error());         \
	}                                                                             \
} while(false)

#define TRY_2(__success, __result)                                            \
auto CONCATENATE(result, __LINE__) = (__result);                              \
if(CONCATENATE(result, __LINE__).has_error()) {                               \
  return utility::error((CONCATENATE(result, __LINE__)).get_error());         \
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
