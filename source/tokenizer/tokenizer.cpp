#include "tokenizer.h"

namespace sigma {
	tokenizer::tokenizer(const std::string& source) : m_source(source) {}

	auto tokenizer::tokenize(const std::string& source) -> std::pair<token_buffer, utility::symbol_table> {
		return tokenizer(source).tokenize();
	}

	auto tokenizer::tokenize() -> std::pair<token_buffer, utility::symbol_table> {
		token_type current = token_type::UNKNOWN;

		while (current != token_type::END_OF_FILE) {
			const token_info info = get_next_token();
			current = info.token.type;

			m_tokens.add_token(info);
		}

		return { std::move(m_tokens), std::move(m_symbols) };
	}

	auto tokenizer::get_next_token() -> token_info {
		// consume preceding spaces
		consume_spaces();

		m_token_start_location = {
			m_current_location.line_index,
			m_current_location.char_index - 1
		};

		// check for EOF so we don't have to do it in the individual brace checks
		if (m_source.end()) {
			return { .token = { token_type::END_OF_FILE }, .location = m_current_location };
		}

		// at this point we have a non-space character
		// alphabetical tokens can either be keywords or identifiers
		if (std::isalpha(m_last_character)) {
			return get_alphabetical_token();
		}

		// sequences starting with a number should be interpreted as numerical tokens
		if (std::isdigit(m_last_character)) {
			return get_numerical_token();
		}

		// single quote characters are interpreted as character literals
		if (m_last_character == '\'') {
			ASSERT(false, "not implemented - char literals");
		}

		// double quote characters are interpreted as string literals
		if (m_last_character == '"') {
			return get_string_literal_token();
		}

		// fall back to extracting special tokens
		return get_special_token();
	}

	void tokenizer::consume_spaces() {
		while (isspace(m_last_character) && !m_source.end()) {
			get_next_char();
		}
	}

	char tokenizer::get_escaped_character() {
		// handle escape sequences
		if (m_last_character == '\\') {
			get_next_char();

			// hexadecimal char sequence
			// note: don's use std::to_lower as the C standard doesn't allow \X escape sequences for hexadecimals
			if (m_last_character == 'x') {
				ASSERT(false, "not implemented");
			}

			switch (m_last_character) {
				case '\\': return '\\';
				case '\'': return '\'';
				case '\"': return '\"';
				case '?':  return '\?';
				case 'a':  return '\a';
				case 'b':  return '\b';
				case 'f':  return '\f';
				case 'n':  return '\n';
				case 'r':  return '\r';
				case 't':  return '\t';
				case 'v':  return '\v';
				case '0':  return '\0';
				default: ASSERT(false, "not implemented - unknown escape sequence");
			}
		}

		return m_last_character;
	}

	auto tokenizer::get_next_char() -> char {
		m_last_character = m_source.get_advance();

		if(m_last_character == '\n') {
			m_current_location.line_index++;
			m_current_location.char_index = 0;
		}
		else {
			m_current_location.char_index++;
		}

		return m_last_character;
	}

	auto tokenizer::get_alphabetical_token() -> token_info {
		// TODO: implement a non-owning string
		m_current_section = m_last_character;

		// consume a sequence of alphanumeric characters
		while (std::isalnum(get_next_char())) {
			m_current_section += m_last_character;
		}

		// check if the value we've extracted is a keyword
		const auto it = m_keyword_tokens.find(m_current_section);
		if (it != m_keyword_tokens.end()) {
			// the string is a keyword
			return { .token = { it->second }, .location = m_token_start_location };
		}

		// the string isn't a keyword, treat it as an identifier
		return {
			.token = { token_type::IDENTIFIER },
			.location   = m_token_start_location,
			.symbol_key = m_symbols.insert(m_current_section)
		};


	}

	auto tokenizer::get_numerical_token() -> token_info {
		m_current_section = m_last_character; // prime the value string
		const char first_character = m_last_character; // store the first char of the sequence
		bool dot_met = false; // keep track of whether we've met the '.' character

		get_next_char();

		// attempt to parse a hexadecimal or binary number
		if (first_character == '0') {
			// hexadecimal
			if (std::tolower(m_last_character) == 'x') {
				ASSERT(false, "not implemented - hex");
				//OUTCOME_TRY(m_value_string, get_hexadecimal_string());
				//return token::number_hexadecimal;
			}

			// binary
			if (std::tolower(m_last_character) == 'b') {
				ASSERT(false, "not implemented - binary");
				//OUTCOME_TRY(m_value_string, get_binary_string());
				//return token::number_binary;
			}
		}

		// fallback to regular number formats 
		while (!std::isspace(m_last_character) && !m_source.end()) {
			if (m_last_character == '.') {
				if (dot_met) {
					// more than one dot
				}

				dot_met = true;
			}
			else if (m_last_character == 'u') {
				if (dot_met) {
					// unsigned with dot
				}

				get_next_char();
				return {
					.token = { token_type::UNSIGNED_LITERAL },
					.location   = m_token_start_location,
					.symbol_key = m_symbols.insert(m_current_section)
				};
			}
			else if (m_last_character == 'f') {
				if (!dot_met) {
					// floating point without a dot
				}

				get_next_char();
				return {
					.token = { token_type::F32_LITERAL },
					.location   = m_token_start_location,
					.symbol_key = m_symbols.insert(m_current_section)
				};
			}
			else if (!std::isdigit(m_last_character)) {
				break;
			}

			m_current_section += m_last_character;
			get_next_char();
		}

		// 0.0 format
		if (dot_met) {
			return {
				.token = { token_type::F64_LITERAL },
				.location = m_token_start_location,
				.symbol_key = m_symbols.insert(m_current_section)
			};
		}

		// 0 format
		return {
			.token = { token_type::SIGNED_LITERAL },
			.location   = m_token_start_location,
			.symbol_key = m_symbols.insert(m_current_section)
		};
	}

	auto tokenizer::get_string_literal_token() -> token_info {
		m_current_section.clear();
		get_next_char(); // read the character after the opening double quote

		while (m_last_character != '"' && !m_source.end()) {
			m_current_section += get_escaped_character();
			get_next_char();
		}

		ASSERT(m_last_character == '"', "invalid string literal terminator");
		get_next_char();

		return {
			.token      = { token_type::STRING_LITERAL },
			.location   = m_token_start_location,
			.symbol_key = m_symbols.insert(m_current_section)
		};
	}

	auto tokenizer::get_special_token() -> token_info {
		m_current_section = m_last_character;

		const auto it = m_special_tokens.find(m_current_section);
		if (it != m_special_tokens.end()) {
			get_next_char();

			// we have a special token consisting of 1 character, check if we can find a longer one using that character
			if (!isspace(m_last_character) && !isalnum(m_last_character)) {
				m_current_section += m_last_character;

				// since we don't have the "//" token in our token table we check for it separately,
				// and if we find it we consider it to be a comment, in this case we ignore all
				// the characters on the same line and return the following token
				if(m_last_character == '/') {
					// ignore all remaining data on the current line
					do {
						get_next_char();
					} while (!m_source.end() && m_last_character != '\n' && m_last_character != '\r');

					return get_next_token(); // return the following token
				}
			}

			return { .token = { it->second }, .location = m_token_start_location };
		}

		ASSERT(false, "unreachable");
		return {};
	}
} // namespace sigma::lex
