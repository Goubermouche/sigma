// Root type checker file, contains a basic type checker implementation
// 
// 1.   Traverse our AST from every root node (functions, globals, namespaces).
// 2.   Resolve type relationships (expected vs. current).
// 3.   Perform implicit casts whenever possible / return errors otherwise.
//          - Implicit casts are inserted into the AST after it has been built, we store the parent
//            node of every node in function parameters so that we can insert it before the node
//            which is being extended/truncated.
//
// -    It might be a good idea to rework this with DP, so that we don't strain the stack so much,
//      this could also improve performance and the overall syntax.

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

		auto type_check_node(handle<node> ast_node, handle<node> parent, data_type expected = {}) -> utility::result<data_type>;

		auto type_check_function_declaration(handle<node> function_node) -> utility::result<data_type>;
		auto type_check_variable_declaration(handle<node> variable_node) -> utility::result<data_type>;

		auto type_check_namespace_declaration(handle<node> variable_node, data_type expected) -> utility::result<data_type>;
		auto type_check_sizeof(handle<node> sizeof_node, handle<node> parent, data_type expected) -> utility::result<data_type>;

		auto type_check_function_call(handle<node> call_node, handle<node> parent, data_type expected) -> utility::result<data_type>;
		auto type_check_return(handle<node> return_node, data_type expected) -> utility::result<data_type>;
		auto type_check_conditional_branch(handle<node> branch_node) -> utility::result<data_type>;
		auto type_check_branch(handle<node> branch_node) -> utility::result<data_type>;
		auto type_check_binary_math_operator(handle<node> operator_node, data_type expected) -> utility::result<data_type>;
		auto type_check_explicit_cast(handle<node> cast_node, handle<node> parent, data_type expected) -> utility::result<data_type>;
		auto type_check_variable_access(handle<node> access_node, handle<node> parent, data_type expected) const-> utility::result<data_type>;
		auto type_check_array_access(handle<node> access_node, handle<node> parent, data_type expected) -> utility::result<data_type>;
		auto type_check_variable_assignment(handle<node> assignment_node) -> utility::result<data_type>;
		auto type_check_load(handle<node> load_node, handle<node> parent, data_type expected) -> utility::result<data_type>;

		auto type_check_numerical_literal(handle<node> literal_node, data_type expected) const-> utility::result<data_type>;
		auto type_check_character_literal(handle<node> literal_node, handle<node> parent, data_type expected) const-> utility::result<data_type>;
		auto type_check_string_literal(handle<node> literal_node, handle<node> parent, data_type expected) const -> utility::result<data_type>;
		auto type_check_bool_literal(handle<node> literal_node, handle<node> parent, data_type expected) const -> utility::result<data_type>;

		static auto inherent_type_cast(data_type original_type, data_type target_type) -> data_type;
		auto implicit_type_cast(data_type original_type, data_type target_type, handle<node> parent, handle<node> target) const -> utility::result<data_type>;
		auto explicit_type_cast(data_type original_type, data_type target_type, handle<node> target) const -> utility::result<void>;;
	private:
		backend_context& m_context;
	};
} // namespace sigma
