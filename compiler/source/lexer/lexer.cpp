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
			if(!extract_next_token(tok)) {
				return false;
			}

			m_tokens.push_back({ tok, m_value_string });
		}

		// print_tokens();

		return true;
	}

	void lexer::print_tokens() const {
		for(const token_data& t : m_tokens) {
			std::cout << std::left << std::setw(40) << token_to_string(t.token);

			if(!t.value.empty()) {
				// the value string may contain escape sequences 
				std::cout << escape_string(t.value);
			}

			std::cout << '\n';
		}
	}

	const token_data& lexer::get_token() {
		m_token_peek_index++;
		return  m_tokens[m_token_index++];
	}

	const token_data& lexer::peek_token() {
		return m_tokens[m_token_peek_index++];
	}

	void lexer::synchronize_indices() {
		m_token_peek_index = m_token_index;
	}

	void lexer::read_char() {
		if (m_last_character == '\n') {
			m_current_line++;
		}

		m_last_character = m_accessor.get_advance();
	}

	bool lexer::extract_next_token(token& tok) {
		m_value_string.clear();

		// ignore spaces between tokens 
		while(isspace(m_last_character) && !m_accessor.end()) {
			read_char();
		}

		// identifiers
		// extract identifiers
		if(isalpha(m_last_character) && !m_accessor.end()) {
			return get_identifier_token(tok);
		}

		// extract numbers
		if(isdigit(m_last_character) && !m_accessor.end()) {
			return get_numerical_token(tok);
		}

		if (m_last_character == '\'') {
			return get_char_literal_token(tok);
		}

		if (m_last_character == '"') {
			return get_string_literal_token(tok);
		}

		// prevent '.' characters from being located at the beginning of a token
		// note: we may want to allow this in some cases (ie. when calling member functions)
		if (m_last_character == '.') {
			compilation_logger::emit_invalid_dot_character_at_token_start_error();
			tok = token::unknown;
			return false;
		}

		// check for EOF so we don't have to do it in the individual brace checks 
		if(m_accessor.end()) {
			tok = token::end_of_file;
			return true;
		}

		// extract special tokens
		std::string special_string;
		special_string = m_last_character;
		const auto operator_token_short = m_special_tokens.find(special_string);

		// todo: check for longer tokens using a while loop.
		// todo: check for longer tokens in case a short token does not exist.

		// first check if we have a special token
		if(operator_token_short != m_special_tokens.end()) {
			read_char();

			// we have a special token consisting of 1 character, check if we can find a longer one using that character 
			if(!isspace(m_last_character) && !isalnum(m_last_character)) {
				special_string += m_last_character;

				const auto operator_token_long = m_special_tokens.find(special_string);
				// we've found a longer token, return it
				if(operator_token_long != m_special_tokens.end()) {
					read_char();
					tok = operator_token_long->second;
					return true;
				}

				// since we don't have the "//" token in our token table we check for it separately, and if we find
				// it we consider it to be a comment, in this case we ignore all the characters on the same line and
				// return the following token.
				if(m_last_character == '/') {
					// ignore all remaining data on the current line
					do {
						read_char();
					} while (!m_accessor.end() && m_last_character != '\n' && m_last_character != '\r');

					return extract_next_token(tok); // return the following token
				}
			}

			tok = operator_token_short->second;
			return true;
		}

		// not a token, return an identifier
		m_value_string = m_last_character;
		tok = token::identifier;
		return true;
    }

	bool lexer::get_identifier_token(token& tok)	{
		bool last_char_was_underscore = false;
		m_value_string = m_last_character;

		// read until we reach the end of our identifier, or the end of the file
		// note that identifiers can contains '_' underscore characters with regular alnum chars between them 
		read_char();
		while ((isalnum(m_last_character) || m_last_character == '_') && !m_accessor.end()) {
			// prevent two underscore characters from being right next to each other
			if (m_last_character == '_' && last_char_was_underscore) {
				compilation_logger::emit_invalid_double_underscore_error();
				return false;
			}

			last_char_was_underscore = (m_last_character == '_');

			m_value_string += m_last_character;
			read_char();
		}

		// check if the current identifier is a keyword
		const auto token = m_keyword_tokens.find(m_value_string);
		if (token != m_keyword_tokens.end()) {
			// return the appropriate token
			m_value_string.clear(); // clear the value string since we don't want our token value to be equal to the token type
			tok = token->second;
			return true;
		}

		// the identifier is not a keyword
		tok = token::identifier;
		return true;
	}

	bool lexer::get_numerical_token(token& tok) {
		m_value_string = m_last_character;
		// keep track of whether we've met the '.' character
		bool dot_met = false;

		// assume that we currently have something like "0"
		// read until we reach the end of our value key, or the end of the file
		read_char();
		while (!isspace(m_last_character) && !m_accessor.end()) {
			if (m_last_character == '.') {
				if (dot_met) {
					compilation_logger::emit_invalid_number_format_only_one_dot_allowed_error();
					return false;
				}

				dot_met = true;
			}
			else if (m_last_character == 'u') {
				if (!dot_met) {
					read_char();
					// 0u format
					tok = token::number_unsigned;
					return true;
				}

				compilation_logger::emit_invalid_number_format_unsigned_number_may_not_contain_dot_characters_error();
				return false;
			}
			else if (m_last_character == 'f') {
				if (dot_met) {
					read_char();
					// 0.0f format
					tok = token::number_f32;
					return true;
				}

				compilation_logger::emit_invalid_number_format_floating_point_must_contain_dot_character_error();
				return false;
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
			tok = token::number_f64;
			return true;
		}

		// 0 format
		tok = token::number_signed;
		return true;
	}

	bool lexer::get_char_literal_token(token& tok) {
		m_value_string = "";
		read_char(); // read the character after the opening quote

		// handle escape characters
		if (m_last_character == '\\') {
			// handle escape sequences
			read_char();
			switch (m_last_character) {
			case '\\': m_value_string += '\\'; break;
			case '\'': m_value_string += '\''; break;
			case 'n': m_value_string += '\n'; break;
			case 't': m_value_string += '\t'; break;
			case 'r': m_value_string += '\r'; break;
			default: m_value_string += m_last_character; break;
			}
		}
		else {
			m_value_string += m_last_character;
		}

		read_char(); // read the character after the literal

		if (m_last_character == '\'') {
			read_char(); // read the character after the closing quote
			tok = token::char_literal;
			std::cout << escape_string(m_value_string) << '\n';
			return true;
		}

		compilation_logger::emit_unterminated_character_literal_error();
		return false;
	}

	bool lexer::get_string_literal_token(token& tok) {
		m_value_string = "";
		read_char();

		while(m_last_character != '"' && !m_accessor.end()) {
			if (m_last_character == '\\') {
				// handle escape sequences
				read_char();
				switch(m_last_character) {
				case '\\': m_value_string += '\\'; break;
				case '\"': m_value_string += '\"'; break;
				case 'n': m_value_string += '\n'; break;
				case 't': m_value_string += '\t'; break;
				case 'r': m_value_string += '\r'; break;
				default: m_value_string += m_last_character; break;
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
			compilation_logger::emit_unterminated_string_literal_error();
			return false;
		}

		tok = token::string_literal;
		return true;
	}
}