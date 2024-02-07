#include "data_type.h"
#include <utility/macros.h>

namespace sigma {
	data_type::data_type(token token, u8 pointer_level)
		: base_type(token_to_type(token)), pointer_level(pointer_level) {}

	data_type::data_type(data_type_base type, u8 pointer_level)
		: base_type(type), pointer_level(pointer_level) {}

	auto data_type::create_unknown() -> data_type {
		return {};
	}

  auto data_type::create_var_arg_promote() -> data_type {
		return { VAR_ARG_PROMOTE, 0 };
  }

  auto data_type::create_bool(u8 pointer_level) -> data_type {
		return { BOOL, pointer_level };
  }

	auto data_type::create_i8(u8 pointer_level) -> data_type {
		return { I8, pointer_level };
	}

	auto data_type::create_i16(u8 pointer_level) -> data_type {
		return { I16, pointer_level };
	}

	auto data_type::create_i32(u8 pointer_level) -> data_type {
		return { I32, pointer_level };
	}

	auto data_type::create_i64(u8 pointer_level) -> data_type {
		return { I64, pointer_level };
	}

	auto data_type::create_u8(u8 pointer_level) -> data_type {
		return { U8, pointer_level };
	}

	auto data_type::create_u16(u8 pointer_level) -> data_type {
		return { U16, pointer_level };
	}

	auto data_type::create_u32(u8 pointer_level) -> data_type {
		return { U32, pointer_level };
	}

	auto data_type::create_u64(u8 pointer_level) -> data_type {
		return { U64, pointer_level };
	}

	auto data_type::create_access(u8 level) const -> data_type {
		ASSERT(pointer_level - level >= 0, "cannot create a negative pointer level");
		const u8 new_pointer_level = pointer_level - level;
		return { base_type, new_pointer_level };
	}

	bool data_type::operator==(data_type other) const {
		return base_type == other.base_type && pointer_level == other.pointer_level;
	}

  bool data_type::operator<(data_type other) const {
		if (pointer_level < other.pointer_level) {
			return true;
		}

		if (pointer_level > other.pointer_level) {
			return false;
		}

		return base_type < other.base_type;
  }

	auto data_type::to_string() const->std::string {
		std::string result;

		switch(base_type) {
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
			case STRUCT:          result = "struct"; break;
			default: NOT_IMPLEMENTED();
		}

		return result + std::string(pointer_level, '*');
	}

	auto data_type::get_alignment() const -> u16 {
		if (pointer_level > 0) {
			return 8;
		}

		switch (base_type) {
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

				for (const auto& member : members) {
					max_alignment = std::max(max_alignment, member.get_alignment());
				}

				return max_alignment;
			}
			default: PANIC("undefined byte width for type '{}'", to_string());
		}

		return 0;
	}

	auto data_type::get_size() const -> u16 {
		if(pointer_level > 0) {
			return 8;
		}

		switch (base_type) {
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

				for (const auto& member : members) {
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
			default: PANIC("undefined byte width for type '{}'", to_string());
		}

		return 0;
	}

	auto data_type::token_to_type(token token) -> data_type_base {
		switch (token.type) {
			case token_type::I8:   return I8;
			case token_type::I16:  return I16;
			case token_type::I32:  return I32;
			case token_type::I64:  return I64;
			case token_type::U8:   return U8;
			case token_type::U16:  return U16;
			case token_type::U32:  return U32;
			case token_type::U64:  return U64;
			case token_type::BOOL: return BOOL;
			case token_type::VOID: return VOID;
			case token_type::CHAR: return CHAR;
			default: PANIC("undefined token -> type conversion for token '{}'", token.to_string());
		}

		return UNKNOWN;
	}

  bool data_type::is_floating_point() const {
		if (pointer_level > 0) {
			return false;
		}

		return base_type == F32 || base_type == F64;
  }

  bool data_type::is_integral() const {
		if(pointer_level > 0) {
			return false;
		}

		switch (base_type) {
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

	bool data_type::is_void() const {
		return base_type == VOID && pointer_level == 0;
	}

  bool data_type::is_unknown() const {
		return base_type == UNKNOWN;
  }

	bool data_type::is_promote() const {
		return base_type == VAR_ARG_PROMOTE;
	}

  bool data_type::is_pointer() const {
		return pointer_level > 0;
  }

	bool data_type::is_void_pointer() const {
		return pointer_level > 0 && base_type == VOID;
	}

  bool data_type::is_signed() const {
		if (pointer_level > 0) {
			return false;
		}

		switch (base_type) {
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

	bool data_type::is_unsigned() const {
		if (pointer_level > 0) {
			return false;
		}

		switch (base_type) {
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

  named_data_type::named_data_type(data_type type, utility::string_table_key identifier_key)
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

		auto get_larger_type(data_type a, data_type b) -> data_type {
			if (
				a.base_type == data_type::UNKNOWN ||
				b.base_type == data_type::UNKNOWN ||
				a.base_type == data_type::VAR_ARG_PROMOTE ||
				b.base_type == data_type::VAR_ARG_PROMOTE
			) {
				return { data_type::UNKNOWN, 0 };
			}

			// consider pointers as automatically the larger/more prominent type
			if(a.is_pointer()) {
				return a;
			}

			if(b.is_pointer()) {
				return b;
			}

			if (a.base_type == b.base_type) {
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

		auto promote_type(data_type type) -> data_type {
			if (type.is_pointer()) {
				return type; // don't promote pointers
			}

			switch (type.base_type) {
				case data_type::VOID: PANIC("cannot dereference a void*");
				case data_type::I8:
				case data_type::I16:
				case data_type::U8:
				case data_type::U16:
				case data_type::BOOL:
				case data_type::CHAR: return data_type::create_i32();
				case data_type::I32:
				case data_type::I64:
				case data_type::U32:
				case data_type::U64:  return type;
				default: NOT_IMPLEMENTED();
			}

			// unreachable
			return {};
		}
	} // namespace detail
} // namespace sigma
