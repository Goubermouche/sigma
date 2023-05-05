#include "operator_unary.h"

namespace channel {
	operator_unary::operator_unary(
		const token_position& position,
		const node_ptr& expression_node
	) : node(position),
	m_expression_node(expression_node) {}

	const node_ptr& operator_unary::get_expression_node() const {
		return m_expression_node;
	}
}