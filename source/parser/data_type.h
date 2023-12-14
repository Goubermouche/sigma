#pragma once
#include "tokenizer/token.h"

namespace sigma {
	using namespace utility::types;
	
	class data_type {
	public:
		enum base : u8 {
			UNKNOWN,         // handled in the type checker
			VAR_ARG_PROMOTE, // promotes the type in a var arg context
			VOID,
			I8,
			I16,
			I32,
			I64,
			BOOL,
			CHAR
		};

		data_type() = default;
		data_type(token token, u8 pointer_level);
		data_type(base type, u8 pointer_level);

		auto operator==(data_type other) const -> bool;

		auto to_string() const -> std::string;

		auto get_byte_width() const -> u16;
	private:
		static auto token_to_type(token token) -> base;
	public:
		base type = UNKNOWN;
		u8 pointer_level = 0;
	};

	struct named_data_type {
		data_type type;
		utility::symbol_table_key identifier_key;
	};
} // sigma::parse
