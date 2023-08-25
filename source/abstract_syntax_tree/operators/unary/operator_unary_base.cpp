#include "operator_unary_base.h"

namespace sigma {
	operator_unary_base::operator_unary_base(
		const utility::file_range& range,
		const node_ptr& expression_node
	) : node(range),
	m_expression_node(expression_node) {}

	const node_ptr& operator_unary_base::get_expression_node() const {
		return m_expression_node;
	}
}