#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief Base AST node, base for all binary operator nodes.
	 */
	class operator_binary_base : public node	{
	public:
		operator_binary_base(
			const token_position& position,
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		const node_ptr& get_left_expression_node() const;
		const node_ptr& get_right_expression_node() const;
	private:
		node_ptr m_left_expression_node;
		node_ptr m_right_expression_node;
	};
}