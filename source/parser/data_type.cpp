#include "data_type.h"
#include <utility/macros.h>

namespace sigma {
	data_type::data_type(token token, u8 pointer_level)
		: type(token_to_type(token)), pointer_level(pointer_level) {}

	data_type::data_type(base type, u8 pointer_level)
		: type(type), pointer_level(pointer_level) {}

	auto data_type::operator==(data_type other) const -> bool {
		return type == other.type && pointer_level == other.pointer_level;
	}

	auto data_type::to_string() const->std::string {
		const static std::unordered_map<base, std::string> type_to_string_map = {
			{ UNKNOWN ,        "unknown" },
			{ VAR_ARG_PROMOTE, "promote" },
			{ VOID ,           "void"    },
			{ I8,              "i8"      },
			{ I16,             "i16"     },
			{ I32,             "i32"     },
			{ I64,             "i64"     },
			{ BOOL,            "bool"    },
			{ CHAR,            "char"    }
		};

		const auto it = type_to_string_map.find(type);
		ASSERT(it != type_to_string_map.end(), "cannot convert the given type to a string");
		return it->second + std::string(pointer_level, '*');
	}

	auto data_type::get_byte_width() const -> u16 {
		if(pointer_level > 0) {
			return sizeof(void*);
		}

		switch (type) {
			case UNKNOWN: return 0;
			case VOID:    return 0;
			case I8:      return sizeof(i8);
			case I16:     return sizeof(i16);
			case I32:     return sizeof(i32);
			case I64:     return sizeof(i64);
			case BOOL:    return 4;
			case CHAR:    return sizeof(char);
		}

		ASSERT(false, "not implemented");
	}

	auto data_type::token_to_type(token token) -> base {
		switch (token.type) {
			case token_type::I8:   return I8;
			case token_type::I16:  return I16;
			case token_type::I32:  return I32;
			case token_type::I64:  return I64;
			case token_type::BOOL: return BOOL;
			default: {
				ASSERT(false, "unhandked token to type conversion");
				return UNKNOWN;
			}
		}
	}
} // sigma::parse
