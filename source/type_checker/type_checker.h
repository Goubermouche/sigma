#pragma once
#include <abstract_syntax_tree/abstract_syntax_tree.h>

namespace sigma {
	struct backend_context;

	/**
	 * \brief A simple type checker implementation, traverses the provided AST and
	 * resolves type relationships, including generics.
	 */
	class type_checker {
	public:
		static auto type_check(backend_context& context) -> utility::result<void>;
	private:
		type_checker(backend_context& context);
		auto type_check() -> utility::result<void>;

		auto type_check_node(handle<node> ast_node, data_type expected = {}) -> utility::result<data_type>;

		auto type_check_function_declaration(handle<node> function_node, data_type expected) -> utility::result<data_type>;
		auto type_check_variable_declaration(handle<node> variable_node, data_type expected) -> utility::result<data_type>;
		auto type_check_namespace_declaration(handle<node> variable_node, data_type expected) -> utility::result<data_type>;

		auto type_check_function_call(handle<node> call_node, data_type expected) -> utility::result<data_type>;
		auto type_check_return(handle<node> return_node, data_type expected) -> utility::result<data_type>;
		auto type_check_conditional_branch(handle<node> branch_node, data_type expected)->utility::result<data_type>;
		auto type_check_branch(handle<node> branch_node, data_type expected)->utility::result<data_type>;
		auto type_check_binary_math_operator(handle<node> operator_node, data_type expected)->utility::result<data_type>;
		auto type_check_variable_access(handle<node> access_node, data_type expected)->utility::result<data_type>;
		auto type_check_variable_assignment(handle<node> assignment_node, data_type expected)->utility::result<data_type>;

		auto type_check_numerical_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type>;
		auto type_check_character_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type>;
		auto type_check_string_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type>;
		auto type_check_bool_literal(handle<node> literal_node, data_type expected) -> utility::result<data_type>;

		static void apply_expected_data_type(data_type& target, data_type source);
	private:
		backend_context& m_context;
	};
} // namespace sigma
