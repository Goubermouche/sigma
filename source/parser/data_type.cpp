#include "data_type.h"
#include <utility/macros.h>

namespace sigma {
	data_type::data_type(token token, u8 pointer_level)
		: base_type(token_to_type(token)), pointer_level(pointer_level) {}

	data_type::data_type(data_type_base type, u8 pointer_level)
		: base_type(type), pointer_level(pointer_level) {}

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
			return sizeof(void*);
		}

		switch (base_type) {
			case UNKNOWN: return 0;
			case VOID:    return 0;
			case I8:      return sizeof(i8);
			case I16:     return sizeof(i16);
			case I32:     return sizeof(i32);
			case I64:     return sizeof(i64);
			case U8:      return sizeof(u8);
			case U16:     return sizeof(u16);
			case U32:     return sizeof(u32);
			case U64:     return sizeof(u64);
			case BOOL:    return 4;
			case CHAR:    return sizeof(char);
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

  named_data_type::named_data_type(data_type type, utility::string_table_key identifier_key)
	  : type(type), identifier_key(identifier_key) {}

	bool named_data_type::operator==(const named_data_type& other) const {
		return type == other.type && identifier_key == other.identifier_key;
	}
} // sigma::parse
