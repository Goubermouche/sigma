#include "type_checker.h"
#include <compiler/compiler/compilation_context.h>

#include "intermediate_representation/target/system/win/win.h"

namespace sigma {
	auto type_checker::type_check(backend_context& context) -> utility::result<void> {
		return type_checker(context).type_check();
	}

	type_checker::type_checker(backend_context& context) : m_context(context) {}

	auto type_checker::type_check() -> utility::result<void> {
		for (const handle<node>& top_level : m_context.syntax.ast.get_nodes()) {
			TRY(type_check_node(top_level));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_node(handle<node> ast_node, data_type expected) -> utility::result<data_type> {
		// map type check functions to node types
		using type_check_function = utility::result<data_type>(type_checker::*)(handle<node>, data_type);
		static std::unordered_map<node_type::underlying, type_check_function> s_checkers = {
			// functions
			{ node_type::FUNCTION_DECLARATION, &type_checker::type_check_function_declaration },
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
		return (this->*it->second)(ast_node, expected);
	}

	auto type_checker::type_check_function_declaration(handle<node> function_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		const auto& signature = function_node->get<function_signature>();

		// check if the function hasn't been declared before
		if(m_context.function_registry.contains_function(signature)) {
			return utility::error::create(
				utility::error::code::FUNCTION_ALREADY_DECLARED, 
				m_context.syntax.string_table.get(signature.identifier_key)
			);
		}

		// register the function
		m_context.function_registry.pre_declare_local_function(signature);
		m_context.variable_registry.push_scope();

		// TODO: handle varargs
		// push temporaries for function parameters
		for(const named_data_type& parameter : signature.parameter_types) {
			m_context.variable_registry.pre_declare_variable(parameter.identifier_key, parameter.type);
		}

		// type check inner statements
		for(const handle<node>& statement : function_node->children) {
			TRY(type_check_node(statement, signature.return_type));
		}

		m_context.variable_registry.pop_scope();

		return SUCCESS;
	}

	auto type_checker::type_check_variable_declaration(handle<node> variable_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		const auto& property = variable_node->get<variable>();

		// check, whether the variable has already been declared in the current context
		if(m_context.variable_registry.contains(property.identifier_key)) {
			return utility::error::create(
				utility::error::code::VARIABLE_ALREADY_DECLARED,
				m_context.syntax.string_table.get(property.identifier_key)
			);
		}

		// register the variable
		m_context.variable_registry.pre_declare_variable(property.identifier_key, property.type);

		// type check the assigned value
		if (variable_node->children.get_size() == 1) {
			TRY(type_check_node(variable_node->children[0], property.type));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_function_call(handle<node> call_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		// at this point the function signature is empty
		auto& callee_signature = call_node->get<function_signature>();
		std::vector<data_type> parameter_data_types(call_node->children.get_size());

		// type check all parameters and store their inherent type
		for(u64 i = 0; i < parameter_data_types.size(); ++i) {
			TRY(parameter_data_types[i], type_check_node(call_node->children[i]));
			ASSERT(parameter_data_types[i].base_type != data_type::UNKNOWN, "unknown typ detected");
			// std::cout << "type: " << parameter_data_types[i].to_string() << '\n';
		}

		TRY(callee_signature, m_context.function_registry.get_callee_signature(callee_signature.identifier_key, parameter_data_types));

		// TODO: cast everything according to the callee signature

		//u64 i = 0;
		//for (; i < callee_signature.parameter_types.get_size(); ++i) {
		//	TRY(type_check_node(call_node->children[i], callee_signature.parameter_types[i].type));
		//}

		//// type check var args
		//// these should be promoted to an expected type:
		//// -   bool -> i32
		//// -   f32  -> f64
		//for (; i < call_node->children.get_size(); ++i) {
		//	TRY(type_check_node(call_node->children[i], { data_type::VAR_ARG_PROMOTE, 0 }));
		//}

		return SUCCESS;
	}

	auto type_checker::type_check_return(handle<node> return_node, data_type expected) -> utility::result<data_type> {
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
		m_context.variable_registry.push_scope();

		for (u64 i = 2; i < branch_node->children.get_size(); ++i) {
			TRY(type_check_node(branch_node->children[i], {}));
		}

		m_context.variable_registry.pop_scope();
		return SUCCESS;
	}

	auto type_checker::type_check_branch(handle<node> branch_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);
		m_context.variable_registry.push_scope();

		// just type check all inner statements
		for (const handle<node>& statement : branch_node->children) {
			TRY(type_check_node(statement, {}));
		}

		m_context.variable_registry.pop_scope();
		return SUCCESS;
	}

	auto type_checker::type_check_binary_math_operator(handle<node> operator_node, data_type expected) -> utility::result<data_type> {
		// type check both operands
		TRY(type_check_node(operator_node->children[0], expected));
		TRY(type_check_node(operator_node->children[1], expected));
		return SUCCESS;
	}

	auto type_checker::type_check_numerical_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type> {
		auto& property = literal_node->get<literal>();
		apply_expected_data_type(property.type, expected);
		return property.type;
	}

	auto type_checker::type_check_string_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type> {
		auto& property = literal_node->get<literal>();
		apply_expected_data_type(property.type, expected);
		return property.type;
	}

	auto type_checker::type_check_bool_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(literal_node);
		SUPPRESS_C4100(expected);

		const data_type boolean(data_type::BOOL, 0);
		return boolean;
	}

	auto type_checker::type_check_variable_access(handle<node> access_node, data_type expected) -> utility::result<data_type> {
		auto& property = access_node->get<variable>();

		// locate the variable
		const auto variable = m_context.variable_registry.get_variable(property.identifier_key);
		if(variable == nullptr) {
			return utility::error::create(utility::error::code::UNKNOWN_VARIABLE, m_context.syntax.string_table.get(property.identifier_key));
		}

		property.type = variable->type; // default to the declared type
		apply_expected_data_type(property.type, expected);
		return property.type;
	}

	auto type_checker::type_check_variable_assignment(handle<node> assignment_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);

		// locate the variable
		const handle<node> variable_node = assignment_node->children[0];
		const auto& variable_property = variable_node->get<variable>();
		const auto variable = m_context.variable_registry.get_variable(variable_property.identifier_key);

		if (variable == nullptr) {
			return utility::error::create(utility::error::code::UNKNOWN_VARIABLE, m_context.syntax.string_table.get(variable_property.identifier_key));
		}

		// type check the assigned value against the declared type
		TRY(type_check_node(assignment_node->children[1], variable->type));
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
