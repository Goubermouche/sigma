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
#include <abstract_syntax_tree/tree.h>

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

		auto type_check_node(handle<ast::node> ast_node, handle<ast::node> parent, data_type expected = {}) -> utility::result<data_type>;

		auto type_check_function_declaration(handle<ast::node> function_node) -> utility::result<data_type>;
		auto type_check_variable_declaration(handle<ast::node> variable_node) -> utility::result<data_type>;

		auto type_check_namespace_declaration(handle<ast::node> variable_node, data_type expected) -> utility::result<data_type>;
		auto type_check_sizeof(handle<ast::node> sizeof_node, handle<ast::node> parent, data_type expected) const -> utility::result<data_type>;

		auto type_check_function_call(handle<ast::node> call_node, handle<ast::node> parent, data_type expected) -> utility::result<data_type>;
		auto type_check_return(handle<ast::node> return_node, data_type expected) -> utility::result<data_type>;
		auto type_check_conditional_branch(handle<ast::node> branch_node) -> utility::result<data_type>;
		auto type_check_branch(handle<ast::node> branch_node) -> utility::result<data_type>;
		auto type_check_binary_math_operator(handle<ast::node> operator_node, data_type expected) -> utility::result<data_type>;
		auto type_check_explicit_cast(handle<ast::node> cast_node, handle<ast::node> parent, data_type expected) -> utility::result<data_type>;
		auto type_check_variable_assignment(handle<ast::node> assignment_node) -> utility::result<data_type>;
		auto type_check_load(handle<ast::node> load_node, data_type expected) -> utility::result<data_type>;

		auto type_check_array_access(handle<ast::node> access_node, handle<ast::node> parent, data_type expected) -> utility::result<data_type>;
		auto type_check_variable_access(handle<ast::node> access_node, handle<ast::node> parent, data_type expected) const->utility::result<data_type>;

		auto type_check_numerical_literal(handle<ast::node> literal_node, data_type expected) const-> utility::result<data_type>;
		auto type_check_character_literal(handle<ast::node> literal_node, handle<ast::node> parent, data_type expected) const-> utility::result<data_type>;
		auto type_check_string_literal(handle<ast::node> literal_node, handle<ast::node> parent, data_type expected) const -> utility::result<data_type>;
		auto type_check_bool_literal(handle<ast::node> literal_node, handle<ast::node> parent, data_type expected) const -> utility::result<data_type>;

		/**
		 * \brief Cast \b original_type to \b target_type as long as \b target_type is known.
		 * \param original_type Original type
		 * \param target_type Target type we want to cast to
		 * \return if \b target_type is known, returns \b target_type, \b original_type otherwise.
		 */
		static auto inherent_type_cast(data_type original_type, data_type target_type) -> data_type;

		/**
		 * \brief Verifies type integrity and attempts to do an implicit type cast, if possible.
		 * \param original_type Original type
		 * \param target_type Preferred type we want to cast to
		 * \param parent Parent AST node of the target node
		 * \param target Target node we want to cast
		 * \return result<data_type> - if no errors occur the final type is returned. 
		 */
		auto implicit_type_cast(data_type original_type, data_type target_type, handle<ast::node> parent, handle<ast::node> target) const -> utility::result<data_type>;
	private:
		backend_context& m_context;
	};
} // namespace sigma
