#include "operator_unary_base.h"

namespace channel {
	operator_unary_base::operator_unary_base(
		const token_location& location,
		const node_ptr& expression_node
	) : node(location),
	m_expression_node(expression_node) {}

	const node_ptr& operator_unary_base::get_expression_node() const {
		return m_expression_node;
	}
}