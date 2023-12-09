#pragma once
#include "utility/filesystem/filesystem.h"

namespace utility::detail {
	/**
	 * \brief Utility string container class with support for caret movements.
	 */
	class string_accessor {
	public:
		string_accessor() = default;

		/**
		 * \brief Constructs the string accessor by filling it with the specified \a string.
		 * \param string String to use as the base of the string accessor
		 */
		string_accessor(const std::string& string);
		string_accessor(s_ptr<text_file> file);

		/**
		 * \brief Increments the caret location.
		 */
		void advance();

		/**
		 * \brief Decrements the caret location.
		 */
		void retreat();

		/**
		 * \brief Retrieves the character at the current caret location, if we are out of bounds an assertion is triggered.
		 * \returns Character at the current caret location
		 */
		[[nodiscard]] auto get() const -> char;

		/**
		 * \brief Retrieves the character at the current caret location and increments the caret location, if we are out of bounds an assertion is triggered
		 * \returns Character at the current caret location 
		 */
		[[nodiscard]] auto get_advance() -> char;

		/**
		 * \brief Checks whether the current caret location is in/out of bounds of the contained string.
		 * \return True if the caret is out of bounds, otherwise False
		 */
		[[nodiscard]] auto end() const -> bool;

		[[nodiscard]] auto get_data() const -> const std::string&;
		[[nodiscard]] auto get_data() -> std::string&;
		[[nodiscard]] auto get_position() const -> u64;

		void set_position(u64 position);
	private:
		std::string m_string; // contained string
		u64 m_position = 0;   // current caret location
	};
}
