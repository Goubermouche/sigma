#pragma once
#include "../types.h"

namespace utility {
	/**
	 * \brief Creates a backslash-escaped version of the \a input string.
	 * \param input String to escape.
	 * \return Escaped version of the given string.
	 */
	inline auto escape_string(const std::string& input) -> std::string {
		// TODO: update to support various hexadecimal and binary strings
		std::string output;

		for (const char ch : input) {
			if (ch == '\\' || ch == '\'' || ch == '\"' || ch == '\a' || ch == '\b' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' || ch == '\x1b') {
				output.push_back('\\');
				switch (ch) {
					case '\\': output.push_back('\\'); break;
					case '\'': output.push_back('\''); break;
					case '\"': output.push_back('\"'); break;
					case '\a': output.push_back('a'); break;
					case '\b': output.push_back('b'); break;
					case '\f': output.push_back('f'); break;
					case '\n': output.push_back('n'); break;
					case '\r': output.push_back('r'); break;
					case '\t': output.push_back('t'); break;
					case '\v': output.push_back('v'); break;
					case '\x1b':
						output.append("x1b");
						break;
					}
			}
			else {
				output.push_back(ch);
			}
		}
		return output;
	}

	inline auto is_only_char(const std::string& s, char c) -> bool {
		for (const char ch : s) {
			if (ch != c) {
				return false;
			}
		}

		return true;
	}

	template<typename type>
	auto unsigned_from_string(const std::string& string, bool& overflow) -> type {
		overflow = false;
		bool is_negative = false;
		type result = 0;

		u64 start_index = 0;
		if (!string.empty() && string[0] == '-') {
			is_negative = true;
			overflow = true;
			start_index = 1;
		}

		for (u64 i = start_index; i < string.length(); ++i) {
			const char ch = string[i];

			if (!std::isdigit(ch)) {
				overflow = true;
				return result;
			}

			i32 digit = ch - '0';

			// check for overflow
			if (result > (std::numeric_limits<type>::max() - digit) / 10) {
				overflow = true;
				result = (result * 10 + digit) & std::numeric_limits<type>::max();
			}
			else {
				result = result * 10 + static_cast<type>(digit);
			}
		}

		// handle underflow by converting to max value for negative inputs
		if (is_negative) {
			return std::numeric_limits<type>::max() - result + 1;
		}

		return result;
	}

	/**
	 * \brief Converts \b str to type, allows overflow behavior, when overflow occurs the \b overflow
	 * flag is set. It's expected that \b str contains a valid value for \b type.
	 * \tparam type Type to convert string to
	 * \param string Str to parse
	 * \param overflowed Overflow flag
	 * \return \b str parsed as \b type.
	 */
	template<typename type>
	type from_string(const std::string& string, bool& overflowed) {
		static_assert(
			std::is_integral_v<type> || std::is_floating_point_v<type>,
			"'type' must be integral or floating point"
			);

		overflowed = false;

		if constexpr (std::is_integral_v<type>) {
			if constexpr (std::is_same_v<type, i8> || std::is_same_v<type, u8>) {
				i32 value;
				std::istringstream stream(string);

				stream >> value;
				overflowed = stream.fail() || value > std::numeric_limits<type>::max() || value < std::numeric_limits<type>::min();
				return static_cast<type>(value);
			}
			else if constexpr (std::is_unsigned_v<type>) {
				return unsigned_from_string<type>(string, overflowed);
			}
			else if constexpr (std::is_signed_v<type>) {
				type value;
				std::istringstream stream(string);

				stream >> value;
				overflowed = stream.fail() || value > std::numeric_limits<type>::max() || value < std::numeric_limits<type>::min();
				return value;
			}
		}
	}
} // namespace utility
