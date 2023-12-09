#include "type_checker.h"

namespace sigma {
	void type_checker::type_check(abstract_syntax_tree& ast, utility::symbol_table& symbols) {
		type_checker(ast, symbols).type_check();
	}

	type_checker::type_checker(abstract_syntax_tree& ast, utility::symbol_table& symbols)
		: m_ast(ast)
	{
		const utility::symbol_table_key printf_key = symbols.insert("printf");
		const utility::symbol_table_key format_key = symbols.insert("format");

		auto printf_params = utility::slice<named_type>(m_ast.get_allocator(), 1);
		printf_params[0] = named_type{ data_type(data_type::CHAR, 1), format_key };

		m_external_functions[printf_key] = function{
			.return_type = data_type(data_type::I32, 0),
			.identifier_key = printf_key,
			.parameters = printf_params,
			.has_var_args = true
		};
	}

	void type_checker::type_check() {
		for (const handle<node> top_level : m_ast.get_nodes()) {
			type_check_node(top_level);
		}
	}

	void type_checker::type_check_node(handle<node> ast_node, data_type expected) {
		switch (ast_node->type) {
			case node_type::FUNCTION:             type_check_function(ast_node); break;
			case node_type::FUNCTION_CALL:        type_check_function_call(ast_node, expected); break;

			// flow control
			case node_type::RETURN:               type_check_return(ast_node, expected); break;
			case node_type::CONDITIONAL_BRANCH:   type_check_conditional_branch(ast_node); break;
			case node_type::BRANCH:               type_check_branch(ast_node); break;

			// variables
			case node_type::VARIABLE_DECLARATION: type_check_variable_declaration(ast_node); break;
			case node_type::VARIABLE_ACCESS:      type_check_variable_access(ast_node, expected); break;

			// operators:
			case node_type::OPERATOR_ADD:
			case node_type::OPERATOR_SUBTRACT:
			case node_type::OPERATOR_MULTIPLY:
			case node_type::OPERATOR_DIVIDE:
			case node_type::OPERATOR_MODULO:      type_check_binary_math_operator(ast_node, expected); break;

			// literals
			case node_type::NUMERICAL_LITERAL:    type_check_numerical_literal(ast_node, expected); break;
			case node_type::STRING_LITERAL:       type_check_string_literal(ast_node, expected); break;
			case node_type::BOOL_LITERAL:         type_check_bool_literal(ast_node, expected); break;

			default: ASSERT(false, "unchecked node detected");
		}
	}

	void type_checker::type_check_function(handle<node> function_node) {
		auto& property = function_node->get<function>();

		// register the function
		m_functions[property.identifier_key] = &property;

		// handle inner statements
		for (const handle<node> statement : function_node->children) {
			type_check_node(statement, property.return_type);
		}
	}

	void type_checker::type_check_variable_declaration(handle<node> variable_node) {
		const auto& property = variable_node->get<variable>();

		// register the variable
		m_local_variables[property.identifier_key] = property.data_type;

		// type check the assigned value
		if (variable_node->children.get_size() == 1) {
			type_check_node(variable_node->children[0], property.data_type);
		}
	}

	void type_checker::type_check_function_call(handle<node> call_node, data_type expected) {
		// TODO: move the function check in here?
		auto& property = call_node->get<function_call>();

		handle<function> callee;
		const auto local_call = m_functions.find(property.callee_identifier_key);

		if (local_call == m_functions.end()) {
			const auto external_call = m_external_functions.find(property.callee_identifier_key);
			ASSERT(external_call != m_external_functions.end(), "call to an unknown function detected");
			callee = &external_call->second;

			property.is_external = true;
		}
		else {
			callee = local_call->second;
			property.is_external = false;
		}

		if (callee->has_var_args) {
			ASSERT(call_node->children.get_size() >= callee->parameters.get_size(), "invalid parameter count");
		}
		else {
			ASSERT(callee->parameters.get_size() == call_node->children.get_size(), "invalid parameter count");
		}

		u64 i = 0;
		for (; i < callee->parameters.get_size(); ++i) {
			type_check_node(call_node->children[i], callee->parameters[i].type);
		}

		// variable arguments should be promoted
		// bool -> i32
		// f32  -> f64
		for (; i < call_node->children.get_size(); ++i) {
			type_check_node(call_node->children[i], { data_type::VAR_ARG_PROMOTE, 0 });
		}
	}

	void type_checker::type_check_return(handle<node> return_node, data_type expected) {
		if (return_node->children.get_size() == 0) {
			ASSERT(
				expected == data_type(data_type::VOID, 0),
				"invalid return data type"
			);
		}
		else {
			type_check_node(return_node->children[0], expected);
		}
	}

	void type_checker::type_check_conditional_branch(handle<node> branch_node) {
		type_check_node(branch_node->children[0], data_type(data_type::BOOL, 0));

		if (branch_node->children[1]) {
			if (branch_node->children[1]->type == node_type::CONDITIONAL_BRANCH) {
				type_check_conditional_branch(branch_node->children[1]);
			}
			else if (branch_node->children[1]->type == node_type::BRANCH) {
				type_check_branch(branch_node->children[1]);
			}
			else {
				ASSERT(false, "unexpected node type");
			}
		}

		for (u64 i = 2; i < branch_node->children.get_size(); ++i) {
			type_check_node(branch_node->children[i], {});
		}
	}

	void type_checker::type_check_branch(handle<node> branch_node) {
		for (const handle<node> statement : branch_node->children) {
			type_check_node(statement, {});
		}
	}

	void type_checker::type_check_binary_math_operator(handle<node> operator_node, data_type expected) {
		type_check_node(operator_node->children[0], expected);
		type_check_node(operator_node->children[1], expected);
	}

	void type_checker::type_check_numerical_literal(handle<node> literal_node, data_type expected) {
		auto& prop = literal_node->get<literal>();
		apply_expected_data_type(prop.data_type, expected);
	}

	void type_checker::type_check_string_literal(handle<node> literal_node, data_type expected) {
		auto& prop = literal_node->get<literal>();
		apply_expected_data_type(prop.data_type, expected);
	}

	void type_checker::type_check_bool_literal(handle<node> literal_node, data_type expected) {
		ASSERT(expected.type == data_type::BOOL, "unexpected type encountered");
	}

	void type_checker::type_check_variable_access(handle<node> access_node, data_type expected) {
		auto& prop = access_node->get<variable_access>();

		const auto it = m_local_variables.find(prop.identifier_key);
		ASSERT(it != m_local_variables.end(), "unknown local variable");

		prop.data_type = it->second; // default to the declared type 

		apply_expected_data_type(prop.data_type, expected);
	}

	void type_checker::apply_expected_data_type(data_type& target, data_type source) {
		if (source.type != data_type::UNKNOWN) {
			if (source.type == data_type::VAR_ARG_PROMOTE) {
				// promote the variable
				// don't promote pointers 
				if (target.pointer_level > 0) {
					return;
				}

				switch (target.type) {
					case data_type::UNKNOWN: ASSERT(false, "promotion on unknown data type"); break;
					case data_type::I32:     return;
					case data_type::BOOL:    target.type = data_type::I32; break;
					default:                        ASSERT(false, "unimplemented promotion");
				}
			}
			else {
				target = source;
			}
		}
	}
} // namespace sigma
