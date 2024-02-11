#pragma once
#include <tokenizer/token.h>
#include <utility/containers/slice.h>

namespace sigma {
	using namespace utility::types;
	
	class type {
	public:
		enum kind {
			UNKNOWN,         // handled in the type checker
			VAR_ARG_PROMOTE, // promotes the type in a var arg context
			UNRESOLVED,      // unresolved types
			VOID,
			I8,
			I16,
			I32,
			I64,
			U8,
			U16,
			U32,
			U64,
			F32,
			F64,
			BOOL,
			CHAR,
			STRUCT
		};

		static auto create_struct(const utility::slice<type, u8>& members, utility::string_table_key identifier) -> type;
		static auto create_member(const type& ty, utility::string_table_key identifier) -> type;

		static auto create_unknown() -> type;
		static auto create_promote() -> type;

		static auto create_i8(u8 pointer_level = 0) -> type;
		static auto create_i16(u8 pointer_level = 0) -> type;
		static auto create_i32(u8 pointer_level = 0) -> type;
		static auto create_i64(u8 pointer_level = 0) -> type;

		static auto create_u8(u8 pointer_level = 0) -> type;
		static auto create_u16(u8 pointer_level = 0) -> type;
		static auto create_u32(u8 pointer_level = 0) -> type;
		static auto create_u64(u8 pointer_level = 0) -> type;

		static auto create_char(u8 pointer_level = 0) -> type;
		static auto create_bool(u8 pointer_level = 0) -> type;
		static auto create_void(u8 pointer_level) -> type;

		auto get_member_offset(utility::string_table_key identifier) const -> u16;
		auto get_alignment() const -> u16;
		auto get_size() const -> u16;

		auto get_member_identifier() const -> utility::string_table_key;
		auto get_struct_members() const -> const utility::slice<type, u8>&;
		auto get_struct_members() -> utility::slice<type, u8>&;
		auto get_unresolved() const -> utility::string_table_key;
		auto get_pointer_level() const -> u8;
		auto get_kind() const -> kind;

		void set_struct_members(const utility::slice<type, u8>& members);
		void set_kind(kind kind);

		auto dereference(u8 level) const -> type;
		auto to_string() const -> std::string;

		auto is_pure_void() const -> bool;

		auto is_void_pointer() const -> bool;
		auto is_pointer() const -> bool;

		auto is_floating_point() const -> bool;
		auto is_integral() const -> bool;
		auto is_unsigned() const -> bool;
		auto is_signed() const -> bool;
		auto is_struct() const -> bool;

		auto is_unresolved() const -> bool;
		auto is_unknown() const -> bool;
		auto is_promote() const -> bool;

		type(const token_info& token, u8 pointer_level);
		type(kind kind, u8 pointer_level);
		type();

		auto operator==(const type& other) const -> bool;
		auto operator<(const type& other) const -> bool;
	private:
		kind m_kind;
		u8 m_pointer_level; // level of indirection
		utility::string_table_key m_identifier; // struct member identifiers

		union {
			// UNRESOLVED
			// unresolved types only provide the type name
			utility::string_table_key m_unresolved;

			// STRUCT
			utility::slice<type, u8> m_struct_members;
		};
	};

	namespace detail {
		auto get_larger_type(type a, type b) -> type;
		auto promote_type(type ty) -> type;
	} // namespace detail

	struct named_data_type {
		named_data_type(type type, utility::string_table_key identifier_key);

		bool operator==(const named_data_type& other) const;

		type type;
		utility::string_table_key identifier_key;
	};

	// a higher-level function signature (contains_function info about custom types and generics)
	struct function_signature {
		bool operator==(const function_signature& other) const;
		bool operator<(const function_signature& other) const;

		type return_type;
		utility::slice<named_data_type> parameter_types;
		bool has_var_args = false;

		utility::string_table_key identifier_key;
	};
} // namespace sigma
