#pragma once
#include "lexer/token.h"

namespace channel {
	enum class type {
		i8,
		i16,
		i32,
		i64
	};

	static type token_to_type(token token) {
		switch(token) {
		case token::keyword_type_i8:
			return type::i8;
		case token::keyword_type_i16:
			return type::i16;
		case token::keyword_type_i32:
			return type::i32;
		case token::keyword_type_i64:
			return type::i64;
		default:
			ASSERT(false, "token '" + token_to_string(token) + "' cannot be converted to a type");
			return type{};
		}
	}
}
