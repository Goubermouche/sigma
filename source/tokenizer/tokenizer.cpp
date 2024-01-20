#include "tokenizer.h"

#include <compiler/compiler/compilation_context.h>
#include <compiler/compiler/diagnostics.h>

namespace sigma {
	tokenizer::tokenizer(const std::string& source, handle<filepath> source_path, frontend_context& context) : m_source(source), m_context(context) {
		m_token_start_location.file = source_path;
		m_current_location.file = source_path;
	}

	auto tokenizer::tokenize(const std::string& source, handle<filepath> source_path, frontend_context& context) -> utility::result<void> {
		return tokenizer(source, source_path, context).tokenize();
	}

	auto tokenizer::tokenize() -> utility::result<void> {
		auto current = token_type::UNKNOWN;

		while (current != token_type::END_OF_FILE) {
			TRY(const token_info info, get_next_token());

			current = info.tok.type;
			m_context.tokens.add_token(info);
		}

		return SUCCESS;
	}

	auto tokenizer::get_next_token() -> utility::result<token_info> {
		// consume preceding spaces
		consume_spaces();

		m_token_start_location.line_index = m_current_location.line_index;
		m_token_start_location.char_index = m_current_location.char_index - 1;

		// check for EOF so we don't have to do it in the individual brace checks
		if(m_source.end()) {
			return token_info{
				.tok = { token_type::END_OF_FILE },
				.location = m_context.allocator.emplace<token_location>(m_token_start_location)
			};
		}

		// at this point we have a non-space character
		// alphabetical tokens can either be keywords or identifiers
		if(std::isalpha(m_last_character)) {
			return get_alphabetical_token();
		}

		// sequences starting with a number should be interpreted as numerical tokens
		if(std::isdigit(m_last_character)) {
			return get_numerical_token();
		}

		// parse negative numerical literals as well
		if(m_last_character == '-' && std::isdigit(m_source.get())) {
			return get_numerical_token();
		}

		// single quote characters are interpreted as character literals
		if(m_last_character == '\'') {
			NOT_IMPLEMENTED();
		}

		// double quote characters are interpreted as string literals
		if(m_last_character == '"') {
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
			// note: don's use std::to_lower as the C standard doesn't allow \X escape sequences
			//       for hexadecimals
			if (m_last_character == 'x') {
				NOT_IMPLEMENTED();
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
				default: NOT_IMPLEMENTED();
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

	auto tokenizer::get_alphabetical_token() -> utility::result<token_info> {
		// TODO: implement a non-owning string
		// TODO: check for "__" sequences
		m_current_section = m_last_character;

		// consume a sequence of alphanumeric characters
		while (std::isalnum(get_next_char())) {
			m_current_section += m_last_character;
		}

		// check if the value we've extracted is a keyword
		const auto it = m_keyword_tokens.find(m_current_section);
		if (it != m_keyword_tokens.end()) {
			// the string is a keyword
			return token_info{
				.tok      = { it->second },
				.location = m_context.allocator.emplace<token_location>(m_token_start_location)
			};
		}

		// the string isn't a keyword, treat it as an identifier
		return token_info{
			.tok        = { token_type::IDENTIFIER },
			.location   = m_context.allocator.emplace<token_location>(m_token_start_location),
			.symbol_key = m_context.strings.insert(m_current_section)
		};
	}

	auto tokenizer::get_numerical_token() -> utility::result<token_info> {
		m_current_section = m_last_character; // prime the value string
		const char first_character = m_last_character; // store the first char of the sequence
		bool dot_met = false; // keep track of whether we've met the '.' character

		get_next_char();

		// attempt to parse a hexadecimal or binary number
		if (first_character == '0') {
			// hexadecimal
			if (std::tolower(m_last_character) == 'x') {
				NOT_IMPLEMENTED();
				//OUTCOME_TRY(m_value_string, get_hexadecimal_string());
				//return token::number_hexadecimal;
			}

			// binary
			if (std::tolower(m_last_character) == 'b') {
				NOT_IMPLEMENTED();
				//OUTCOME_TRY(m_value_string, get_binary_string());
				//return token::number_binary;
			}
		}

		// fallback to regular number formats 
		while (!std::isspace(m_last_character) && !m_source.end()) {
			if (m_last_character == '.') {
				if (dot_met) {
					return error::emit(error::code::NUMERICAL_LITERAL_MORE_THAN_ONE_DOT);
				}

				dot_met = true;
			}
			else if (m_last_character == 'u') {
				if (dot_met) {
					return error::emit(error::code::NUMERICAL_LITERAL_UNSIGNED_WITH_DOT);
				}

				get_next_char();
				return token_info{
					.tok        = { token_type::UNSIGNED_LITERAL },
					.location   = m_context.allocator.emplace<token_location>(m_token_start_location),
					.symbol_key = m_context.strings.insert(m_current_section)
				};
			}
			else if (m_last_character == 'f') {
				if (!dot_met) {
					return error::emit(error::code::NUMERICAL_LITERAL_FP_WITHOUT_DOT);
				}

				get_next_char();
				return token_info{
					.tok        = { token_type::F32_LITERAL },
					.location   = m_context.allocator.emplace<token_location>(m_token_start_location),
					.symbol_key = m_context.strings.insert(m_current_section)
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
			return token_info{
				.tok        = { token_type::F64_LITERAL },
				.location   = m_context.allocator.emplace<token_location>(m_token_start_location),
				.symbol_key = m_context.strings.insert(m_current_section)
			};
		}

		// 0 format
		return token_info{
			.tok        = { token_type::SIGNED_LITERAL },
			.location   = m_context.allocator.emplace<token_location>(m_token_start_location),
			.symbol_key = m_context.strings.insert(m_current_section)
		};
	}

	auto tokenizer::get_string_literal_token() -> utility::result<token_info> {
		m_current_section.clear();
		get_next_char(); // read the character after the opening double quote

		while (m_last_character != '"' && !m_source.end()) {
			m_current_section += get_escaped_character();
			get_next_char();
		}

		if(m_last_character != '"') {
			return error::emit(error::code::INVALID_STRING_TERMINATOR);
		}

		get_next_char();

		return token_info{
			.tok        = { token_type::STRING_LITERAL },
			.location   = m_context.allocator.emplace<token_location>(m_token_start_location),
			.symbol_key = m_context.strings.insert(m_current_section)
		};
	}

	auto tokenizer::get_special_token() -> utility::result<token_info> {
		m_current_section = m_last_character;

		const auto it = m_special_tokens.find(m_current_section);
		if (it != m_special_tokens.end()) {
			get_next_char();

			// we have a special token consisting of 1 character, check if we can find a longer
			// one using that character
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

			return token_info{
				.tok      = { it->second },
				.location = m_context.allocator.emplace<token_location>(m_token_start_location)
			};
		}

		NOT_IMPLEMENTED();
		return token_info{};
	}
} // namespace sigma::lex
