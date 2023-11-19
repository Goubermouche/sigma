#include "lexer.h"
#include <utility/string_helper.h>

namespace sigma {
	token_list::token_list(std::vector<token_data> tokens)
		: m_tokens(tokens) {}

	void token_list::print_tokens() const {
		for (const token_data& t : m_tokens) {
			utility::console::out << utility::console::left << utility::console::width(50) << token_to_string(t.get_token());

			if (!t.get_value().empty()) {
				// the value string may contain escape sequences 
				utility::console::out << utility::detail::escape_string(t.get_value());
			}

			utility::console::out << '\n';
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

	utility::outcome::result<void> token_list::expect_token(token token) {
		get_token();

		if (m_current_token.get_token() == token) {
			return utility::outcome::success();
		}

		return utility::outcome::failure(utility::error::emit_assembly<utility::error_code::parser_unexpected_token>(
			utility::file_range{}, // m_current_token.get_position(),
			token,
			m_current_token.get_token()
		));
	}

	std::vector<token_data>::iterator token_list::begin() {
		return m_tokens.begin();
	}

	std::vector<token_data>::iterator token_list::end() {
		return m_tokens.end();
	}

	void token_list::set_indices(
		u64 index
	) {
		m_main_token_index = index;
		m_peek_token_index = index;
	}

	void token_list::synchronize_indices() {
		m_peek_token_index = m_main_token_index;
	}

	lexer::lexer(s_ptr<utility::text_file> file)
		: m_source_file(file) {}

	utility::outcome::result<token_list> lexer::tokenize() {
		m_accessor = utility::detail::string_accessor(m_source_file);

		token tok = token::unknown;
		while(tok != token::end_of_file) {
			OUTCOME_TRY(tok, get_next_token());

			m_tokens.emplace_back(
				token_data{
					tok,
					m_value_string,
					utility::file_range {
						m_source_file,
						utility::file_position {
							m_current_line_index,
							m_current_token_start_char_index
						},
						utility::file_position {
							m_current_line_index,
							m_current_token_end_char_index
						}
					}
				}
			);

			// console::out << token_to_string(tok) << "(" << m_current_token_start_char_index << " -> " << m_current_token_end_char_index << ")\n";
		}

		return { m_tokens };
	}

	char lexer::get_next_char() {
		m_last_character = m_accessor.get_advance();

		if (m_last_character == '\n') {
			m_current_line_index++;
			m_current_char_index = 0;
		}

		m_current_char_index++;
		return m_last_character;
	}

	utility::outcome::result<token> lexer::get_next_token() {
		// consume empty space
		while (isspace(m_last_character) && !m_accessor.end()) {
			get_next_char();
		}

		// check for EOF so we don't have to do it in the individual brace checks
		if (m_accessor.end()) {
			return token::end_of_file;
		}

		m_current_token_start_char_index = m_current_char_index - 2;

		// at this point we have a non-space character

		// alphabetical tokens can either be keywords or identifiers
		if(std::isalpha(m_last_character) && !m_accessor.end()) {
			return get_alphabetical_token();
		}

		// sequences starting with a number should be interpreted as numerical tokens
		if(std::isdigit(m_last_character) && !m_accessor.end()) {
			return get_numerical_token();
		}

		// single quote characters are interpreted as character literals
		if(m_last_character == '\'') {
			return get_character_literal_token();
		}

		// double quote characters are interpreted as string literals
		if(m_last_character == '"') {
			return get_string_literal_token();
		}

		// prevent '.' characters from being located at the beginning of a token
		// note: we may want to allow this in some cases (ie. when calling member functions)
		if (m_last_character == '.') {
			return utility::outcome::failure(
				utility::error::emit_assembly<utility::error_code::lexer_invalid_dot_at_start>()
			);
		}

		// extract special tokens
		return get_special_token();
	}

	utility::outcome::result<token> lexer::get_special_token() {
		m_value_string = m_last_character;

		const auto operator_token_short = m_special_tokens.find(m_value_string);
		// first check if we have a special token
		if (operator_token_short != m_special_tokens.end()) {
			get_next_char();
			m_current_token_end_char_index = m_current_char_index - 2;

			// we have a special token consisting of 1 character, check if we can find a longer one using that character 
			if (!isspace(m_last_character) && !isalnum(m_last_character)) {
				m_value_string += m_last_character;

				const auto operator_token_long = m_special_tokens.find(m_value_string);
				// we've found a longer token, return it
				if (operator_token_long != m_special_tokens.end()) {
					get_next_char();
					m_current_token_end_char_index = m_current_char_index - 2;
					return operator_token_long->second;
				}

				// since we don't have the "//" token in our token table we check for it separately, and if we find
				// it we consider it to be a comment, in this case we ignore all the characters on the same line and
				// return the following token.
				if (m_last_character == '/') {
					// ignore all remaining data on the current line
					do {
						get_next_char();
					} while (!m_accessor.end() && m_last_character != '\n' && m_last_character != '\r');

					return get_next_token(); // return the following token
				}
			}

			return operator_token_short->second;
		}

		// not a token, fallback to an identifier
		m_value_string = m_last_character;
		m_current_token_end_char_index = m_current_char_index - 2;
		return token::identifier;
	}

	utility::outcome::result<token> lexer::get_alphabetical_token() {
		m_value_string = m_last_character; // prime the value string

		// consume a sequence of alphanumeric characters
		while (std::isalnum(get_next_char()) || m_last_character == '_') {
			m_value_string += m_last_character;
		}

		// don't allow 3 or more consecutive underscore characters
		if (m_value_string.find("___") != std::string::npos) {
			return utility::outcome::failure(utility::error::emit_assembly<utility::error_code::lexer_double_underscore>());
		}

		m_current_token_end_char_index = m_current_char_index - 2;

		// check if the value we've extracted is a keyword
		const auto it = m_keyword_tokens.find(m_value_string);
		if(it != m_keyword_tokens.end()) {
			// the string is a keyword
			return it->second;
		}

		// the string isn't a keyword, treat it as an identifier
		return token::identifier;
	}

	utility::outcome::result<token> lexer::get_numerical_token() {
		m_value_string = m_last_character; // prime the value string
		const char first_character = m_last_character; // store the first char of the sequence
		bool dot_met = false; // keep track of whether we've met the '.' character

		get_next_char();

		// attempt to parse a hexadecimal or binary number
		if (first_character == '0') {
			// hexadecimal
			if(std::tolower(m_last_character) == 'x') {
				OUTCOME_TRY(m_value_string, get_hexadecimal_string());
				return token::number_hexadecimal;
			}

			// binary
			if (std::tolower(m_last_character) == 'b') {
				OUTCOME_TRY(m_value_string, get_binary_string());
				return token::number_binary;
			}
		}

		// fallback to regular number formats 
		while(!isspace(m_last_character) && !m_accessor.end()) {
			if(m_last_character == '.') {
				if(dot_met) {
					return utility::outcome::failure(
						utility::error::emit_assembly<utility::error_code::lexer_invalid_number_format_more_than_one_dot>()
					);
				}

				dot_met = true;
			}
			else if(m_last_character == 'u') {
				if(dot_met) {
					return utility::outcome::failure(
						utility::error::emit_assembly<utility::error_code::lexer_invalid_number_format_unsigned_containing_dot>()
					);
				}

				get_next_char();
				m_current_token_end_char_index = m_current_char_index - 2;
				return token::number_unsigned;
			}
			else if(m_last_character == 'f') {
				if(!dot_met) {
					return utility::outcome::failure(
						utility::error::emit_assembly<utility::error_code::lexer_invalid_number_format_floating_point_without_dot>()
					);
				}

				get_next_char();
				m_current_token_end_char_index = m_current_char_index - 2;
				return token::number_f32;
			}
			else if(!std::isdigit(m_last_character)) {
				break;
			}

			m_value_string += m_last_character;
			get_next_char();
		}

		m_current_token_end_char_index = m_current_char_index - 2;

		// 0.0 format
		if (dot_met) {
			return token::number_f64;
		}

		// 0 format
		return token::number_signed;
	}

	utility::outcome::result<std::string> lexer::get_hexadecimal_string(
		u64 max_length
	) {
		// the last character, at this point is an x
		std::string bytecode;
		u64 parsed_count = 0;

		// consume all valid hex characters within the specified range
		while (parsed_count < max_length && utility::detail::is_hex(get_next_char()) && !m_accessor.end()) {
			bytecode += m_last_character;
			parsed_count++;
		}

		return bytecode;
	}

	utility::outcome::result<std::string> lexer::get_binary_string(
		u64 max_length
	) {
		std::string bytecode;
		u64 parsed_count = 0;

		while (parsed_count < max_length && utility::detail::is_bin(get_next_char()) && !m_accessor.end()) {
			bytecode += m_last_character;
			parsed_count++;
		}

		return bytecode;
	}

	utility::outcome::result<std::string> lexer::get_escaped_character() {
		// handle escape sequences
		if (m_last_character == '\\') {
			get_next_char();

			// hexadecimal char sequence
			// note: don's use std::to_lower as the C standard doesn't allow \X escape sequences for hexadecimals
			if (m_last_character == 'x') {
				OUTCOME_TRY(const std::string & hex_string, get_hexadecimal_string(2));
				OUTCOME_TRY(const u64 hex_value, utility::detail::string_to_hex(hex_string));
				return std::string() + static_cast<char>(hex_value);
			}

			switch (m_last_character) {
			case '\\': return "\\";
			case '\'': return "\'";
			case '\"': return "\"";
			case '?':  return "\?";
			case 'a':  return "\a";
			case 'b':  return "\b";
			case 'f':  return "\f";
			case 'n':  return "\n";
			case 'r':  return "\r";
			case 't':  return "\t";
			case 'v':  return "\v";
			case '0':  return "\0";
			default:
				return utility::outcome::failure(
					utility::error::emit_assembly<utility::error_code::lexer_unknown_escape_sequence>(m_last_character)
				);
			}
		}

		return std::string() + m_last_character;
	}

	utility::outcome::result<token> lexer::get_character_literal_token() {
		get_next_char(); // read the character after the opening quote
		OUTCOME_TRY(m_value_string, get_escaped_character());
		get_next_char(); // consume the single quote after the character literal

		if(m_last_character != '\'') {
			return utility::outcome::failure(
				utility::error::emit_assembly<utility::error_code::lexer_unterminated_character_literal>()
			);
		}

		get_next_char(); // read the character after the closing quote
		m_current_token_end_char_index = m_current_char_index - 2;
		return token::char_literal;
	}

	utility::outcome::result<token> lexer::get_string_literal_token() {
		m_value_string.clear();
		get_next_char(); // read the character after the opening double quote

		while(m_last_character != '"' && !m_accessor.end()) {
			OUTCOME_TRY(const std::string & str, get_escaped_character());
			m_value_string += str;
			get_next_char();
		}

		if(m_last_character != '"') {
			return utility::outcome::failure(
				utility::error::emit_assembly<utility::error_code::lexer_unterminated_string_literal>()
			);
		}

		get_next_char(); // read the character after the closing double quote
		m_current_token_end_char_index = m_current_char_index - 2;
		return token::string_literal;
	}

	token_data::token_data(
		token tok,
		const std::string& value,
		const utility::file_range& range
	) : m_token(tok), m_value(value), m_range(range) {}

	token token_data::get_token() const {
		return m_token;
	}

	const std::string& token_data::get_value() const {
		return m_value;
	}

	const utility::file_range& token_data::get_range() const {
		return m_range;
	}
}
