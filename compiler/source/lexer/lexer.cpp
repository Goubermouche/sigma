#include "lexer.h"

namespace channel {
	lexer::lexer(const std::string& source_file)
		: m_source_file(source_file) {}

	bool lexer::tokenize() {
		std::string source;

		// check if the file exists, and if it has been opened successfully
		if (detail::read_file(m_source_file, source)) {
			m_accessor = detail::string_accessor(source);
		}
		else {
			compilation_logger::emit_cannot_open_file_error(m_source_file);
			return false;
		}

		// tokenize
		token tok = token::unknown;
		while(tok != token::end_of_file) {
			tok = extract_next_token();

			switch(tok) {
			// m_identifier_string
			case token::identifier:
				m_tokens.push_back({ tok, m_identifier_string });
				break;
			// m_value_string
			case token::number_unsigned:
			case token::number_signed:
			case token::number_f32:
			case token::number_f64:
			case token::char_literal:
			case token::string_literal:
				m_tokens.push_back({ tok, m_value_string });
				break;
			default:
				m_tokens.push_back({ tok, "" });
			}
		}

		// print_tokens();

		return true;
	}

	void lexer::print_tokens() const {
		for(const token_value_pair& t : m_tokens) {
			std::cout << std::left << std::setw(40) << token_to_string(t.token);

			if(!t.value.empty()) {
				// the value string may contain escape sequences 
				std::cout << escape_string(t.value);
			}

			std::cout << '\n';
		}
	}

	const token_value_pair& lexer::get_token() {
		m_token_peek_index++;
		return  m_tokens[m_token_index++];
	}

	const token_value_pair& lexer::peek_token() {
		return m_tokens[m_token_peek_index++];
	}

	void lexer::synchronize_indices() {
		m_token_peek_index = m_token_index;
	}

	u64 lexer::get_current_line_number() const {
		return m_current_line;
	}

	void lexer::read_char() {
		if (m_last_character == '\n') {
			m_current_line++;
		}

		m_last_character = m_accessor.get_advance();
	}

	token lexer::extract_next_token() {
		// ignore spaces between tokens 
		while(isspace(m_last_character) && !m_accessor.end()) {
			read_char();
		}

		// identifiers
		// extract identifiers
		if(isalpha(m_last_character) && !m_accessor.end()) {
			return get_identifier_token();
		}

		// extract numbers
		if(isdigit(m_last_character) && !m_accessor.end()) {
			return get_number_token();
		}

		if (m_last_character == '\'') {
			return get_char_literal_token();
		}

		if (m_last_character == '"') {
			return get_string_literal_token();
		}

		// prevent '.' characters from being located at the beginning of a token
		// note: we may want to allow this in some cases (ie. when calling member functions)
		if (m_last_character == '.') {
			ASSERT(false, "[lexer]: invalid '.' character at token start");
			return token::unknown;
		}

		// check for EOF so we don't have to do it in the individual brace checks 
		if(m_accessor.end()) {
			return token::end_of_file;
		}

		// extract special tokens
		m_operator_string = m_last_character;
		const auto operator_token_short = m_special_tokens.find(m_operator_string);

		// todo: check for longer tokens using a while loop.
		// todo: check for longer tokens in case a short token does not exist.

		// first check if we have a special token
		if(operator_token_short != m_special_tokens.end()) {
			read_char();

			// we have a special token consisting of 1 character, check if we can find a longer one using that character 
			if(!isspace(m_last_character) && !isalnum(m_last_character)) {
				m_operator_string += m_last_character;

				const auto operator_token_long = m_special_tokens.find(m_operator_string);
				// we've found a longer token, return it
				if(operator_token_long != m_special_tokens.end()) {
					read_char();
					return operator_token_long->second;
				}

				// since we don't have the "//" token in our token table we check for it separately, and if we find
				// it we consider it to be a comment, in this case we ignore all the characters on the same line and
				// return the following token.
				if(m_last_character == '/') {
					// ignore all remaining data on the current line
					do {
						read_char();
					} while (!m_accessor.end() && m_last_character != '\n' && m_last_character != '\r');

					return extract_next_token(); // return the following token
				}
			}

			return operator_token_short->second;
		}

		// not a token, return an identifier
		m_identifier_string = m_last_character;
		return token::identifier;
    }

