#pragma once
#include "lexer/lexer.h"

namespace sigma {
	// note: 2-letter tokens need their first token to exist, otherwise the longer version doesn't get
	// selected and an identifier is returned.

	class char_by_char_lexer : public lexer {
	public:
		char_by_char_lexer() = default;

		outcome::result<void> tokenize() override;
	private:
		/**
		 * \brief Helper function that reads the next char in the provided source file and advances the accessor caret.
		 */
		void read_char();

		/**
		 * \brief Extracts the next token from the given source file and returns it.
		 * \return Extracted token
		 */
		outcome::result<void> extract_next_token(
			token& tok
		);

		/**
		 * \brief Extracts the next numerical token from the source accessor.
		 * \return Keyword/identifier token, depending on the format and keyword availability
		 */
		outcome::result<void> get_identifier_token(
			token& tok
		);

		/**
		 * \brief Extracts the next numerical token from the source accessor.
		 * \return Best-fitting numerical token
		 */
		outcome::result<void> get_numerical_token(
			token& tok
		);

		outcome::result<void> get_char_literal_token(
			token& tok
		);

		outcome::result<void> get_string_literal_token(
			token& tok
		);
	private:
		char m_last_character = ' ';
		std::string m_value_string;
		u64 m_current_line = 1;
		u64 m_current_character = 1;
	};
}