#include "operator_unary.h"

namespace channel {
	operator_unary::operator_unary(
		u64 line_number, 
		const node_ptr& expression_node
	) : node(line_number),
	m_expression_node(expression_node) {}

	const node_ptr& operator_unary::get_expression_node() const {
		return m_expression_node;
	}
}