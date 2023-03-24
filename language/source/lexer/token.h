#pragma once
#include "../utility/macros.h"

namespace language {
	enum class token {

		l_parenthesis,      // token identifier for the '(' character
		r_parenthesis,      // token identifier for the ')' character
		l_brace,            // token identifier for the '{' character
		r_brace,            // token identifier for the '}' character
					        
		// keywords
		// signed integers
		keyword_i8,         // i8  0
		keyword_i16,        // i16 0
		keyword_i32,        // i32 0
		keyword_i64,        // i64 0

		// unsigned integers
		keyword_u8,         // u8  0u
		keyword_u16,        // u16 0u
		keyword_u32,        // u32 0u
		keyword_u64,        // u64 0u

		// floating point
		keyword_f32,        // f32 0.0f
		keyword_f64,        // f64 0.0

		// symbols
		symbol_plus,        // +
		symbol_minus,       // -
		symbol_asterisk,    // *
		symbol_slash,       // /
		symbol_equals,      // =
		symbol_exclamation, // !
		symbol_semicolon,   // ;

		// numbers
		number_signed,
		number_unsigned,
		number_f32,
		number_f64,

		definition,
		identifier,

		end_of_file,
		unknown
	};

	inline std::string token_to_string(const token& token) {
		switch (token) {
		case token::l_brace:
			return "l_brace";
		case token::r_brace:
			return "r_brace";
		case token::l_parenthesis:
			return "l_parenthesis";
		case token::r_parenthesis:
			return "r_parenthesis";

		// keywords
		// signed integers
		case token::keyword_i8:
			return "keyword_i8";
		case token::keyword_i16:
			return "keyword_i16";
		case token::keyword_i32:
			return "keyword_i32";
		case token::keyword_i64:
			return "keyword_i64";

		// unsigned integers
		case token::keyword_u8:
			return "keyword_u8";
		case token::keyword_u16:
			return "keyword_u16";
		case token::keyword_u32:
			return "keyword_u32";
		case token::keyword_u64:
			return "keyword_u64";

		// floating point
		case token::keyword_f32:
			return "keyword_f32";
		case token::keyword_f64:
			return "keyword_f64";

		// symbols
		case token::symbol_plus:
			return "symbol_plus";
		case token::symbol_minus:
			return "symbol_minus";
		case token::symbol_asterisk:
			return "symbol_asterisk";
		case token::symbol_slash:
			return "symbol_slash";
		case token::symbol_equals:
			return "symbol_equals";
		case token::symbol_exclamation:
			return "symbol_exclamation";
		case token::symbol_semicolon:
			return "symbol_semicolon";

		// numbers
		case token::number_signed:
			return "number_signed";
		case token::number_unsigned:
			return "number_unsigned";
		case token::number_f32:
			return "number_f32";
		case token::number_f64:
			return "number_f64";

		// other
		case token::identifier:
			return "identifier";
		case token::definition:
			return "definition";
		case token::end_of_file:
			return "end_of_file";
		case token::unknown:
			return "unknown";
		}

		return "";
	}
}
