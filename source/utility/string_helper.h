#pragma once
#include "utility/types.h"

namespace utility::detail {
	/**
	 * \brief Chooses a string based on \a count (plural vs. singular).
	 * \param count Count
	 * \param singular Singular string
	 * \param plural Plural strings
	 * \return Singular/plural string based on the \a count. 
	 */
	auto format_ending(u64 count, const std::string& singular, const std::string& plural) -> std::string;

	/**
	 * \brief Creates a string of length \a end with caret characters ('^') starting at \a start.
	 * \param str String to underline with carets 
	 * \param start Start index 
	 * \param end End index
	 * \return String containing a caret underline.
	 */
	auto create_caret_underline(const std::string& str, u64 start, u64 end) -> std::string;

	/**
	 * \brief Checks if the char \a c is part of the hexadecimal character set (0123456789ABCDEF).
	 * \param c Character to check
	 * \return True if the character is part of the hexadecimal character set.
	 */
	bool is_hex(char c);

	/**
	 * \brief Checks if the char \a c is part of the binary character set (01).
	 * \param c Character to check
	 * \return True if the character is part of the binary character set.
	 */
	bool is_bin(char c);

	/**
	 * \brief Creates a backslash-escaped version of the \a input string.
	 * \param input String to escape.
	 * \return Escaped version of the given string.
	 */
	auto escape_string(const std::string& input) -> std::string;

	auto remove_leading_spaces(const std::string& str) -> std::pair<u64, std::string>;

	void string_replace(std::string& str, const std::string& from, const std::string& to);

	auto remove_first_line(const std::string& string) -> std::string;

	template<typename type>
	auto unsigned_from_string(const std::string& string, bool& overflow) -> type {
		overflow = false;
		bool is_negative = false;
		type result = 0;

		u64 start_index = 0;
		if(!string.empty() && string[0] == '-') {
			is_negative = true;
			overflow = true;
			start_index = 1;
		}

		for(u64 i = start_index; i < string.length(); ++i) {
			const char ch = string[i];

			if (!std::isdigit(ch)) {
				overflow = true;
				return result;
			}

			i32 digit = ch - '0';

			// check for overflow
			if(result > (std::numeric_limits<type>::max() - digit) / 10) {
				overflow = true;
				result = (result * 10 + digit) & std::numeric_limits<type>::max();
			}
			else {
				result = result * 10 + digit;
			}
		}

		// handle underflow by converting to max value for negative inputs
		if(is_negative) {
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
			if constexpr (std::is_unsigned_v<type>) {
				return unsigned_from_string<type>(string, overflowed);
			}
			else if constexpr (std::is_same_v<type, i8>) {
				i32 value;
				std::istringstream stream(string);

				stream >> value;
				overflowed = stream.fail() || value > std::numeric_limits<type>::max() || value < std::numeric_limits<type>::min();
				return static_cast<type>(value);
			}
			else if constexpr (std::is_signed_v<type>) {
				type value;
				std::istringstream stream(string);

				stream >> value;
				overflowed = stream.fail() || value > std::numeric_limits<type>::max() || value < std::numeric_limits<type>::min();
				return value;
			}
		}

		return type();
	}
} // namespace sigma
