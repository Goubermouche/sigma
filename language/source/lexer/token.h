#pragma once

namespace language {
	enum class token {
		end_of_file,

		l_parenthesis, // token identifier for the '(' character
		r_parenthesis, // token identifier for the ')' character
		l_brace,       // token identifier for the '{' character
		r_brace,       // token identifier for the '}' character

		// keywords
		keyword_i8,    // i8
		keyword_i16,   // i16
		keyword_i32,   // i32
		keyword_i64,   // i64

		// symbols
		symbol_plus,        // +
		symbol_minus,       // -
		symbol_asterisk,    // *
		symbol_slash,       // /
		symbol_exclamation, // !
		symbol_semicolon,   // ;

		definition,
		identifier,
		unknown
	};

	inline std::string token_to_string(const token& token) {
		switch (token) {
		case token::end_of_file:
			return "end_of_file";
		case token::l_brace:
			return "l_brace";
		case token::r_brace:
			return "r_brace";
		case token::l_parenthesis:
			return "l_parenthesis";
		case token::r_parenthesis:
			return "r_parenthesis";

		// keywords
		case token::keyword_i8:
			return "keyword_i8";
		case token::keyword_i16:
			return "keyword_i16";
		case token::keyword_i32:
			return "keyword_i32";
		case token::keyword_i64:
			return "keyword_i64";

		// symbols
		case token::symbol_plus:
			return "symbol_plus";
		case token::symbol_minus:
			return "symbol_minus";
		case token::symbol_asterisk:
			return "symbol_asterisk";
		case token::symbol_slash:
			return "symbol_slash";
		case token::symbol_exclamation:
			return "symbol_exclamation";
		case token::symbol_semicolon:
			return "symbol_semicolon";

		// other
		case token::identifier:
			return "identifier";
		case token::definition:
			return "definition";
		case token::unknown:
			return "unknown";
		}

		return "";
	}
}
