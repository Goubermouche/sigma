#pragma once
#include <parser/data_type.h>

#include <utility/containers/property.h>
#include <utility/containers/handle.h>
#include <utility/containers/slice.h>

namespace sigma {
	using namespace utility::types;

	struct node_type {
		enum underlying {
			UNKNOWN,

			FUNCTION,
			FUNCTION_CALL,

			RETURN,

			BRANCH,
			CONDITIONAL_BRANCH,

			VARIABLE_DECLARATION,
			VARIABLE_ACCESS,
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
		utility::symbol_table_key identifier_key;
		utility::slice<named_type> parameters;
		bool has_var_args = false;
	};

	struct function_call {
		utility::symbol_table_key callee_identifier_key;
		bool is_external;
	};

	struct return_statement {};

	struct literal {
		utility::symbol_table_key value_key;
		data_type data_type;
	};

	struct bool_literal {
		bool value;
	};

	struct variable {
		utility::symbol_table_key identifier_key;
		data_type data_type;
	};

	struct variable_access {
		utility::symbol_table_key identifier_key;
		data_type data_type;
	};

	// implementation details of specific node types:
	// CONDITIONAL_BRANCH:
	// -   children[0] = condition
	// -   children[1 - n] = statements
	// BRANCH
	// -   children[0 - n] = statements

	using node_properties = utility::property<
		function, return_statement, literal, function_call, variable, variable_access, bool_literal
	>;

	struct node : node_properties {
		node_type type;
		utility::slice<handle<node>> children;
	};
} // namespace sigma
