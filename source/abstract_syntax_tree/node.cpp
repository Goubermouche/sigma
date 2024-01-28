#include "node.h"
#include "utility/macros.h"

namespace sigma::ast {
	node_type::node_type(underlying type) : type(type) {}

	auto node_type::to_string() const->std::string {
		switch(type) {
			case UNKNOWN:               return "UNKNOWN";

			case VARIABLE_DECLARATION:  return "VARIABLE_DECLARATION";
			case FUNCTION_DECLARATION:  return "FUNCTION_DECLARATION";
			case NAMESPACE_DECLARATION: return "NAMESPACE_DECLARATION";

			case FUNCTION_CALL:         return "FUNCTION_CALL";
			case RETURN:                return "RETURN";

			case SIZEOF:                return "SIZEOF";

			case BRANCH:                return "BRANCH";
			case CONDITIONAL_BRANCH:    return "CONDITIONAL_BRANCH";

			case ARRAY_ACCESS:          return "ARRAY_ACCESS";
			case VARIABLE_ACCESS:       return "VARIABLE_ACCESS";

			case STORE:                 return "STORE";
			case LOAD:                  return "LOAD";

			case OPERATOR_ADD:          return "OPERATOR_ADD";
			case OPERATOR_SUBTRACT:     return "OPERATOR_SUBTRACT";
			case OPERATOR_MULTIPLY:     return "OPERATOR_MULTIPLY";
			case OPERATOR_DIVIDE:       return "OPERATOR_DIVIDE";
			case OPERATOR_MODULO:       return "OPERATOR_MODULO";

			case CAST:                  return "CAST";

			case NUMERICAL_LITERAL:     return "NUMERICAL_LITERAL";
			case CHARACTER_LITERAL:     return "CHARACTER_LITERAL";
			case STRING_LITERAL:        return "STRING_LITERAL";
			case BOOL_LITERAL:          return "BOOL_LITERAL";
			case NULL_LITERAL:          return "NULL_LITERAL";

			default: PANIC("to_string() not implemented for node '{}'", static_cast<u16>(type));
		}

		return std::string();
	}

	node_type::operator underlying() const {
		return type;
	}

  auto node::is_branch() const -> bool {
    return type == node_type::BRANCH || type == node_type::CONDITIONAL_BRANCH;
  }
} // namespace sigma::ast
