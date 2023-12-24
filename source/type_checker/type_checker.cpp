#include "type_checker.h"
#include <compiler/compiler/compilation_context.h>

namespace sigma {
	auto type_checker::type_check(compilation_context& context) -> utility::result<void> {
		return type_checker(context).type_check();
	}

	type_checker::type_checker(compilation_context& context) : m_context(context) {
		// TODO: replace this by a global function manager

		{
			const utility::string_table_key printf_key = context.string_table.insert("printf");
			const utility::string_table_key format_key = context.string_table.insert("format");

			auto printf_params = utility::slice<named_data_type>(m_context.ast.get_allocator(), 1);
			printf_params[0] = named_data_type{ data_type(data_type::CHAR, 1), format_key };

			m_external_functions[printf_key] = function{
				.return_type = data_type(data_type::I32, 0),
				.parameter_types = printf_params,
				.has_var_args = true,
				.identifier_key = printf_key
			};
		}

		{
			const utility::string_table_key printf_key = context.string_table.insert("puts");
			const utility::string_table_key format_key = context.string_table.insert("str");

			auto printf_params = utility::slice<named_data_type>(m_context.ast.get_allocator(), 1);
			printf_params[0] = named_data_type{ data_type(data_type::CHAR, 1), format_key };

			m_external_functions[printf_key] = function{
				.return_type = data_type(data_type::I32, 0),
				.parameter_types = printf_params,
				.has_var_args = false,
				.identifier_key = printf_key
			};
		}
	}

