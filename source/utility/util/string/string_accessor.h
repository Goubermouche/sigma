#pragma once
#include "../macros.h"

namespace utility {
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
		string_accessor(const std::string& string) : m_string(string) {}

		/**
		 * \brief Increments the caret location.
		 */
		void advance() {
			m_position++;
		}

		/**
		 * \brief Decrements the caret location.
		 */
		void retreat() {
			m_position--;
		}

		auto peek_next_char() const -> char {
			ASSERT(m_position + 1 <= m_string.size(), "accessor out of range! (peek_next_char)");
			return m_string[m_position + 1];
		}

		/**
		 * \brief Retrieves the character at the current caret location, if we are out of bounds an assertion is triggered.
		 * \returns Character at the current caret location
		 */
		[[nodiscard]] auto get() const -> char {
			// check if we are inside of our strings' bounds
			if (m_position <= m_string.size()) {
				return m_string[m_position];
			}

			// out-of-bounds access
			return EOF;
		}

		/**
		 * \brief Retrieves the character at the current caret location and increments the caret location, if we are out of bounds an assertion is triggered
		 * \returns Character at the current caret location
		 */
		[[nodiscard]] auto get_advance() -> char {
			const char temp = get();
			advance();
			return temp;
		}

		/**
		 * \brief Checks whether the current caret location is in/out of bounds of the contained string.
		 * \return True if the caret is out of bounds, otherwise False
		 */
		[[nodiscard]] auto end() const -> bool {
			return m_position > m_string.size();
		}

		[[nodiscard]] auto get_data() const -> const std::string& {
			return m_string;
		}

		[[nodiscard]] auto get_data() -> std::string& {
			return m_string;
		}

		[[nodiscard]] auto get_position() const->u64 {
			return m_position;
		}

		void set_position(u64 position) {
			// check if we are inside of our strings' bounds
			ASSERT(position <= m_string.size(), "accessor out of range! (set)");
			m_position = position;
		}
	private:
		std::string m_string; // contained string
		u64 m_position = 0;   // current caret location
	};
} // namespace utility
