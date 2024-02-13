#include "type.h"

#include <util/macros.h>

namespace sigma {
  named_data_type::named_data_type(sigma::type type, utility::string_table_key identifier_key)
	  : type(type), identifier_key(identifier_key) {}

	bool named_data_type::operator==(const named_data_type& other) const {
		return type == other.type && identifier_key == other.identifier_key;
	}

	bool function_signature::operator==(const function_signature& other) const {
		return
			identifier_key == other.identifier_key &&
			return_type == other.return_type &&
			parameter_types == other.parameter_types &&
			has_var_args == other.has_var_args;
	}

	bool function_signature::operator<(const function_signature& other) const {
		// we explicitly don't take the return type into account, since we cannot reason about which
		// function is being called when the only difference is the return type itself

		if (parameter_types.get_size() < other.parameter_types.get_size()) { return true; }
		if (parameter_types.get_size() > other.parameter_types.get_size()) { return false; }

		for (size_t i = 0; i < other.parameter_types.get_size(); ++i) {
			if (parameter_types[i].type < other.parameter_types[i].type) { return true; }
			if (parameter_types[i].type < other.parameter_types[i].type) { return false; }
		}

		return has_var_args < other.has_var_args;
	}

	namespace detail {

		auto get_larger_type(type a, type b) -> type {
			if (a.is_unknown() || b.is_unknown() || a.is_promote() || b.is_promote()) {
				return type::create_unknown();
			}

			// consider pointers as automatically the larger/more prominent type
			if(a.is_pointer()) {
				return a;
			}

			if(b.is_pointer()) {
				return b;
			}

			if (a == b) {
				return a; // same type
			}

			const u16 width_a = a.get_size();
			const u16 width_b = b.get_size();

			if (width_a == width_b) {
				// prefer signed over unsigned
				if (a.is_signed() && b.is_unsigned()) {
					return a;
				}

				if (a.is_unsigned() && b.is_signed()) {
					return b;
				}

				// if both are either signed or unsigned, return either.
				return a;
			}

			// return the type with the larger byte width.
			return width_a > width_b ? a : b;
		}

		auto promote_type(type ty) -> type {
			if (ty.is_pointer()) {
				return ty; // don't promote pointers
			}

			switch (ty.get_kind()) {
				case type::VOID: PANIC("cannot dereference a void*");
				case type::I8:
				case type::I16:
				case type::U8:
				case type::U16:
				case type::BOOL:
				case type::CHAR: return type::create_i32();
				case type::I32:
				case type::I64:
				case type::U32:
				case type::U64:  return ty;
				default: NOT_IMPLEMENTED();
			}

			// unreachable
			return {};
		}
	}

	type::type(const token_info& token, u8 pointer_level)
		: m_pointer_level(pointer_level) {
			switch (token.tok.type) {
			case token_type::I8:   m_kind = I8; break;
			case token_type::I16:  m_kind = I16; break;
			case token_type::I32:  m_kind = I32; break;
			case token_type::I64:  m_kind = I64; break;
			case token_type::U8:   m_kind = U8; break;
			case token_type::U16:  m_kind = U16; break;
			case token_type::U32:  m_kind = U32; break;
			case token_type::U64:  m_kind = U64; break;
			case token_type::BOOL: m_kind = BOOL; break;
			case token_type::VOID: m_kind = VOID; break;
			case token_type::CHAR: m_kind = CHAR; break;
			case token_type::IDENTIFIER: {
				// custom types, resolved in the type checker
				m_kind = UNRESOLVED;
				m_unresolved = token.symbol_key;
				break;
			}
			default: PANIC("undefined token -> type conversion for token '{}'", token.tok.to_string());
		}
	}

	type::type(kind kind, u8 pointer_level)
		: m_kind(kind), m_pointer_level(pointer_level) {}

	type::type() : m_kind(UNKNOWN) {}

	auto type::operator==(const type& other) const -> bool {
		if(m_kind == other.m_kind && m_pointer_level == other.m_pointer_level) {
			if(is_struct()) {
				return m_identifier == other.m_identifier && m_namespaces == other.m_namespaces;
			}

			return true;
		}

		return false;
	}

	auto type::operator<(const type& other) const -> bool {
		if(m_pointer_level < other.m_pointer_level) {
			return true;
		}

		if(m_pointer_level > other.m_pointer_level) {
			return false;
		}

		if(m_kind < other.m_kind) {
			return true;
		}

		if(m_kind > other.m_kind) {
			return false;
		}

		return m_identifier < other.m_identifier;
	}

