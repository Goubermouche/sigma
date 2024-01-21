#pragma once
#include <tokenizer/token.h>

namespace sigma {
	using namespace utility::types;
	
	struct data_type {
		enum data_type_base : u8 {
			UNKNOWN,         // handled in the type checker
			VAR_ARG_PROMOTE, // promotes the type in a var arg context
			VOID,
			I8,
			I16,
			I32,
			I64,
			U8,
			U16,
			U32,
			U64,
			BOOL,
			CHAR
		};

		data_type() = default;
		data_type(token token, u8 pointer_level);
		data_type(data_type_base type, u8 pointer_level);

		bool operator==(data_type other) const;
		bool operator<(data_type other) const;

		static auto token_to_type(token token) -> data_type_base;

		bool is_integer() const;
		bool is_void() const;
		bool is_unknown() const;
		bool is_pointer() const;
		bool is_signed() const;

		auto to_string() const -> std::string;
		auto get_byte_width() const -> u16;

		data_type_base base_type = UNKNOWN;
		u8 pointer_level = 0;
	};

	struct named_data_type {
		named_data_type(data_type type, utility::string_table_key identifier_key);

		bool operator==(const named_data_type& other) const;

		data_type type;
		utility::string_table_key identifier_key;
	};
} // sigma::parse
