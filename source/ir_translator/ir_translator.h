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
#include <intermediate_representation/builder.h>

namespace sigma {
	struct backend_context;

	class ir_translator {
	public:
		static auto translate(backend_context& context) -> utility::result<void>;
	private:
		ir_translator(backend_context& context);
		auto translate() -> utility::result<void>;

		handle<ir::node> translate_node(handle<node> ast_node);

		void translate_function_declaration(handle<node> function_node);
		void translate_variable_declaration(handle<node> variable_node);
		void translate_namespace_declaration(handle<node> namespace_node);

		void translate_return(handle<node> return_node);

		/**
		 * \brief Translates a conditional ast branch node into IR.
		 * \param branch_node Branch node to translate
		 * \param end_control End control, nullptr by default, when set to a non-null value the branch
		 * will go here after it finishes running
		 */
		void translate_conditional_branch(handle<node> branch_node, handle<ir::node> end_control = nullptr);

		/**
		 * \brief Translates a non-conditional ast branch node into IR.
		 * \param branch_node Branch node to translate
		 * \param exit_control
		 */
		void translate_branch(handle<node> branch_node, handle<ir::node> exit_control);

		auto translate_numerical_literal(handle<node> numerical_literal_node) const->handle<ir::node>;
		auto translate_character_literal(handle<node> character_literal_node) const->handle<ir::node>;
		auto translate_string_literal(handle<node> string_literal_node) const->handle<ir::node>;
		auto translate_bool_literal(handle<node> bool_literal_node) const->handle<ir::node>;

		auto translate_binary_math_operator(handle<node> operator_node) -> handle<ir::node>;
		auto translate_cast(handle<node> cast_node) -> handle<ir::node>;
		auto translate_function_call(handle<node> call_node) -> handle<ir::node>;

		auto translate_variable_access(handle<node> access_node) const-> handle<ir::node>;
		auto translate_variable_assignment(handle<node> assignment_node) -> handle<ir::node>;

		auto literal_to_ir(const ast_literal& literal) const-> handle<ir::node>;
	private:
		backend_context& m_context;
	};
} // namespace sigma
