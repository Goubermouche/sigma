#pragma once
#include "parser/data_type.h"

#include "utility/containers/property.h"
#include "utility/containers/handle.h"
#include "utility/containers/slice.h"

namespace sigma {
	using namespace utility::types;

	struct node_type {
		enum underlying : u16 {
			UNKNOWN,

			FUNCTION_DECLARATION,
			FUNCTION_CALL,

			RETURN,

			// children[1 - n] = statements
			BRANCH,
			// children[0] = condition
			// children[1 - n] = statements
			// whenever children[1] == nullptr, the branch goes back to the source control, otherwise
			// it continues to the branch at children[1]
			CONDITIONAL_BRANCH,

			VARIABLE_DECLARATION,
			VARIABLE_ACCESS,
			// children[0] = variable
			// children[1] = assigned value
			VARIABLE_ASSIGNMENT,

			OPERATOR_ADD,
			OPERATOR_SUBTRACT,
			OPERATOR_MULTIPLY,
			OPERATOR_DIVIDE,
			OPERATOR_MODULO,

			NUMERICAL_LITERAL,
			STRING_LITERAL,
			BOOL_LITERAL
		};

		node_type() = default;
		node_type(underlying type);

		auto to_string() const -> std::string;
		operator underlying() const;

		underlying type;
	};

	struct node;

	struct function {
		data_type return_type;
		utility::slice<named_data_type> parameter_types;
		bool has_var_args = false;

		utility::string_table_key identifier_key;
	};

	struct function_call {
		utility::string_table_key callee_identifier_key;
		bool is_external;
	};

	struct return_statement {};

	struct literal {
		utility::string_table_key value_key; // literal value represented as a string
		data_type type;
	};

	struct bool_literal {
		bool value;
	};

	struct variable {
		utility::string_table_key identifier_key;
		data_type type;
	};

	using node_properties = utility::property<
		function, return_statement, literal, function_call, variable, bool_literal
	>;

	struct node : node_properties {
		node_type type;
		utility::slice<handle<node>> children;
	};
} // namespace sigma
