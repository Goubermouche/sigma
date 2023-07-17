#include "lexer.h"
#include "utility/filesystem.h"

namespace sigma {
	token_list::token_list(std::vector<token_data> tokens)
		: m_tokens(tokens) {}


	void token_list::print_tokens() const {
		for (const token_data& t : m_tokens) {
			// console::out << std::left << std::setw(40) << token_to_string(t.get_token());

			if (!t.get_value().empty()) {
				// the value string may contain escape sequences 
				console::out << escape_string(t.get_value());
			}

			console::out << '\n';
		}
	}

	const token_data& token_list::get_token() {
		m_peek_token_index++;
		m_current_token = m_tokens[m_main_token_index++];
		return m_current_token;
	}

	const token_data& token_list::peek_token() {
		return m_tokens[m_peek_token_index++];
	}

	const token_data& token_list::get_current_token() const {
		return m_current_token;
	}

	outcome::result<void> token_list::expect_token(token token) {
		get_token();

		if (m_current_token.get_token() == token) {
			return outcome::success();
		}

		return outcome::failure(error::emit<3000>(
			m_current_token.get_position(),
			token,
			m_current_token.get_token()
		));
	}

	void token_list::synchronize_indices() {
		m_peek_token_index = m_main_token_index;
	}

	outcome::result<void> lexer::set_source_filepath(
		const filepath& path
	) {
		// check if the file exists, and if it has been opened successfully
		OUTCOME_TRY(auto file_contents, detail::read_file(path));

		m_source_path = path;
		m_accessor = detail::string_accessor(file_contents);
		return outcome::success();
	}

	token_list lexer::get_token_list() const {
		return { m_tokens };
	}

	outcome::result<void> lexer::tokenize() {
		// tokenize
		token tok = token::unknown;
		while (tok != token::end_of_file) {
			OUTCOME_TRY(extract_next_token(tok));

			m_tokens.push_back({
				tok,
				m_value_string,
				file_position {
					m_source_path,
					m_current_line,
					m_current_character
				}
				});
		}

		return outcome::success();
	}

	void lexer::read_char() {
		if (m_last_character == '\n') {
			m_current_line++;
			m_current_character = 1;
		}

		m_last_character = m_accessor.get_advance();
		m_current_character++;
	}

	outcome::result<void> lexer::extract_next_token(token& tok) {
		m_value_string.clear();

		// ignore spaces between tokens 
		while (isspace(m_last_character) && !m_accessor.end()) {
			read_char();
		}

		// identifiers
		// extract identifiers
		if (isalpha(m_last_character) && !m_accessor.end()) {
			return get_identifier_token(tok);
		}

		// extract numbers
		if (isdigit(m_last_character) && !m_accessor.end()) {
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
			return outcome::failure(error::emit<2000>());
		}

		// check for EOF so we don't have to do it in the individual brace checks 
		if (m_accessor.end()) {
			tok = token::end_of_file;
			return outcome::success();
		}

		// extract special tokens
		std::string special_string;
		special_string = m_last_character;
		const auto operator_token_short = m_special_tokens.find(special_string);

		// todo: check for longer tokens using a while loop.
		// todo: check for longer tokens in case a short token does not exist.

		// first check if we have a special token
		if (operator_token_short != m_special_tokens.end()) {
			read_char();

			// we have a special token consisting of 1 character, check if we can find a longer one using that character 
			if (!isspace(m_last_character) && !isalnum(m_last_character)) {
				special_string += m_last_character;

				const auto operator_token_long = m_special_tokens.find(special_string);
				// we've found a longer token, return it
				if (operator_token_long != m_special_tokens.end()) {
					read_char();
					tok = operator_token_long->second;
					return outcome::success();
				}

				// since we don't have the "//" token in our token table we check for it separately, and if we find
				// it we consider it to be a comment, in this case we ignore all the characters on the same line and
				// return the following token.
				if (m_last_character == '/') {
					// ignore all remaining data on the current line
					do {
						read_char();
					} while (!m_accessor.end() && m_last_character != '\n' && m_last_character != '\r');

					return extract_next_token(tok); // return the following token
				}
			}

			tok = operator_token_short->second;
			return outcome::success();
		}

		// not a token, return an identifier
		m_value_string = m_last_character;
		tok = token::identifier;
		return outcome::success();
	}

	outcome::result<void> lexer::get_identifier_token(token& tok) {
		bool last_char_was_underscore = false;
		m_value_string = m_last_character;

		// read until we reach the end of our identifier, or the end of the file
		// note that identifiers can contains '_' underscore characters with regular alnum chars between them 
		read_char();
		while ((isalnum(m_last_character) || m_last_character == '_') && !m_accessor.end()) {
			// prevent two underscore characters from being right next to each other
			if (m_last_character == '_' && last_char_was_underscore) {
				return outcome::failure(error::emit<2001>());
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
			return outcome::success();
		}

		// the identifier is not a keyword
		tok = token::identifier;
		return outcome::success();
	}

	outcome::result<void> lexer::get_numerical_token(token& tok) {
		m_value_string = m_last_character;
		// keep track of whether we've met the '.' character
		bool dot_met = false;

		// assume that we currently have something like "0"
		// read until we reach the end of our value key, or the end of the file
		read_char();
		while (!isspace(m_last_character) && !m_accessor.end()) {
			if (m_last_character == '.') {
				if (dot_met) {
					return outcome::failure(error::emit<2002>());
				}

				dot_met = true;
			}
			else if (m_last_character == 'u') {
				if (!dot_met) {
					read_char();
					// 0u format
					tok = token::number_unsigned;
					return outcome::success();
				}

				return outcome::failure(error::emit<2003>());
			}
			else if (m_last_character == 'f') {
				if (dot_met) {
					read_char();
					// 0.0f format
					tok = token::number_f32;
					return outcome::success();
				}

				return outcome::failure(error::emit<2004>());
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
			return outcome::success();
		}

		// 0 format
		tok = token::number_signed;
		return outcome::success();
	}

	outcome::result<void> lexer::get_char_literal_token(
		token& tok
	) {
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
			return outcome::success();
		}

		return outcome::failure(error::emit<2005>());
	}

	outcome::result<void> lexer::get_string_literal_token(
		token& tok
	) {
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
					u64 hex_value;
					sscanf_s(hex_chars, "%" SCNx64, &hex_value);
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
			return outcome::failure(error::emit<2006>());
		}

		tok = token::string_literal;
		return outcome::success();
	}

	token_data::token_data(
		token tok,
		const std::string& value,
		const file_position& position
	)
		: m_token(tok), m_value(value), m_position(position) {}

	token token_data::get_token() const {
		return m_token;
	}

	const std::string& token_data::get_value() const {
		return m_value;
	}

	const file_position& token_data::get_position() const {
		return m_position;
	}
}