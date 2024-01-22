#include "data_type.h"
#include <utility/macros.h>

namespace sigma {
	data_type::data_type(token token, u8 pointer_level)
		: base_type(token_to_type(token)), pointer_level(pointer_level) {}

	data_type::data_type(data_type_base type, u8 pointer_level)
		: base_type(type), pointer_level(pointer_level) {}

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
		const static std::unordered_map<data_type_base, std::string> type_to_string_map = {
			{ UNKNOWN ,        "unknown" },
			{ VAR_ARG_PROMOTE, "promote" },
			{ VOID ,           "void"    },
			{ I8,              "i8"      },
			{ I16,             "i16"     },
			{ I32,             "i32"     },
			{ I64,             "i64"     },
			{ U8,              "u8"      },
			{ U16,             "u16"     },
			{ U32,             "u32"     },
			{ U64,             "u64"     },
			{ BOOL,            "bool"    },
			{ CHAR,            "char"    }
		};

		const auto it = type_to_string_map.find(base_type);

		ASSERT(
			it != type_to_string_map.end(),
			"cannot convert the given base type '{}' to a string",
			static_cast<u8>(base_type)
		);

		return it->second + std::string(pointer_level, '*');
	}

	auto data_type::get_byte_width() const -> u16 {
		if(pointer_level > 0) {
			return 8;
		}

		switch (base_type) {
			case UNKNOWN: return 0;
			case VOID:    return 0;
			case I8:      return 1;
			case I16:     return 2;
			case I32:     return 4;
			case I64:     return 8;
			case U8:      return 1;
			case U16:     return 2;
			case U32:     return 4;
			case U64:     return 8;
			case BOOL:    return 4;
			case CHAR:    return 1;
			case VAR_ARG_PROMOTE: return 0;
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

  bool data_type::is_integer() const {
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

  auto get_larger_type(data_type a, data_type b) -> data_type {
		if (a.base_type == data_type::UNKNOWN || b.base_type == data_type::UNKNOWN ||
			a.base_type == data_type::VAR_ARG_PROMOTE || b.base_type == data_type::VAR_ARG_PROMOTE) {
			return { data_type::UNKNOWN, 0 };
		}

		if (a.base_type == b.base_type) {
			return a; // same type
		}

		// Prioritize higher types in the hierarchy
		static const std::unordered_map<data_type::data_type_base, u8> type_priority = {
			{ data_type::VOID, 0 }, { data_type::BOOL, 1  }, { data_type::CHAR, 2 },
			{ data_type::I8,   3 }, { data_type::U8,   4  }, { data_type::I16,  5 },
			{ data_type::U16,  6 }, { data_type::I32,  7  }, { data_type::U32,  8 },
			{ data_type::I64,  9 }, { data_type::U64,  10 }
		};

		// Compare based on type hierarchy.
		const u8 priority_a = type_priority.at(a.base_type);
		const u8 priority_b = type_priority.at(b.base_type);

		// return the type with the higher priority.
		return priority_a > priority_b ? a : b;
  }
} // sigma::parse
