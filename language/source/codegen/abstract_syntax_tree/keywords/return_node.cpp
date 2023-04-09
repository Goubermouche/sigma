#include "return_node.h"

namespace channel {
	return_node::return_node(u64 line_number, node* expression)
		: node(line_number), m_expression(expression) {}

	bool return_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(return_node);
		return visitor.visit_return_node(*this, out_value);
	}

	std::string return_node::get_node_name() const {
		return "return_node";
	}

	node* return_node::get_expression() const {
		return m_expression;
	}
}