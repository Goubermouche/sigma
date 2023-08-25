#pragma once
#include "abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents an assignment to a non-array variable.
	 */
	class assignment_node : public node {
	public:
		assignment_node(
			const utility::file_range& range,
			const node_ptr& variable,
			const node_ptr& expression_node
		);

		utility::outcome::result<value_ptr> accept(
			abstract_syntax_tree_visitor_template& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix,
			bool is_last
		) override;

		const node_ptr& get_variable_node() const;
		const node_ptr& get_expression_node() const;
	private:
		node_ptr m_variable_node;
		node_ptr m_expression_node;
	};
}