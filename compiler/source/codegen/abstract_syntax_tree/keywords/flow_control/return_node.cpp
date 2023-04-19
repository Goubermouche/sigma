#include "return_node.h"

namespace channel {
	return_node::return_node(u64 line_number, node* return_expression_node)
		: node(line_number), m_return_expression_node(return_expression_node) {}

	bool return_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(return_node);
		return visitor.visit_return_node(*this, out_value);
	}

	std::string return_node::get_node_name() const {
		return "return_node";
	}

	node* return_node::get_return_expression_node() const {
		return m_return_expression_node;
	}
}