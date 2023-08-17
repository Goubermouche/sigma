#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief Base AST node, base for all unary operator nodes.
	 */
	class operator_unary_base : public node {
	public:
		operator_unary_base(
			const file_range& range,
			const node_ptr& expression_node
		);

		const node_ptr& get_expression_node() const;
	private:
		node_ptr m_expression_node;
	};
}