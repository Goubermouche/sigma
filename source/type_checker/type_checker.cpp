#include "type_checker.h"

#include <intermediate_representation/target/system/win/win.h>
#include <compiler/compiler/compilation_context.h>
#include <compiler/compiler/diagnostics.h>
#include <utility/string_helper.h>

namespace sigma {
	auto type_checker::type_check(backend_context& context) -> utility::result<void> {
		return type_checker(context).type_check();
	}

	type_checker::type_checker(backend_context& context) : m_context(context) {}

	auto type_checker::type_check() -> utility::result<void> {
		for (const handle<node>& top_level : m_context.ast.get_nodes()) {
			TRY(type_check_node(top_level));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_node(handle<node> ast_node, data_type expected) -> utility::result<data_type> {
		// map type check functions to node types
		using type_check_function = utility::result<data_type>(type_checker::*)(handle<node>, data_type);
		static std::unordered_map<node_type::underlying, type_check_function> s_checkers = {
			// functions
			{ node_type::FUNCTION_DECLARATION,  &type_checker::type_check_function_declaration  },
			{ node_type::FUNCTION_CALL,         &type_checker::type_check_function_call         },
			{ node_type::NAMESPACE_DECLARATION, &type_checker::type_check_namespace_declaration },

			// control flow
			{ node_type::RETURN,                &type_checker::type_check_return                },
			{ node_type::CONDITIONAL_BRANCH,    &type_checker::type_check_conditional_branch    },
			{ node_type::BRANCH,                &type_checker::type_check_branch                },

			// variables
			{ node_type::VARIABLE_DECLARATION,  &type_checker::type_check_variable_declaration  },
			{ node_type::VARIABLE_ACCESS,       &type_checker::type_check_variable_access       },
			{ node_type::VARIABLE_ASSIGNMENT,   &type_checker::type_check_variable_assignment   },

			// binary operators
			{ node_type::OPERATOR_ADD,          &type_checker::type_check_binary_math_operator  },
			{ node_type::OPERATOR_SUBTRACT,     &type_checker::type_check_binary_math_operator  },
			{ node_type::OPERATOR_MULTIPLY,     &type_checker::type_check_binary_math_operator  },
			{ node_type::OPERATOR_DIVIDE,       &type_checker::type_check_binary_math_operator  },
			{ node_type::OPERATOR_MODULO,       &type_checker::type_check_binary_math_operator  },

			// literals
			{ node_type::NUMERICAL_LITERAL,     &type_checker::type_check_numerical_literal     },
			{ node_type::CHARACTER_LITERAL,     &type_checker::type_check_character_literal     },
			{ node_type::STRING_LITERAL,        &type_checker::type_check_string_literal        },
			{ node_type::BOOL_LITERAL,          &type_checker::type_check_bool_literal          },
		};

		// locate the relevant type check function
		const auto it = s_checkers.find(ast_node->type);
		ASSERT(it != s_checkers.end(), "unhandled node type detected");

		// run the relevant function
		return (this->*it->second)(ast_node, expected);
	}

	auto type_checker::type_check_namespace_declaration(handle<node> variable_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		const ast_namespace& namespace_scope = variable_node->get<ast_namespace>();
		m_context.semantics.push_namespace(namespace_scope.identifier_key);

		for(const handle<node> statement : variable_node->children) {
			TRY(type_check_node(statement));
		}

		m_context.semantics.pop_scope();
		return data_type();
	}

	auto type_checker::type_check_function_declaration(handle<node> function_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		const ast_function& function = function_node->get<ast_function>();

		// check if the function hasn't been declared before
		if(m_context.semantics.contains_function(function.signature)) {
			const std::string& identifier = m_context.strings.get(function.signature.identifier_key);
			return error::emit(error::code::FUNCTION_ALREADY_DECLARED, function_node->location, identifier);
		}

		// register the function
		m_context.semantics.pre_declare_local_function(function.signature);
		m_context.semantics.push_scope();

		// push temporaries for function parameters
		for(const named_data_type& parameter : function.signature.parameter_types) {
			auto& variable = m_context.semantics.pre_declare_variable(parameter.identifier_key, parameter.type);
			variable.flags |= variable::FUNCTION_PARAMETER | variable::LOCAL;
		}

		// type check inner statements
		for(const handle<node>& statement : function_node->children) {
			TRY(type_check_node(statement, {}));
		}

		m_context.semantics.pop_scope();

		// this value won't be used
		return data_type();
	}

	auto type_checker::type_check_variable_declaration(handle<node> variable_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		const ast_variable& variable = variable_node->get<ast_variable>();

		// we cannot declare purely 'void' variables
		if(variable.type.is_void()) {
			const std::string& identifier_str = m_context.strings.get(variable.identifier_key);
			return error::emit(error::code::VOID_VARIABLE, variable_node->location, identifier_str);
		}

		// check, whether the variable has already been declared in the current context
		if(m_context.semantics.contains_variable(variable.identifier_key)) {
			const std::string& identifier_str = m_context.strings.get(variable.identifier_key);
			return error::emit(error::code::VARIABLE_ALREADY_DECLARED, variable_node->location, identifier_str);
		}

		// register the variable
		auto& declaration = m_context.semantics.pre_declare_variable(variable.identifier_key, variable.type);
		declaration.flags |= variable::LOCAL;

		// type check the assigned value
		if (variable_node->children.get_size() == 1) {
			TRY(type_check_node(variable_node->children[0], variable.type));
		}

		// this value won't be used
		return data_type();
	}

	auto type_checker::type_check_function_call(handle<node> call_node, data_type expected) -> utility::result<data_type> {
		std::vector<data_type> parameter_data_types(call_node->children.get_size());
		ast_function_call& function = call_node->get<ast_function_call>();

		// type check all parameters and store their inherent type
		for(u64 i = 0; i < parameter_data_types.size(); ++i) {
			TRY(parameter_data_types[i], type_check_node(call_node->children[i]));
			ASSERT(parameter_data_types[i].base_type != data_type::UNKNOWN, "unknown parameter type detected");
		}

		// at this point the function signature is empty, we gotta find a valid one
		TRY(function.signature, m_context.semantics.create_callee_signature(call_node, parameter_data_types));

		u64 i = 0;
		for(; i < function.signature.parameter_types.get_size(); ++i) {
			implicit_type_cast(parameter_data_types[i], function.signature.parameter_types[i].type, call_node->children[i]);

			// TRY(type_check_node(call_node->children[i], function.signature.parameter_types[i].type));
		}

		// type check var args
		for(; i < call_node->children.get_size(); ++i) {
			// TRY(type_check_node(call_node->children[i], { data_type::VAR_ARG_PROMOTE, 0 }));
			implicit_type_cast(parameter_data_types[i], { data_type::VAR_ARG_PROMOTE, 0 }, call_node->children[i]);
		}

		ASSERT(i == call_node->children.get_size(), "invalid parameter count");

		// pass the return type along
		TRY(const data_type result_type, implicit_type_cast(function.signature.return_type, expected, call_node));
		return result_type;
	}

	auto type_checker::type_check_return(handle<node> return_node, data_type expected) -> utility::result<data_type> {
		if (return_node->children.get_size() == 0) {
			// return an empty
			// verify that the parent function expects an empty return type
			if(expected != data_type(data_type::VOID, 0)) {
				return error::emit(error::code::VOID_RETURN, return_node->location, expected.to_string());
			}
		}
		else {
			// return a value
			TRY(type_check_node(return_node->children[0], expected));
		}

		// this value won't be used
		return data_type();
	}

	auto type_checker::type_check_conditional_branch(handle<node> branch_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		// type check the condition
		TRY(type_check_node(branch_node->children[0], data_type(data_type::BOOL, 0)));

		// if children[1] exists, we have another branch node
		if (branch_node->children[1]) {
			// type check another conditional branch
			if (branch_node->children[1]->type == node_type::CONDITIONAL_BRANCH) {
				TRY(type_check_conditional_branch(branch_node->children[1], expected));
			}
			// type check a regular branch
			else if (branch_node->children[1]->type == node_type::BRANCH) {
				TRY(type_check_branch(branch_node->children[1], expected));
			}
			else {
				PANIC("unexpected node type"); // unreachable
			}
		}

		// type check inner statements
		m_context.semantics.push_scope();

		for (u64 i = 2; i < branch_node->children.get_size(); ++i) {
			TRY(type_check_node(branch_node->children[i], {}));
		}

		m_context.semantics.pop_scope();
		// this value won't be used
		return data_type();
	}

	auto type_checker::type_check_branch(handle<node> branch_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		m_context.semantics.push_scope();

		// just type check all inner statements
		for (const handle<node>& statement : branch_node->children) {
			TRY(type_check_node(statement, {}));
		}

		m_context.semantics.pop_scope();
		// this value won't be used
		return data_type();
	}

	auto type_checker::type_check_binary_math_operator(handle<node> operator_node, data_type expected) -> utility::result<data_type> {
		// type check both operands
		TRY(const data_type left, type_check_node(operator_node->children[0], expected));

		// TODO: maybe expected should be == left?
		TRY(type_check_node(operator_node->children[1], expected));
		return left;
	}

	auto type_checker::type_check_numerical_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type> {
		auto& literal = literal_node->get<ast_literal>();
		literal.type = inherent_type_cast(literal.type, expected);

		const std::string& value_str = m_context.strings.get(literal.value_key);
		bool overflow = false;

		// check for type overflow
		switch (literal.type.base_type) {
			case data_type::I8: {
				const auto value = utility::detail::from_string<i8>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "i8"); }
				break;
			}
			case data_type::I16: {
				const auto value = utility::detail::from_string<i16>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "i16"); }
				break;
			}
			case data_type::I32: {
				const auto value = utility::detail::from_string<i32>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "i32"); }
				break;
			}
			case data_type::I64: {
				const auto value = utility::detail::from_string<i64>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "i64"); }
				break;
			}
			case data_type::U8: {
				const auto value = utility::detail::from_string<u8>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "u8"); }
				break;
			}
			case data_type::U16: {
				const auto value = utility::detail::from_string<u16>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "u16"); }
				break;
			}
			case data_type::U32: {
				const auto value = utility::detail::from_string<u32>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "u32"); }
				break;
			}
			case data_type::U64: {
				const auto value = utility::detail::from_string<u64>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "u64"); }
				break;
			}
			default: NOT_IMPLEMENTED();
		}

		return literal.type;
	}

	auto type_checker::type_check_character_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type> {
		auto& literal = literal_node->get<ast_literal>();
		TRY(literal.type, implicit_type_cast(literal.type, expected, literal_node));
		return literal.type;
	}

	auto type_checker::type_check_string_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type> {
		auto& literal = literal_node->get<ast_literal>();
		TRY(literal.type, implicit_type_cast(literal.type, expected, literal_node));
		return literal.type;
	}

	auto type_checker::type_check_bool_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(literal_node);
		SUPPRESS_C4100(expected);

		return data_type(data_type::BOOL, 0);
	}

	auto type_checker::inherent_type_cast(data_type original_type, data_type target_type) -> data_type {

		if(target_type.is_unknown()) {
			return original_type;
		}

		return target_type;
	}

	auto type_checker::implicit_type_cast(data_type original_type, data_type target_type, handle<node> original) const -> utility::result<data_type> {
		if(target_type.is_unknown()) {
			return original_type;
		}

		if(target_type.is_promote()) {
			// promote the original type
			target_type = promote_type(original_type);
			// use the new promoted type and (up)cast to it
		}

		// no cast needed
		if(original_type == target_type) {
			return original_type;
		}

		// don't cast pointers implicitly
		if(original_type.is_pointer() || target_type.is_pointer()) {
			return error::emit(error::code::INVALID_IMPLICIT_CAST, original->location, original_type.to_string(), target_type.to_string());
		}

		// target type is known at this point, try to cast to it
		const u16 original_byte_width = original_type.get_byte_width();
		const u16 target_byte_width = target_type.get_byte_width();

		// no cast needed, neither type is a pointer, just return
		if(original_byte_width == target_byte_width) {
			return original_type;
		}

		ASSERT(original->parent, "parent is nulltr");

		// insert a cast node between the original node and its parent node
		//  parent         parent
		//    |              |
		//    |      -->    cast
		//    |              |
		// original     	original

		const bool truncate = original_byte_width > target_byte_width;
		const handle<node> cast_node = m_context.ast.create_node<ast_cast>(truncate ? node_type::CAST_TRUNCATE : node_type::CAST_EXTEND, 1);
		ast_cast& cast = cast_node->get<ast_cast>();
		cast.original_type = original_type;
		cast.target_type = target_type;

		const handle<node> parent = original->parent;

		// find the 'original' node in the parent

		u64 index_in_parent = 0;
		for(; index_in_parent < parent->children.get_size(); index_in_parent++) {
			if(parent->children[index_in_parent] == original) {
				break;
			}
		}

		// replace this node by the relevant cast
		parent->children[index_in_parent] = cast_node;
		cast_node->children[0] = original;

		warning::emit(
			truncate ? warning::code::IMPLICIT_TRUNCATION_CAST : warning::code::IMPLICIT_EXTENSION_CAST,
			original->location,
			original_type.to_string(),
			target_type.to_string()
		);

		return target_type;
	}

	auto type_checker::type_check_variable_access(handle<node> access_node, data_type expected) -> utility::result<data_type> {
		auto& accessed_variable = access_node->get<ast_variable>();

		// locate the variable
		TRY(const auto accessed, m_context.semantics.find_variable(accessed_variable.identifier_key));

		if(accessed == nullptr) {
			const std::string& identifier_str = m_context.strings.get(accessed_variable.identifier_key);
			return error::emit(error::code::UNKNOWN_VARIABLE, access_node->location, identifier_str);
		}

		// default to the declared type
		TRY(accessed_variable.type, implicit_type_cast(accessed->type, expected, access_node));
		return accessed_variable.type; // return the type checked value
	}

	auto type_checker::type_check_variable_assignment(handle<node> assignment_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);

		// locate the variable
		const handle<node> variable_node = assignment_node->children[0];
		const auto& variable = variable_node->get<ast_variable>();

		TRY(const auto declaration, m_context.semantics.find_variable(variable.identifier_key));

		if (declaration == nullptr) {
			const std::string& identifier_str = m_context.strings.get(variable.identifier_key);
			return error::emit(error::code::UNKNOWN_VARIABLE_ASSIGN, variable_node->location, identifier_str);
		}

		// type check the assigned value against the declared type
		TRY(type_check_node(assignment_node->children[1], declaration->type));

		// this value won't be used
		return data_type();
	}

	auto promote_type(data_type type) -> data_type {
		if (type.is_pointer()) {
			return type; // don't promote pointers
		}

		switch (type.base_type) {
			case data_type::VOID: PANIC("cannot deref a void*");
			case data_type::I8:
			case data_type::I16:
			case data_type::U8:
			case data_type::U16:
			case data_type::BOOL:
			case data_type::CHAR: return data_type::create_i32();
			case data_type::I32:
			case data_type::I64:
			case data_type::U32:
			case data_type::U64:  return type;

			default: NOT_IMPLEMENTED();
		}

		// unreachable
		return {}; 
	}
} // namespace sigma
