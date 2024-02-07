#include "token.h"
#include <utility/macros.h>

namespace sigma {
	token::token(token_type type) : type(type) {}

	auto token::is_type() const -> bool {
		switch (type) {
		case token_type::I8:
		case token_type::I16:
		case token_type::I32:
		case token_type::I64:
		case token_type::U8:
		case token_type::U16:
		case token_type::U32:
		case token_type::U64:
		case token_type::BOOL:
		case token_type::VOID:
		case token_type::CHAR:
			return true;
		default:
			return false;
		}
	}

	auto token::is_numerical_literal() const -> bool {
		switch (type) {
		case token_type::SIGNED_LITERAL:
		case token_type::UNSIGNED_LITERAL:
		case token_type::F32_LITERAL:
		case token_type::F64_LITERAL:
		case token_type::HEXADECIMAL_LITERAL:
		case token_type::BINARY_LITERAL:
			return true;
		default:
			return false;
		}
	}

	auto token::to_string() const -> std::string {
		switch (type) {
			case token_type::UNKNOWN:               return "UNKNOWN";
			case token_type::LEFT_PARENTHESIS:      return "(";
			case token_type::RIGHT_PARENTHESIS:     return ")";
			case token_type::LEFT_BRACE:            return "{";
			case token_type::RIGHT_BRACE:           return "}";
			case token_type::LEFT_BRACKET:          return "[";
			case token_type::RIGHT_BRACKET:         return "]";
			case token_type::COMMA:                 return ",";
			case token_type::SEMICOLON:             return ";";
			case token_type::SINGLE_QUOTE:          return "'";
			case token_type::DOUBLE_QUOTE:          return "\"";
			case token_type::MODULO:                return "%";
			case token_type::SLASH:                 return "/";
			case token_type::ASTERISK:              return "*";
			case token_type::PLUS_SIGN:             return "+";
			case token_type::MINUS_SIGN:            return "-";
			case token_type::EQUALS_SIGN:           return "=";
			case token_type::COLON:                 return ":";
			case token_type::LESS_THAN:             return "<";
			case token_type::GREATER_THAN:          return ">";
			case token_type::EXCLAMATION_MARK:      return "!";

			case token_type::LESS_THAN_OR_EQUAL:    return "<=";
			case token_type::GREATER_THAN_OR_EQUAL: return ">=";
			case token_type::EQUALS:                return "==";
			case token_type::NOT_EQUALS:            return "!=";
			case token_type::CONJUNCTION:           return "&&";
			case token_type::DISJUNCTION:           return "||";
			case token_type::INLINE_COMMENT:        return "//";

			// default type keywords
			case token_type::I8:                    return "I8";
			case token_type::I16:                   return "I16";
			case token_type::I32:                   return "I32";
			case token_type::I64:                   return "I64";

			case token_type::U8:                    return "U8";
			case token_type::U16:                   return "U16";
			case token_type::U32:                   return "U32";
			case token_type::U64:                   return "U64";

			case token_type::BOOL:                  return "BOOL";
			case token_type::VOID:                  return "VOID";
			case token_type::CHAR:                  return "CHAR";
			case token_type::STRUCT:                return "STRUCT";

			// control flow
			case token_type::RET:                   return "RET";
			case token_type::IF:                    return "IF";
			case token_type::ELSE:                  return "ELSE";

			// other keywords
			case token_type::NAMESPACE:             return "NAMESPACE";
			case token_type::CAST:								  return "CAST";
			case token_type::SIZEOF:						    return "SIZEOF";
			case token_type::ALIGNOF:						    return "ALIGNOF";

			// literals
			case token_type::SIGNED_LITERAL:        return "SIGNED_LITERAL";
			case token_type::UNSIGNED_LITERAL:      return "UNSIGNED_LITERAL";
			case token_type::F32_LITERAL:           return "F32_LITERAL";
			case token_type::F64_LITERAL:           return "F64_LITERAL";
			case token_type::HEXADECIMAL_LITERAL:   return "HEXADECIMAL_LITERAL";
			case token_type::BINARY_LITERAL:        return "BINARY_LITERAL";
			case token_type::STRING_LITERAL:        return "STRING_LITERAL";
			case token_type::CHARACTER_LITERAL:     return "CHARACTER_LITERAL";
			case token_type::BOOL_LITERAL_TRUE:     return "BOOL_LITERAL_TRUE";
			case token_type::BOOL_LITERAL_FALSE:    return "BOOL_LITERAL_FALSE";
			case token_type::NULL_LITERAL:					return "null";

			case token_type::IDENTIFIER:            return "IDENTIFIER";
			case token_type::END_OF_FILE:           return "EOF";
		}

		NOT_IMPLEMENTED();
		return "ERROR";
	}

	auto token::operator==(token other) const -> bool {
		return type == other.type;
	}

	auto token::operator==(token_type other) const -> bool {
		return type == other;
	}

	token::operator token_type() const {
		return type;
	}

} // sigma::lex