	token lexer::get_identifier_token()	{
		bool last_char_was_underscore = false;
		m_identifier_string = m_last_character;

		// read until we reach the end of our identifier, or the end of the file
		// note that identifiers can contains '_' underscore characters with regular alnum chars between them 
		read_char();
		while ((isalnum(m_last_character) || m_last_character == '_') && !m_accessor.end()) {
			// prevent two underscore characters from being right next to each other
			if (m_last_character == '_' && last_char_was_underscore) {
				ASSERT(false, "[lexer]: two '_' characters immediately one after another are not allowed");
			}
			else {
				last_char_was_underscore = (m_last_character == '_');
			}

			m_identifier_string += m_last_character;
			read_char();
		}

		// check if the current identifier is a keyword
		const auto token = m_keyword_tokens.find(m_identifier_string);
		if (token != m_keyword_tokens.end()) {
			// return the appropriate token
			return token->second;
		}

		// the identifier is not a keyword
		return token::identifier;
	}

	token lexer::get_number_token() {
		m_value_string = m_last_character;
		// keep track of whether we've met the '.' character
		bool dot_met = false;

		// assume that we currently have something like "0"
		// read until we reach the end of our value key, or the end of the file
		read_char();
		while (!isspace(m_last_character) && !m_accessor.end()) {
			if (m_last_character == '.') {
				if (dot_met) {
					ASSERT(false, "[lexer]: invalid number: cannot declare number with more than one '.' characters");
				}
				dot_met = true;
			}
			else if (m_last_character == 'u') {
				if (!dot_met) {
					read_char();
					// 0u format
					return token::number_unsigned;
				}

				ASSERT(false, "[lexer]: invalid number_unsigned token");
				break;
			}
			else if (m_last_character == 'f') {
				if (dot_met) {
					read_char();
					// 0.0f format
					return token::number_f32;
				}
				ASSERT(false, "[lexer]: invalid number_f32 token (missing '.' character)");
			}
			// break early if we have a non-special and non-digit character
			else if (!isdigit(m_last_character)) {
				break;
			}

			m_value_string += m_last_character;
			read_char();
		}

		// 0.0 format
		if (dot_met) {
			return token::number_f64;
		}

		// 0 format
		return token::number_signed;
	}

	token lexer::get_char_literal_token() {
		m_value_string = "";
		read_char(); // read the character after the opening quote

		// handle escape characters
		if (m_last_character == '\\') {
			read_char();
			// todo: ... handle specific escape characters (e.g., '\\', '\'', '\n', '\t', etc.) ...
		}

		m_value_string += m_last_character;
		read_char(); // read the character after the literal

		if (m_last_character == '\'') {
			read_char(); // read the character after the closing quote
			return token::char_literal;
		}

		ASSERT(false, "[lexer]: unterminated character literal");
		return token::unknown;
	}

	token lexer::get_string_literal_token() {
		m_value_string = "";
		read_char();

		while(m_last_character != '"' && !m_accessor.end()) {
			if (m_last_character == '\\') {
				// handle escape sequences
				read_char();
				switch(m_last_character) {
				case 'n':
					m_value_string.push_back('\n');
					break;
				case 't':
					m_value_string.push_back('\t');
					break;
				case '\\':
					m_value_string.push_back('\\');
					break;
				case '"':
					m_value_string.push_back('"');
					break;
				default:
					ASSERT(false, "invalid escape sequence used in string literal");
					break;
				}
			}
			else {
				m_value_string.push_back(m_last_character);
			}

			read_char();
		}

		if(m_last_character == '"') {
			read_char();
		}
		else {
			ASSERT(false, "unterminated string literal used");
		}

		return token::string_literal;
	}
}