	auto type_checker::type_check() -> utility::result<void> {
		for (const handle<node>& top_level : m_context.ast.get_nodes()) {
			TRY(type_check_node(top_level));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_node(handle<node> ast_node, data_type expected) -> utility::result<void> {
		// map type check functions to node types
		using type_check_function = utility::result<void>(type_checker::*)(handle<node>, data_type);
		static std::unordered_map<node_type::underlying, type_check_function> s_checkers = {
			// functions
			{ node_type::FUNCTION_DECLARATION, &type_checker::type_check_function             },
			{ node_type::FUNCTION_CALL,        &type_checker::type_check_function_call        },

			// control flow
			{ node_type::RETURN,               &type_checker::type_check_return               },
			{ node_type::CONDITIONAL_BRANCH,   &type_checker::type_check_conditional_branch   },
			{ node_type::BRANCH,               &type_checker::type_check_branch               },

			// variables
			{ node_type::VARIABLE_DECLARATION, &type_checker::type_check_variable_declaration },
			{ node_type::VARIABLE_ACCESS,      &type_checker::type_check_variable_access      },
			{ node_type::VARIABLE_ASSIGNMENT,  &type_checker::type_check_variable_assignment  },

			// binary operators
			{ node_type::OPERATOR_ADD,         &type_checker::type_check_binary_math_operator },
			{ node_type::OPERATOR_SUBTRACT,    &type_checker::type_check_binary_math_operator },
			{ node_type::OPERATOR_MULTIPLY,    &type_checker::type_check_binary_math_operator },
			{ node_type::OPERATOR_DIVIDE,      &type_checker::type_check_binary_math_operator },
			{ node_type::OPERATOR_MODULO,      &type_checker::type_check_binary_math_operator },

			// literals
			{ node_type::NUMERICAL_LITERAL,    &type_checker::type_check_numerical_literal    },
			{ node_type::STRING_LITERAL,       &type_checker::type_check_string_literal       },
			{ node_type::BOOL_LITERAL,         &type_checker::type_check_bool_literal         }
		};

		// locate the relevant type check function
		const auto it = s_checkers.find(ast_node->type);
		ASSERT(it != s_checkers.end(), "unhandled node type detected");

		// run the relevant function
		TRY((this->*it->second)(ast_node, expected));
		return SUCCESS;
	}

	auto type_checker::type_check_function(handle<node> function_node, data_type expected) -> utility::result<void> {
		SUPPRESS_C4100(expected);
		auto& property = function_node->get<function>();

		// register the function
		m_functions[property.identifier_key] = &property;

		// type check inner statements
		for (const handle<node>& statement : function_node->children) {
			TRY(type_check_node(statement, property.return_type));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_variable_declaration(handle<node> variable_node, data_type expected) -> utility::result<void> {
		SUPPRESS_C4100(expected);
		const auto& property = variable_node->get<variable>();

		// register the variable
		m_local_variables[property.identifier_key] = property.type;

		// type check the assigned value
		if (variable_node->children.get_size() == 1) {
			TRY(type_check_node(variable_node->children[0], property.type));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_function_call(handle<node> call_node, data_type expected) -> utility::result<void> {
		SUPPRESS_C4100(expected);
		auto& property = call_node->get<function_call>();

		// locate the callee
		handle<function> callee;
		const auto local_call = m_functions.find(property.callee_identifier_key);

		if (local_call == m_functions.end()) {
			const auto external_call = m_external_functions.find(property.callee_identifier_key);

			if(external_call == m_external_functions.end()) {
				return utility::error::create(utility::error::code::UNKNOWN_FUNCTION, m_context.string_table.get(property.callee_identifier_key));
			}

			callee = &external_call->second;
			property.is_external = true;
		}
		else {
			callee = local_call->second;
			property.is_external = false;
		}

		// verify the supplied parameter count
		// var args
		if(callee->has_var_args && call_node->children.get_size() < callee->parameter_types.get_size()) {
			return utility::error::create(
				utility::error::code::INVALID_FUNCTION_PARAMETER_COUNT,
				std::to_string(callee->parameter_types.get_size()) + "+", 
				call_node->children.get_size()
			);
		}

		// regular functions
		if(callee->parameter_types.get_size() != call_node->children.get_size()) {
			return utility::error::create(
				utility::error::code::INVALID_FUNCTION_PARAMETER_COUNT,
				callee->parameter_types.get_size(), 
				call_node->children.get_size()
			);
		}

		// type check regular function parameters (non-var arg)
		u64 i = 0;
		for (; i < callee->parameter_types.get_size(); ++i) {
			TRY(type_check_node(call_node->children[i], callee->parameter_types[i].type));
		}

		// type check var args
		// these should be promoted to an expected type:
		// -   bool -> i32
		// -   f32  -> f64
		for (; i < call_node->children.get_size(); ++i) {
			TRY(type_check_node(call_node->children[i], { data_type::VAR_ARG_PROMOTE, 0 }));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_return(handle<node> return_node, data_type expected) -> utility::result<void> {
		if (return_node->children.get_size() == 0) {
			// 'ret' - verify that the parent function expects an empty return type
			if(expected != data_type(data_type::VOID, 0)) {
				return utility::error::create(utility::error::code::VOID_RETURN, expected.to_string());
			}
		}
		else {
			// 'ret type'
			TRY(type_check_node(return_node->children[0], expected));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_conditional_branch(handle<node> branch_node, data_type expected) -> utility::result<void> {
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
		for (u64 i = 2; i < branch_node->children.get_size(); ++i) {
			TRY(type_check_node(branch_node->children[i], {}));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_branch(handle<node> branch_node, data_type expected) -> utility::result<void> {
		SUPPRESS_C4100(expected);
		
		// just type check all inner statements
		for (const handle<node>& statement : branch_node->children) {
			TRY(type_check_node(statement, {}));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_binary_math_operator(handle<node> operator_node, data_type expected) -> utility::result<void> {
		// type check both operands
		TRY(type_check_node(operator_node->children[0], expected));
		TRY(type_check_node(operator_node->children[1], expected));
		return SUCCESS;
	}

	auto type_checker::type_check_numerical_literal(handle<node> literal_node, data_type expected) -> utility::result<void> {
		auto& property = literal_node->get<literal>();
		apply_expected_data_type(property.type, expected);
		return SUCCESS;
	}

	auto type_checker::type_check_string_literal(handle<node> literal_node, data_type expected) -> utility::result<void> {
		auto& property = literal_node->get<literal>();
		apply_expected_data_type(property.type, expected);
		return SUCCESS;
	}

	auto type_checker::type_check_bool_literal(handle<node> literal_node, data_type expected) -> utility::result<void> {
		SUPPRESS_C4100(literal_node);
		const data_type boolean(data_type::BOOL, 0);

		// NOTE: we should probably check for the ability to upcast integers to booleans etc.
		if(expected != boolean) {
			return utility::error::create(utility::error::code::UNEXPECTED_TYPE, boolean.to_string(), expected.to_string());
		}

		return SUCCESS;
	}

	auto type_checker::type_check_variable_access(handle<node> access_node, data_type expected) -> utility::result<void> {
		auto& property = access_node->get<variable>();

		// locate the variable
		const auto it = m_local_variables.find(property.identifier_key);

		if(it == m_local_variables.end()) {
			return utility::error::create(utility::error::code::UNKNOWN_VARIABLE, m_context.string_table.get(property.identifier_key));
		}

		property.type = it->second; // default to the declared type
		apply_expected_data_type(property.type, expected);
		return SUCCESS;
	}

	auto type_checker::type_check_variable_assignment(handle<node> assignment_node, data_type expected) -> utility::result<void> {
		SUPPRESS_C4100(expected);

		// locate the variable
		const handle<node> variable_node = assignment_node->children[0];
		const auto& variable_property = variable_node->get<variable>();
		const auto it = m_local_variables.find(variable_property.identifier_key);

		if (it == m_local_variables.end()) {
			return utility::error::create(utility::error::code::UNKNOWN_VARIABLE, m_context.string_table.get(variable_property.identifier_key));
		}

		// type check the assigned value against the declared type
		TRY(type_check_node(assignment_node->children[1], it->second));
		return SUCCESS;
	}

	void type_checker::apply_expected_data_type(data_type& target, data_type source) {
		// basically a cast call

		if (source.base_type != data_type::UNKNOWN) {
			if (source.base_type == data_type::VAR_ARG_PROMOTE) {
				// promote the variable
				// don't promote pointers 
				if (target.pointer_level > 0) {
					return;
				}

				switch (target.base_type) {
					case data_type::UNKNOWN: PANIC("promotion on unknown data type"); break;
					case data_type::I32:     return;
					case data_type::BOOL:    target.base_type = data_type::I32; break;
					default: NOT_IMPLEMENTED();
				}
			}
			else {
				target = source;
			}
		}
	}
} // namespace sigma
