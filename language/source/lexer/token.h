#pragma once

namespace language {
	enum class token {
		end_of_file = -1,

		l_brace = -2,     // token identifier for the '{' character
		r_brace = -3,     // token identifier for the '}' character

		keyword_i8 = -4,
		keyword_i16 = -5,
		keyword_i32 = -6,
		keyword_i64 = -7,

		identifier = -8,
	};

	inline std::string token_to_string(const token& token) {
		switch (token) {
		case token::end_of_file:
			return "end_of_file";
		case token::l_brace:
			return "l_brace";
		case token::r_brace:
			return "r_brace";
		case token::keyword_i8:
			return "keyword_i8";
		case token::keyword_i16:
			return "keyword_i16";
		case token::keyword_i32:
			return "keyword_i32";
		case token::keyword_i64:
			return "keyword_i64";
		case token::identifier:
			return "identifier";
		}

		const char c = static_cast<char>(token);
		return std::string (&c, 1);
	}
}
