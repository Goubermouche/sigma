#pragma once
#include "ir_translator/values/function_registry.h"
#include "ir_translator/values/variable_registry.h"

#include <abstract_syntax_tree/abstract_syntax_tree.h>

namespace sigma {
	struct compilation_context;

	class ir_translator {
	public:
		static auto translate(compilation_context& context, ir::target target) -> ir::module;
	private:
		ir_translator(compilation_context& context, ir::target target);
		auto translate() -> ir::module;

		handle<ir::node> translate_node(handle<node> ast_node);

		void translate_function_declaration(handle<node> function_node);
		void translate_variable_declaration(handle<node> variable_node);
		void translate_return(handle<node> return_node);

		/**
		 * \brief Translates a conditional ast branch node into IR.
		 * \param branch_node Branch node to translate
		 * \param end_control End control, nullptr by default, when set to a non-null value the branch
		 * will go here after it finishes running
		 */
		void translate_conditional_branch(
			handle<node> branch_node, handle<ir::node> end_control = nullptr
		);

		/**
		 * \brief Translates a non-conditional ast branch node into IR.
		 * \param branch_node Branch node to translate
		 * \param exit_control
		 */
		void translate_branch(handle<node> branch_node, handle<ir::node> exit_control);

		auto translate_numerical_literal(handle<node> numerical_literal_node) const->handle<ir::node>;
		auto translate_string_literal(handle<node> string_literal_node) const->handle<ir::node>;
		auto translate_bool_literal(handle<node> bool_literal_node) const->handle<ir::node>;

		auto translate_binary_math_operator(handle<node> operator_node) -> handle<ir::node>;
		auto translate_function_call(handle<node> call_node) -> handle<ir::node>;

		auto translate_variable_access(handle<node> access_node) -> handle<ir::node>;
		auto translate_variable_assignment(handle<node> assignment_node) -> handle<ir::node>;

		auto literal_to_ir(literal& literal) const-> handle<ir::node>;
		static auto data_type_to_ir(data_type dt) -> ir::data_type;
	private:
		compilation_context& m_context;

		ir::module m_module;
		ir::builder m_builder;

		detail::function_registry m_functions;
		detail::variable_registry m_variables;
	};
} // namespace sigma
