#pragma once

#include "../utility/filesystem.h"
#include "token.h"

namespace language {
	class lexer {
	public:
		lexer(const std::string& source_file);

		/**
		 * \brief Extracts the next token from the given source file and returns it.
		 * \return Extracted token
		 */
		token get_token();
	private:
		std::ifstream m_source; // source file
		std::string m_identifier_string; // current identifier
		char m_last_character = ' ';

		const std::unordered_map<std::string, token> m_token_map = {
			{ "void", token::definition  },
			{ "i8"  , token::keyword_i8  },
			{ "i16" , token::keyword_i16 },
			{ "i32" , token::keyword_i32 },
			{ "i64" , token::keyword_i64 }
		};
	};
}