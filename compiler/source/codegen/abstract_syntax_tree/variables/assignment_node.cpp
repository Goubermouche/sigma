#include "assignment_node.h"

namespace channel {
	assignment_node::assignment_node(u64 line_number, const std::string& variable_identifier, node* expression_node)
		: node(line_number), m_variable_identifier(variable_identifier), m_expression_node(expression_node) {}

	bool assignment_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(assignment_node);
		return visitor.visit_assignment_node(*this, out_value);
	}

	std::string assignment_node::get_node_name() const {
		return "assignment_node";
	}

	const std::string& assignment_node::get_variable_identifier() const {
		return m_variable_identifier;
	}

	node* assignment_node::get_expression_node() const {
		return m_expression_node;
	}
}