#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief Base AST node, base for all unary operator nodes.
	 */
	class operator_unary : public node {
	public:
		operator_unary(
			const token_position& position,
			const node_ptr& expression_node
		);

		const node_ptr& get_expression_node() const;
	private:
		node_ptr m_expression_node;
	};
}