	auto type::create_member(const type& ty, utility::string_table_key identifier) -> type {
		type member_ty = { ty.m_kind, ty.m_pointer_level };
		member_ty.m_identifier = identifier;

		if(ty.m_kind == UNRESOLVED) {
			member_ty.m_unresolved = ty.m_unresolved;
		}

		return member_ty;
	}

	auto type::create_struct(const utility::slice<type, u8>& members, utility::string_table_key identifier) -> type {
		type struct_ty = { STRUCT, 0 };
		struct_ty.m_struct_members = members;
		struct_ty.m_identifier = identifier;

		return struct_ty;
	}

	auto type::create_unknown() -> type {
		return { UNKNOWN, 0 };
	}

	auto type::create_promote() -> type {
		return { VAR_ARG_PROMOTE, 0 };
	}

	auto type::create_i8(u8 pointer_level) -> type {
		return { I8, pointer_level };
	}

	auto type::create_i16(u8 pointer_level) -> type {
		return { I16, pointer_level };
	}

	auto type::create_i32(u8 pointer_level) -> type {
		return { I32, pointer_level };
	}

	auto type::create_i64(u8 pointer_level) -> type {
		return { I64, pointer_level };
	}

	auto type::create_u8(u8 pointer_level) -> type {
		return { U8, pointer_level };
	}

	auto type::create_u16(u8 pointer_level) -> type {
		return { U16, pointer_level };
	}

	auto type::create_u32(u8 pointer_level) -> type {
		return { U32, pointer_level };
	}

	auto type::create_u64(u8 pointer_level) -> type {
		return { U64, pointer_level };
	}

	auto type::create_char(u8 pointer_level) -> type {
		return { CHAR, pointer_level };
	}

	auto type::create_bool(u8 pointer_level) -> type {
		return { BOOL, pointer_level };
	}

	auto type::create_void(u8 pointer_level) -> type {
		return { VOID, pointer_level };
	}

	auto type::get_member_offset(utility::string_table_key identifier) const -> u16 {
		ASSERT(m_kind == STRUCT, "cannot calculate struct member offset on a non-struct type");
		u16 offset = 0;

		for (const auto& member : m_struct_members) {
			// calculate padding based on alignment
			const u16 alignment = member.get_alignment();
			if (alignment != 0) {
				const u16 padding = (alignment - (offset % alignment)) % alignment;
				offset += padding;
			}

			// check if this is the target member
			if (member.m_identifier == identifier) {
				return offset;
			}

			// add the size of the current member
			offset += member.get_size();
		}

		PANIC("unknown member");
		return 0;
	}

	auto type::get_alignment() const -> u16 {
		if (m_pointer_level > 0) {
			return 8;
		}

		switch(m_kind) {
			case VAR_ARG_PROMOTE: return 0;
			case UNKNOWN:         return 0;
			case VOID:            return 0;
			case I8:              return 1;
			case I16:             return 2;
			case I32:             return 4;
			case I64:             return 8;
			case U8:              return 1;
			case U16:             return 2;
			case U32:             return 4;
			case U64:             return 8;
			case BOOL:            return 1;
			case CHAR:            return 1;
			case STRUCT: {
				u16 max_alignment = 0;

				for (const auto& member : m_struct_members) {
					max_alignment = std::max(max_alignment, member.get_alignment());
				}

				return max_alignment;
			}
			default: PANIC("undefined byte width for type '{}'", to_string());
		}

		return 0;
	}

	auto type::get_size() const -> u16 {
		if (m_pointer_level > 0) {
			return 8;
		}

		switch (m_kind) {
			case VAR_ARG_PROMOTE: return 0;
			case UNKNOWN:         return 0;
			case VOID:            return 0;
			case I8:              return 1;
			case I16:             return 2;
			case I32:             return 4;
			case I64:             return 8;
			case U8:              return 1;
			case U16:             return 2;
			case U32:             return 4;
			case U64:             return 8;
			case BOOL:            return 1;
			case CHAR:            return 1;
			case STRUCT: {
				u16 total_size = 0;
				u16 max_alignment = 0;

				for (const auto& member : m_struct_members) {
					const u16 alignment = member.get_alignment();
					const u16 padding = (alignment - (total_size % alignment)) % alignment;

					max_alignment = std::max(max_alignment, alignment);

					// align the current total size to the member's alignment requirement
					total_size += padding + member.get_size();
				}

				// align the total size of the struct to the largest member's alignment
				const u16 struct_padding = (max_alignment - (total_size % max_alignment)) % max_alignment;
				total_size += struct_padding;

				return total_size;
			}
			default: PANIC("undefined byte size for type '{}'", to_string());
		}

		return 0;
	}

