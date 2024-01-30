#pragma once
#include <abstract_syntax_tree/tree.h>
#include <intermediate_representation/builder.h>

namespace sigma {
	struct backend_context;

	class ir_translator {
	public:
		static auto translate(backend_context& context) -> utility::result<void>;
	private:
		ir_translator(backend_context& context);
		auto translate() -> utility::result<void>;

		auto translate_node(handle<ast::node> ast_node) -> handle<ir::node>;

		// declarations
		void translate_namespace_declaration(handle<ast::node> namespace_node);
		void translate_function_declaration(handle<ast::node> function_node);
		void translate_variable_declaration(handle<ast::node> variable_node);

		// literals
		auto translate_numerical_literal(handle<ast::node> numerical_literal_node) const -> handle<ir::node>;
		auto translate_character_literal(handle<ast::node> character_literal_node) const -> handle<ir::node>;
		auto translate_string_literal(handle<ast::node> string_literal_node) const -> handle<ir::node>;
		auto translate_bool_literal(handle<ast::node> bool_literal_node) const -> handle<ir::node>;

		// expressions
		auto translate_binary_math_operator(handle<ast::node> operator_node) -> handle<ir::node>;
		auto translate_binary_comparison_operator(handle<ast::node> operator_node) -> handle<ir::node>;
		auto translate_binary_equality_operator(handle<ast::node> operator_node) -> handle<ir::node>;
		auto translate_predicate_operator(handle<ast::node> operator_node) -> handle<ir::node>;

		// statements
		void translate_return(handle<ast::node> return_node);

		/**
		 * \brief Translates a conditional ast branch node into IR.
		 * \param branch_node Branch node to translate
		 * \param end_control End control, nullptr by default, when set to a non-null value the branch
		 * will go here after it finishes running
		 */
		void translate_conditional_branch(handle<ast::node> branch_node, handle<ir::node> end_control = nullptr);

		/**
		 * \brief Translates a non-conditional ast branch node into IR.
		 * \param branch_node Branch node to translate
		 * \param exit_control
		 */
		void translate_branch(handle<ast::node> branch_node, handle<ir::node> exit_control);

		// loads / stores
		auto translate_variable_access(handle<ast::node> access_node) const -> handle<ir::node>;
		auto translate_array_access(handle<ast::node> access_node) -> handle<ir::node>;
		auto translate_store(handle<ast::node> assignment_node) -> handle<ir::node>;
		auto translate_load(handle<ast::node> load_node) -> handle<ir::node>;

		// other
		auto translate_function_call(handle<ast::node> call_node) -> handle<ir::node>;
		auto translate_cast(handle<ast::node> cast_node) -> handle<ir::node>;
		auto translate_sizeof(handle<ast::node> sizeof_node) const -> handle<ir::node>;

		// utility
		auto literal_to_ir(const ast::named_type_expression& literal) const -> handle<ir::node>;
	private:
		backend_context& m_context;
	};
} // namespace sigma
