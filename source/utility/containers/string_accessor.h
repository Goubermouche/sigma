#pragma once
#include "utility/filesystem/filesystem.h"

namespace sigma::detail {
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
		string_accessor(std::shared_ptr<text_file> file);

		/**
		 * \brief Increments the caret location.
		 */
		inline void advance();

		/**
		 * \brief Decrements the caret location.
		 */
		inline void retreat();

		/**
		 * \brief Retrieves the character at the current caret location, if we are out of bounds an assertion is triggered.
		 * \returns Character at the current caret location
		 */
		inline char get() const;

		/**
		 * \brief Retrieves the character at the current caret location and increments the caret location, if we are out of bounds an assertion is triggered
		 * \returns Character at the current caret location 
		 */
		char get_advance();

		/**
		 * \brief Checks whether the current caret location is in/out of bounds of the contained string.
		 * \return True if the caret is out of bounds, otherwise False
		 */
		bool end() const;

		u64 get_position() const;
		void set_position(u64 position);
	private:
		std::string m_string; // contained string
		u64 m_position = 0;   // current caret location
	};
}