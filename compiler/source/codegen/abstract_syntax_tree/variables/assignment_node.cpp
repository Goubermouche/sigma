#include "assignment_node.h"

namespace channel {
	assignment_node::assignment_node(u64 line_number, const std::string& name, node* expression)
		: node(line_number), m_name(name), m_expression(expression) {}

	bool assignment_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(assignment_node);
		return visitor.visit_assignment_node(*this, out_value);
	}

	std::string assignment_node::get_node_name() const {
		return "assignment_node";
	}

	const std::string& assignment_node::get_name() const {
		return m_name;
	}

	node* assignment_node::get_expression() const {
		return m_expression;
	}
}