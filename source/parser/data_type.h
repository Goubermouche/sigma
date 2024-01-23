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

		static auto create_unknown() -> data_type;
		static auto create_var_arg_promote() -> data_type;
		static auto create_bool(u8 pointer_level = 0) -> data_type;

		static auto create_i8(u8 pointer_level = 0) -> data_type;
		static auto create_i16(u8 pointer_level = 0) -> data_type;
		static auto create_i32(u8 pointer_level = 0) -> data_type;
		static auto create_i64(u8 pointer_level = 0) -> data_type;

		static auto create_u8(u8 pointer_level = 0) -> data_type;
		static auto create_u16(u8 pointer_level = 0) -> data_type;
		static auto create_u32(u8 pointer_level = 0) -> data_type;
		static auto create_u64(u8 pointer_level = 0) -> data_type;

		bool operator==(data_type other) const;
		bool operator<(data_type other) const;

		static auto token_to_type(token token) -> data_type_base;

		bool is_unknown() const;
		bool is_promote() const;

		bool is_integral() const;
		bool is_void() const;
		bool is_pointer() const;
		bool is_signed() const;
		bool is_unsigned() const;

		auto to_string() const -> std::string;
		auto get_byte_width() const -> u16;

		data_type_base base_type = UNKNOWN;
		u8 pointer_level = 0;
	};

	auto get_larger_type(data_type a, data_type b) -> data_type;
	auto promote_type(data_type type) -> data_type;

	struct named_data_type {
		named_data_type(data_type type, utility::string_table_key identifier_key);

		bool operator==(const named_data_type& other) const;

		data_type type;
		utility::string_table_key identifier_key;
	};
} // sigma::parse
