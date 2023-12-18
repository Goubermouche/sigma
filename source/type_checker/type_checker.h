#pragma once
#include <abstract_syntax_tree/abstract_syntax_tree.h>

namespace sigma {
	struct compilation_context;

	/**
	 * \brief A simple type checker implementation, traverses the provided AST and
	 * resolves type relationships, including generics.
	 */
	class type_checker {
	public:
		static void type_check(compilation_context& context);
	private:
		type_checker(compilation_context& context);
		void type_check();

		void type_check_node(handle<node> ast_node, data_type expected = {});

		void type_check_function(handle<node> function_node);
		void type_check_variable_declaration(handle<node> variable_node);

		void type_check_function_call(handle<node> call_node, data_type expected);
		void type_check_return(handle<node> return_node, data_type expected);
		void type_check_conditional_branch(handle<node> branch_node);
		void type_check_branch(handle<node> branch_node);
		void type_check_binary_math_operator(handle<node> operator_node, data_type expected);
		void type_check_variable_access(handle<node> access_node, data_type expected);
		void type_check_variable_assignment(handle<node> assignment_node);

		static void type_check_numerical_literal(handle<node> literal_node, data_type expected);
		static void type_check_string_literal(handle<node> literal_node, data_type expected);
		static void type_check_bool_literal(handle<node> literal_node, data_type expected);

		static void apply_expected_data_type(data_type& target, data_type source);
	private:
		compilation_context& m_context;

		// TODO: create a function registry
		std::unordered_map<utility::string_table_key, handle<function>> m_functions;
		std::unordered_map<utility::string_table_key, function> m_external_functions;

		std::unordered_map<utility::string_table_key, data_type> m_local_variables;
	};
} // namespace sigma
