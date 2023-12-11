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
	std::string format_ending(
		u64 count,
		const std::string& singular,
		const std::string& plural
	);

	/**
	 * \brief Creates a string of length \a end with caret characters ('^') starting at \a start.
	 * \param str String to underline with carets 
	 * \param start Start index 
	 * \param end End index
	 * \return String containing a caret underline.
	 */
	std::string create_caret_underline(
		const std::string& str,
		u64 start,
		u64 end
	);

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
	std::string escape_string(const std::string& input);

	std::pair<u64, std::string> remove_leading_spaces(
		const std::string& str
	);

	void string_replace(
		std::string& str,
		const std::string& from,
		const std::string& to
	);
}
