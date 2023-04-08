#include "return_node.h"

namespace channel {
	return_node::return_node(u64 line_index, node* expression)
		: node(line_index), m_expression(expression)
	{}

	value* return_node::accept(visitor& visitor) {
		LOG_NODE_NAME(return_node);
		return visitor.visit_return_node(*this);
	}

	std::string return_node::get_node_name() const {
		return "return_node";
	}

	node* return_node::get_expression() const {
		return m_expression;
	}
}