#include "lexer.h"

namespace language {
	lexer::lexer(const std::string& source_file)
		: m_source(std::ifstream(source_file)) {

		// check if the file exists, and if it has been opened successfully
		if(!std::filesystem::exists(source_file) || m_source.bad()) {
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

	token lexer::get_token() {
		// ignore spaces between tokens 
		while(isspace(m_last_character)) {
			m_source.get(m_last_character);
		}

		// identifiers
		// extract identifier, and identifier has to begin with a letter
		if(isalpha(m_last_character) && !m_source.eof()) {
			m_identifier_string = m_last_character;

			// read until we reach the end of our identifier, or the end of the file
			m_source.get(m_last_character);
			while(isalnum(m_last_character) && !m_source.eof()) {
				m_identifier_string += m_last_character;
				m_source.get(m_last_character);
			}

			// check if the current identifier is a keyword
			const auto token = m_token_map.find(m_identifier_string);
			if(token != m_token_map.end()) {
				// return the appropriate token
				return token->second;
			}

			// the identifier is not a keyword
			return token::identifier;
		}

		// check for EOF so we don't have to do it in the individual brace checks 
		if(m_source.eof()) {
			return token::end_of_file;
		}

		// arguments
		if(m_last_character == '(') {
			m_source.get(m_last_character);
			return token::l_parenthesis;
		}

		if(m_last_character == ')') {
			m_source.get(m_last_character);
			return token::r_parenthesis;
		}

		// scopes
		if(m_last_character == '{') {
			m_source.get(m_last_character);
			return token::l_brace;
		}

		if(m_last_character == '}') {
			m_source.get(m_last_character);
			return token::r_brace;
		}

		// comments
		if(m_last_character == '/') {
			m_source.get(m_last_character);

			// check if the second character is also a slash
			if (m_last_character == '/') {
				// ignore all remaining data on the current line
				do {
					m_source.get(m_last_character);
				} while (!m_source.eof() && m_last_character != '\n' && m_last_character != '\r');
				return get_token(); // return the following token
			}
			// if it's not a slash we have to handle it as a division operation
			else {
				// probably a division operation
			}
		}

        return token::unknown;
    }
}