	auto type::get_member_identifier() const -> utility::string_table_key {
		return m_identifier;
	}

	auto type::get_struct_members() const -> const utility::slice<type, u8>& {
		ASSERT(m_kind == STRUCT, "cannot get struct members from a non-struct type");
		return m_struct_members;
	}

	auto type::get_struct_members() -> utility::slice<type, u8>& {
		ASSERT(m_kind == STRUCT, "cannot get struct members from a non-struct type");
		return m_struct_members;
	}

	auto type::get_unresolved() const -> utility::string_table_key {
		ASSERT(m_kind == UNRESOLVED, "cannot get unresolved typename from a resolved type");
		return m_unresolved;
	}

	auto type::get_namespaces() const -> const namespace_list& {
		return m_namespaces;
	}

	auto type::get_pointer_level() const -> u8 {
		return m_pointer_level;
	}

	auto type::get_kind() const -> kind {
		return m_kind;
	}

	void type::set_struct_members(const utility::slice<type, u8>& members) {
		m_struct_members = members;
	}

	void type::set_namespaces(const namespace_list& namespaces) {
		m_namespaces = namespaces;
	}

	void type::set_kind(kind kind) {
		m_kind = kind;
	}

	auto type::dereference(u8 level) const -> type {
		ASSERT(m_pointer_level - level >= 0, "cannot create a negative pointer level");
		const u8 new_pointer_level = m_pointer_level - level;
		return { m_kind, new_pointer_level };
	}

	auto type::to_string() const -> std::string {
		std::string result;

		switch(m_kind) {
			case VAR_ARG_PROMOTE: result = "promote"; break;
			case UNKNOWN:         result = "unknown"; break;
			case VOID:            result = "void"; break;
			case I8:              result = "i8"; break;
			case I16:             result = "i16"; break;
			case I32:             result = "i32"; break;
			case I64:             result = "i64"; break;
			case U8:              result = "u8"; break;
			case U16:             result = "u16"; break;
			case U32:             result = "u32"; break;
			case U64:             result = "u64"; break;
			case BOOL:            result = "bool"; break;
			case CHAR:            result = "char"; break;
			case UNRESOLVED:      result = "unresolved"; break;
			case STRUCT: {
				result = "struct{ ";
				for (const auto& member : m_struct_members) {
					result += member.to_string() + " ";
				}

				result += "}";
				break;
			}
			default: NOT_IMPLEMENTED();
		}

		return result + std::string(m_pointer_level, '*');
	}

	auto type::is_pure_void() const -> bool {
		return m_pointer_level == 0 && m_kind == VOID;
	}

	auto type::is_floating_point() const -> bool {
		if (m_pointer_level > 0) {
			return false;
		}

		return m_kind == F32 || m_kind == F64;
	}

	auto type::is_integral() const -> bool {
		if (m_pointer_level > 0) {
			return false;
		}

		switch(m_kind) {
			case I8:
			case I16:
			case I32:
			case I64:
			case U8:
			case U16:
			case U32:
			case U64:
			case BOOL:
				return true;
			default:
				return false;
			}
	}

	auto type::is_unsigned() const -> bool {
		if(m_pointer_level > 0) {
			return false;
		}

		switch (m_kind) {
			case U8:
			case U16:
			case U32:
			case U64:
			case BOOL:
				return true;
			default:
				return false;
		}
	}

	auto type::is_void_pointer() const -> bool {
		return m_pointer_level > 0 && m_kind == VOID;
	}

	auto type::is_pointer() const -> bool {
		return m_pointer_level > 0;
	}

	auto type::is_signed() const -> bool {
		if (m_pointer_level > 0) {
			return false;
		}

		switch (m_kind) {
			case I8:
			case I16:
			case I32:
			case I64:
			case CHAR:
				return true;
			default:
				return false;
		}
	}

	auto type::is_struct() const -> bool {
		return m_kind == STRUCT;
	}

	auto type::is_unresolved() const -> bool {
		return m_kind == UNRESOLVED;
	}

	auto type::is_unknown() const -> bool {
		return m_kind == UNKNOWN;
	}

	auto type::is_promote() const -> bool {
		return m_kind == VAR_ARG_PROMOTE;
	}
} // namespace sigma
