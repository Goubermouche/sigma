#include "array_assignment_node.h"

namespace channel {
	array_assignment_node::array_assignment_node(u64 line_number, const std::string& name, node* array_index_node, node* expression)
		: node(line_number), m_name(name), m_array_index_node(array_index_node), m_expression(expression) {}

	bool array_assignment_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(array_assignment_node);
		return visitor.visit_array_assignment_node(*this, out_value);
	}

	std::string array_assignment_node::get_node_name() const {
		return "array_assignment_node";
	}

	const std::string& array_assignment_node::get_name() const {
		return m_name;
	}

	node* array_assignment_node::get_array_index_node() const {
		return m_array_index_node;
	}

	node* array_assignment_node::get_expression_node() const {
		return m_expression;
	}
}

