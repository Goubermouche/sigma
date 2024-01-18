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

			// function_signature
			FUNCTION_CALL,

			NAMESPACE_DECLARATION,

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

	// TODO: move this elsewhere
	// a higher-level function signature (contains_function info about custom types and generics)
	struct function_signature {
		bool operator==(const function_signature& other) const;
		bool operator<(const function_signature& other) const;

		data_type return_type;
		utility::slice<named_data_type> parameter_types;
		bool has_var_args = false;

		utility::string_table_key identifier_key;
	};

	struct ast_function {
		function_signature signature;
		handle<token_location> location; // debug location
	};

	struct ast_function_call {
		function_signature signature;
		handle<token_location> location; // debug location
		std::vector<utility::string_table_key> namespaces; // TEMP
	};

	struct ast_return {
		handle<token_location> location; // debug location
	};

	struct ast_literal {
		utility::string_table_key value_key; // literal value represented as a string
		data_type type;
	};

	struct ast_bool_literal {
		bool value;
	};

	struct ast_variable {
		data_type type;
		utility::string_table_key identifier_key;
		handle<token_location> location; // debug location
	};

	struct ast_namespace {
		utility::string_table_key identifier_key;
	};

	using node_properties = utility::property<
		ast_function, ast_function_call, ast_return, ast_literal, ast_variable, ast_bool_literal, ast_namespace
	>;

	struct node : node_properties {
		node_type type;
		utility::slice<handle<node>> children;
	};
} // namespace sigma
