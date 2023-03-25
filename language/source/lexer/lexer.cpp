#include "lexer.h"

namespace language {
	lexer::lexer(const std::string& source_file) {
		std::string source;

		// check if the file exists, and if it has been opened successfully
		if (detail::read_file(source_file, source)) {
			m_accessor = detail::string_accessor(source);
		}
		else {
			ASSERT(false, std::string("cannot open source file '" + source_file + "'\n").c_str());
		}
	}

	void lexer::read_char() {
		m_last_character = m_accessor.get_advance();
	}

	// considerations: handle nested scopes using a stack (when we encounter a new l_brace we push a new scope to the
	//                 stack, then we add variables to that scope, once we encounter an r_brace we pop the stack.
	// implications:   add a scope class which will contain all variables in the given scope. note that we need to add
	//                 a check for l_brace/r_brace into the parser main loop, and manage the scope stack over there.

	token lexer::get_token() {
		// ignore spaces between tokens 
		while(isspace(m_last_character) && !m_accessor.end()) {
			read_char();
		}

		// identifiers
		// extract identifier, and identifier has to begin with a letter
		if(isalpha(m_last_character) && !m_accessor.end()) {
			return get_identifier_token();
		}

		// extract numbers
		if(isdigit(m_last_character) && !m_accessor.end()) {
			return get_number_token();
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

					return get_token(); // return the following token
				}
			}

			return operator_token_short->second;
		}

		// not a token, return an identifier
		m_identifier_string = m_last_character;
		return token::identifier;
    }

	token lexer::get_identifier_token()	{
		m_identifier_string = m_last_character;

		// read until we reach the end of our identifier, or the end of the file
		read_char();
		while (isalnum(m_last_character) && !m_accessor.end()) {
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
				ASSERT(false, "[lexer]: invalid number_f32 token");
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
}