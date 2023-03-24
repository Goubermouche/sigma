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

	// considerations: handle nested scopes using a stack (when we encounter
	//                 a new l_brace we push a new scope to the stack, then
	//                 we add variables to that scope, once we encounter an
	//                 r_brace we pop the stack.

	// implications:   add a scope class which will contain all variables in
	//                 the given scope. note that we need to add a check for
	//                 l_brace/r_brace into the parser main loop, and manage
	//                 the scope stack over there.

	// todo:           implement a get function that checks if EOF has been 
	//                 found.

	// todo:           implement a class for containing the source string
	//                 and necessary caret operations.

	token lexer::get_token() {
		// ignore spaces between tokens 
		while(isspace(m_last_character) && !m_accessor.end()) {
			read_char();
		}

		// identifiers
		// extract identifier, and identifier has to begin with a letter
		if(isalpha(m_last_character) && !m_accessor.end()) {
			m_identifier_string = m_last_character;

			// read until we reach the end of our identifier, or the end of the file
			read_char();
			while(isalnum(m_last_character) && !m_accessor.end()) {
				m_identifier_string += m_last_character;
				read_char();
			}

			// check if the current identifier is a keyword
			const auto token = m_keyword_tokens.find(m_identifier_string);
			if(token != m_keyword_tokens.end()) {
				// return the appropriate token
				return token->second;
			}

			// the identifier is not a keyword
			return token::identifier;
		}

		// check for EOF so we don't have to do it in the individual brace checks 
		if(m_accessor.end()) {
			return token::end_of_file;
		}

		// comments
		if(m_last_character == '/') {
			read_char();

			// check if the second character is also a slash
			if (m_last_character == '/') {
				// ignore all remaining data on the current line
				do {
					read_char();
				} while (!m_accessor.end() && m_last_character != '\n' && m_last_character != '\r');
				return get_token(); // return the following token
			}
			// if it's not a slash we have to handle it as a division operation
			else {
				// probably a division operation
				read_char();
				return token::symbol_slash;
			}
		}

		// char tokens ('{', '}', etc.)
		const auto char_token = m_single_tokens.find(m_last_character);
		if (char_token != m_single_tokens.end()) {
			read_char();

			// return the appropriate token
			return char_token->second;
		}

		// not a token, return an identifier
		m_identifier_string = m_last_character;
		return token::identifier;
    }

	void lexer::read_char() {
		m_accessor.get(m_last_character);
		m_accessor.advance();
	}
}