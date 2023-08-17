#include "operator_binary_base.h"

namespace sigma {
	operator_binary_base::operator_binary_base(
		const file_range& range,
		const node_ptr& left_expression_node,
		const node_ptr& right_expression_node
	) : node(range),
	m_left_expression_node(left_expression_node),
	m_right_expression_node(right_expression_node) {}

	const node_ptr& operator_binary_base::get_left_expression_node() const {
		return m_left_expression_node;
	}

	const node_ptr& operator_binary_base::get_right_expression_node() const {
		return m_right_expression_node;
	}
}