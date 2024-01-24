#include "node.h"
#include "utility/macros.h"

namespace sigma {
	node_type::node_type(underlying type) : type(type) {}

	auto node_type::to_string() const->std::string {
		switch(type) {
			case UNKNOWN:               return "UNKNOWN";
			case FUNCTION_DECLARATION:  return "FUNCTION_DECLARATION";
			case FUNCTION_CALL:         return "FUNCTION_CALL";

			case NAMESPACE_DECLARATION: return "NAMESPACE_DECLARATION";
			case SIZEOF:                return "SIZEOF";

			case RETURN:                return "RETURN";

			case BRANCH:                return "BRANCH";
			case CONDITIONAL_BRANCH:    return "CONDITIONAL_BRANCH";

			case VARIABLE_DECLARATION:  return "VARIABLE_DECLARATION";
			case VARIABLE_ACCESS:       return "VARIABLE_ACCESS";
			case VARIABLE_ASSIGNMENT:   return "VARIABLE_ASSIGNMENT";

			case OPERATOR_ADD:          return "OPERATOR_ADD";
			case OPERATOR_SUBTRACT:     return "OPERATOR_SUBTRACT";
			case OPERATOR_MULTIPLY:     return "OPERATOR_MULTIPLY";
			case OPERATOR_DIVIDE:       return "OPERATOR_DIVIDE";
			case OPERATOR_MODULO:       return "OPERATOR_MODULO";

			case EXPLICIT_CAST:         return "EXPLICIT_CAST";
			case CAST_EXTEND:           return "CAST_EXTEND";
			case CAST_TRUNCATE:         return "CAST_TRUNCATE";

			case NUMERICAL_LITERAL:     return "NUMERICAL_LITERAL";
			case CHARACTER_LITERAL:     return "CHARACTER_LITERAL";
			case STRING_LITERAL:        return "STRING_LITERAL";
			case BOOL_LITERAL:          return "BOOL_LITERAL";

			default: PANIC("to_string() not implemented for node '{}'", static_cast<u16>(type));
		}

		return "";
	}

	node_type::operator underlying() const {
		return type;
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

		if(parameter_types.get_size() < other.parameter_types.get_size()) { return true;  }
		if(parameter_types.get_size() > other.parameter_types.get_size()) { return false; }

		for (size_t i = 0; i < other.parameter_types.get_size(); ++i) {
			if(parameter_types[i].type < other.parameter_types[i].type) { return true;  }
			if(parameter_types[i].type < other.parameter_types[i].type) { return false; }
		}

		return has_var_args < other.has_var_args;
  }

  auto node::is_branch() const -> bool {
    return type == node_type::BRANCH || type == node_type::CONDITIONAL_BRANCH;
  }
} // sigma
