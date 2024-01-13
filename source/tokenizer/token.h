#pragma once
#include <utility/containers/string_table.h>
#include <utility/containers/handle.h>

namespace sigma {
	using namespace utility::types;

	enum class token_type {
		UNKNOWN,

		LEFT_PARENTHESIS,  // (
		RIGHT_PARENTHESIS, // )
		LEFT_BRACE,        // {
		RIGHT_BRACE,       // }
		// LEFT_BRACKET,      // [
		// RIGHT_BRACKET,     // ]
		COMMA,             // ,
		SEMICOLON,         // ;
		SINGLE_QUOTE,      // '
		DOUBLE_QUOTE,      // "
		// HASH,              // #
		// AMPERSAND,         // &
		MODULO,            // %
		SLASH,             // /
		ASTERISK,          // *
		PLUS_SIGN,         // +
		MINUS_SIGN,        // +
		EQUALS_SIGN,       // =

		// default type keywords
		I8,
		I16,
		I32,
		I64,

		BOOL,

		// control flow
		RET,                 // ret
		IF,                  // if
		ELSE,                // else

		// literals
		SIGNED_LITERAL,      // 0
		UNSIGNED_LITERAL,    // 0u
		F32_LITERAL,         // 0.0f
		F64_LITERAL,         // 0.0
		HEXADECIMAL_LITERAL, // 0x / 0X
		BINARY_LITERAL,      // 0b / 0B
		STRING_LITERAL,      // "text"
		BOOL_LITERAL_TRUE,   // true
		BOOL_LITERAL_FALSE,  // false

		IDENTIFIER,
		END_OF_FILE
	};

	struct token {
		token() = default;
		token(token_type type);

		auto is_type() const -> bool;
		auto is_numerical_literal() const -> bool;

		auto to_string() const->std::string;

		auto operator==(token other) const -> bool;
		auto operator==(token_type other) const -> bool;

		operator token_type() const;

		token_type type;
	};

	struct token_location {
		u32 line_index = 0;
		u32 char_index = 0;
	};

	struct token_info {
		token tok;
		handle<token_location> location;
		utility::string_table_key symbol_key;
	};
}
