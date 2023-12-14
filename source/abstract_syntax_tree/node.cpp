#include "node.h"
#include "utility/macros.h"

namespace sigma {
	node_type::node_type(underlying type) : type(type) {}

	auto node_type::to_string() const->std::string {
		switch(type) {
			case UNKNOWN:              return "UNKNOWN";
			case FUNCTION:             return "FUNCTION";
			case FUNCTION_CALL:        return "FUNCTION_CALL";

			case RETURN:               return "RETURN";

			case BRANCH:               return "BRANCH";
			case CONDITIONAL_BRANCH:   return "CONDITIONAL_BRANCH";

			case VARIABLE_DECLARATION: return "VARIABLE_DECLARATION";
			case VARIABLE_ACCESS:      return "VARIABLE_ACCESS";
			case VARIABLE_ASSIGNMENT:  return "VARIABLE_ASSIGNMENT";

			case OPERATOR_ADD:         return "OPERATOR_ADD";
			case OPERATOR_SUBTRACT:    return "OPERATOR_SUBTRACT";
			case OPERATOR_MULTIPLY:    return "OPERATOR_MULTIPLY";
			case OPERATOR_DIVIDE:      return "OPERATOR_DIVIDE";
			case OPERATOR_MODULO:      return "OPERATOR_MODULO";

			case NUMERICAL_LITERAL:    return "NUMERICAL_LITERAL";
			case STRING_LITERAL:       return "STRING_LITERAL";
			case BOOL_LITERAL:         return "BOOL_LITERAL";

			default: NOT_IMPLEMENTED();
		}

		return std::string();
	}

	node_type::operator underlying() const {
		return type;
	}

} // sigma
