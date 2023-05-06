#include "lexer.h"
#include "utility/filesystem.h"

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
			error::emit<1000>(m_source_file).print();
			return false;
		}

		// tokenize
		token tok = token::unknown;
		while(tok != token::end_of_file) {
			if(!extract_next_token(tok)) {
				return false;
			}

			m_tokens.push_back({ 
				tok,
				m_value_string,
				token_position {
					m_source_file,
					m_current_line,
					m_current_character
				}
			});
		}

		return true;
	}

	void lexer::print_tokens() const {
		for(const token_data& t : m_tokens) {
			console::out << std::left << std::setw(40) << token_to_string(t.get_token());

			if(!t.get_value().empty()) {
				// the value string may contain escape sequences 
				console::out << escape_string(t.get_value());
			}

			console::out << '\n';
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
			m_current_character = 1;
		}

		m_last_character = m_accessor.get_advance();
		m_current_character++;
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
			error::emit<2000>().print();
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
				error::emit<2001>().print();
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
					error::emit<2002>().print();
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

				error::emit<2003>().print();
				return false;
			}
			else if (m_last_character == 'f') {
				if (dot_met) {
					read_char();
					// 0.0f format
					tok = token::number_f32;
					return true;
				}

				error::emit<2004>().print();
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
			return true;
		}

		error::emit<2005>().print();
		return false;
	}

	bool lexer::get_string_literal_token(token& tok) {
		m_value_string = "";
		read_char();

		while (m_last_character != '"' && !m_accessor.end()) {
			if (m_last_character == '\\') {
				// handle escape sequences
				read_char();
				switch (m_last_character) {
				case '\\': m_value_string += '\\'; break;
				case '\"': m_value_string += '\"'; break;
				case 'n': m_value_string += '\n'; break;
				case 't': m_value_string += '\t'; break;
				case 'r': m_value_string += '\r'; break;
				case 'x': { // handle hexadecimal escape sequence
					char hex_chars[3] = { 0 };
					read_char();
					hex_chars[0] = m_last_character;
					read_char();
					hex_chars[1] = m_last_character;
					unsigned int hex_value;
					sscanf(hex_chars, "%x", &hex_value);
					m_value_string += static_cast<char>(hex_value);
					break;
				}
				default: m_value_string += '\\'; m_value_string += m_last_character; break;
				}
			}
			else {
				m_value_string.push_back(m_last_character);
			}

			read_char();
		}

		if (m_last_character == '"') {
			read_char();
		}
		else {
			error::emit<2006>().print();
			return false;
		}

		tok = token::string_literal;
		return true;
	}

	token_data::token_data(token tok, const std::string& value, const token_position& position)
		: m_token(tok), m_value(value), m_position(position) {}

	token token_data::get_token() const	{
		return m_token;
	}

	const std::string& token_data::get_value() const {
		return m_value;
	}

	const token_position& token_data::get_token_position() const {
		return m_position;
	}
}
