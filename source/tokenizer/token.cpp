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
		case token_type::BOOL:
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
			case token_type::UNKNOWN:             return "UNKNOWN";
			case token_type::LEFT_PARENTHESIS:    return "(";
			case token_type::RIGHT_PARENTHESIS:   return ")";
			case token_type::LEFT_BRACE:          return "{";
			case token_type::RIGHT_BRACE:         return "}";
			case token_type::COMMA:               return ",";
			case token_type::SEMICOLON:           return ";";
			case token_type::SINGLE_QUOTE:        return "'";
			case token_type::DOUBLE_QUOTE:        return "\"";
			case token_type::MODULO:              return "%";
			case token_type::SLASH:               return "/";
			case token_type::ASTERISK:            return "*";
			case token_type::PLUS_SIGN:           return "+";
			case token_type::MINUS_SIGN:          return "+";
			case token_type::EQUALS_SIGN:         return "=";
			case token_type::COLON:               return ":";

			// default type keywords
			case token_type::I8:                  return "I8";
			case token_type::I16:                 return "I16";
			case token_type::I32:                 return "I32";
			case token_type::I64:                 return "I64";

			case token_type::BOOL:                return "BOOL";

			// control flow
			case token_type::RET:                 return "RET";
			case token_type::IF:                  return "IF";
			case token_type::ELSE:                return "ELSE";

			// other keywords
			case token_type::NAMESPACE:           return "NAMESPACE";

			// literals
			case token_type::SIGNED_LITERAL:      return "SIGNED_LITERAL";
			case token_type::UNSIGNED_LITERAL:    return "UNSIGNED_LITERAL";
			case token_type::F32_LITERAL:         return "F32_LITERAL";
			case token_type::F64_LITERAL:         return "F64_LITERAL";
			case token_type::HEXADECIMAL_LITERAL: return "HEXADECIMAL_LITERAL";
			case token_type::BINARY_LITERAL:      return "BINARY_LITERAL";
			case token_type::STRING_LITERAL:      return "STRING_LITERAL";
			case token_type::BOOL_LITERAL_TRUE:   return "BOOL_LITERAL_TRUE";
			case token_type::BOOL_LITERAL_FALSE:  return "BOOL_LITERAL_FALSE";

			case token_type::IDENTIFIER:          return "IDENTIFIER";
			case token_type::END_OF_FILE:         return "EOF";
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
