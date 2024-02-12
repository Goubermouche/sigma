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
		using type_check_result = utility::result<type>;
		using ast_node = handle<ast::node>;

		type_checker(backend_context& context);
		auto type_check() -> utility::result<void>;

		auto type_check_node(ast_node target, ast_node parent, type expected = type::create_unknown()) -> type_check_result;

		// declarations
		auto type_check_namespace_declaration(ast_node declaration) -> type_check_result;
		auto type_check_function_declaration(ast_node declaration) -> type_check_result;
		auto type_check_variable_declaration(ast_node declaration) const -> type_check_result;
		auto type_check_struct_declaration(ast_node declaration) const -> type_check_result;

		// literals
		auto type_check_character_literal(ast_node literal, ast_node parent, type expected) const ->type_check_result;
		auto type_check_string_literal(ast_node literal, ast_node parent, type expected) const -> type_check_result;
		auto type_check_bool_literal(ast_node literal, ast_node parent, type expected) const -> type_check_result;
		auto type_check_numerical_literal(ast_node literal, type expected) const -> type_check_result;

		// expressions
		auto type_check_binary_math_operator(ast_node binop, type expected) -> type_check_result;
		auto type_check_binary_comparison_operator(ast_node binop, ast_node parent, type expected) -> type_check_result;
		auto type_check_predicate_operator(ast_node binop, ast_node parent, type expected) -> type_check_result;
		auto type_check_not_operator(ast_node op, ast_node parent, type expected) -> type_check_result;

		// statements
		auto type_check_return(ast_node statement) -> type_check_result;
		auto type_check_conditional_branch(ast_node branch) -> type_check_result;
		auto type_check_branch(ast_node branch) -> type_check_result;

		// loads / stores
		auto type_check_variable_access(ast_node access, ast_node parent, type expected) const->type_check_result;
		auto type_check_array_access(ast_node access, ast_node parent, type expected) -> type_check_result;
		auto type_check_local_member_access(ast_node access, ast_node parent, type expected) -> type_check_result;
		auto type_check_load(ast_node load, type expected) -> type_check_result;
		auto type_check_store(ast_node store) -> type_check_result;

		// other
		auto type_check_alignof(ast_node alignof_node, ast_node parent, type expected) const->type_check_result;
		auto type_check_sizeof(ast_node sizeof_node, ast_node parent, type expected) const->type_check_result;
		auto type_check_function_call(ast_node call, ast_node parent, type expected) -> type_check_result;
		auto type_check_explicit_cast(ast_node cast, ast_node parent, type expected) -> type_check_result;


		/**
		 * \brief Cast \b original_type to \b target_type as long as \b target_type is known.
		 * \param original_type Original type
		 * \param target_type Target type we want to cast to
		 * \return if \b target_type is known, returns \b target_type, \b original_type otherwise.
		 */
		static auto inherent_type_cast(type original_type, type target_type) -> type;

		/**
		 * \brief Verifies type integrity and attempts to do an implicit type cast, if possible.
		 * \param original_type Original type
		 * \param target_type Preferred type we want to cast to
		 * \param parent Parent AST node of the target node
		 * \param target Target node we want to cast
		 * \return result<type> - if no errors occur the final type is returned. 
		 */
		auto implicit_type_cast(type original_type, type target_type, ast_node parent, ast_node target) const -> type_check_result;
	private:
		backend_context& m_context;
		function_signature m_current_function;
	};
} // namespace sigma
