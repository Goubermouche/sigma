#pragma once
#include <compiler/compiler/type_system/namespace_list.h>
#include <compiler/compiler/type_system/data_type.h>

#include "utility/containers/property.h"
#include "utility/containers/handle.h"
#include "utility/containers/slice.h"

namespace sigma::ast {
	struct node_type {
		enum underlying : u16 {
			UNKNOWN,

			// children[0] = assigned value, if there is one
			VARIABLE_DECLARATION,
			FUNCTION_DECLARATION,
			NAMESPACE_DECLARATION,
			STRUCT_DECLARATION,

			FUNCTION_CALL,
			RETURN,

			ALIGNOF,
			SIZEOF,

			// children[1 ... n] = statements
			BRANCH,
			// children[0] = condition
			// children[1 ... n] = statements
			// whenever children[1] == nullptr, the branch goes back to the source control, otherwise
			// it continues to the branch at children[1]
			CONDITIONAL_BRANCH,

			// storage[index expressions]
			// children[0] = storage
			// children[1 ... n] = index expressions
			ARRAY_ACCESS,
			VARIABLE_ACCESS,
			LOCAL_MEMBER_ACCESS,

			// storage = assigned value
			// children[0] = storage
			// children[1] = assigned value
			STORE,
			// children[0] = value to load
			LOAD,

			OPERATOR_ADD,
			OPERATOR_SUBTRACT,
			OPERATOR_MULTIPLY,
			OPERATOR_DIVIDE,
			OPERATOR_MODULO,

			OPERATOR_CONJUNCTION,
			OPERATOR_DISJUNCTION,

			OPERATOR_GREATER_THAN,
			OPERATOR_LESS_THAN,
			OPERATOR_GREATER_THAN_OR_EQUAL,
			OPERATOR_LESS_THAN_OR_EQUAL,
			OPERATOR_EQUAL,
			OPERATOR_NOT_EQUAL,

			OPERATOR_LOGICAL_NOT,

			// cast
			// children[0] = value
			CAST,

			NUMERICAL_LITERAL,
			CHARACTER_LITERAL,
			STRING_LITERAL,
			BOOL_LITERAL,
			NULL_LITERAL
		};

		node_type() = default;
		node_type(underlying type);

		auto to_string() const -> std::string;
		operator underlying() const;

		underlying type;
	};

	struct bool_literal {
		bool value;
	};

	struct named_expression {
		// NAMESPACE:          name of the namespace
		utility::string_table_key key;
	};

	struct comparison_expression {
		// OPERATOR_GREATER_THAN
		// OPERATOR_LESS_THAN
		// OPERATOR_GREATER_THAN_OR_EQUAL
		// OPERATOR_LESS_THAN_OR_EQUAL
		// OPERATOR_EQUAL
		// OPERATOR_NOT_EQUAL

		enum class type : u8 {
			INTEGRAL_SIGNED,
			INTEGRAL_UNSIGNED,
			FLOATING_POINT,
			POINTER
		};

		type type;
	};

	struct type_expression {
		// ARRAY_ACCESS: type at the 0'th index of the array
		// ALIGNOF:      type we want to know the alignment of
		// SIZEOF:       type we want to know the size of
		// LOAD:         type of the value we're loading
		data_type type;
	};

	struct named_type_expression {
		// NUMERICAL_LITERAL:    key stores the literal value, type the specific type
		// CHARACTER_LITERAL:    key stores the literal value, type holds 'char'
		// STRING_LITERAL:       key stores the literal value, type holds 'char*'
		// VARIABLE_DECLARATION: key stores the identifier, type holds the declared type
		// VARIABLE_ACCESS:      key stores the identifier, type is resolved in the type checker
		//                       and holds the type of the accessed variable
		// STRUCT_DECLARATION:   key holds the name of the structure, type describes the struct
		// STORE:
		// LOCAL_MEMBER_ACCESS:
		utility::string_table_key key;
		data_type type;
	};

	struct cast {
		// casts original_type to target_type
		data_type original_type;
		data_type target_type;
	};

	struct function {
		// FUNCTION_DECLARATION: signature of the declared function, constructed by the parser
		function_signature signature;
	};

	struct function_call {
		// FUNCTION_CALL
		// signature of the called function, resolved by the type checker
		function_signature signature;
		// optional namespace directives, empty by default
		// ie. utility::detail::call() would be { utility, detail }
		namespace_list namespaces;
	};

	using node_properties = utility::property<
		named_type_expression,
		comparison_expression,
		named_expression,
		type_expression,
		function_call,
		bool_literal,
		function,
		cast
	>;

	struct node : node_properties {
		auto is_branch() const -> bool;

		utility::slice<handle<node>, u16> children;

		// metadata
		node_type type;
		handle<token_location> location = nullptr;
	};
} // namespace sigma::ast
