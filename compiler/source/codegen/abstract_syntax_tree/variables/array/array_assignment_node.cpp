#include "array_assignment_node.h"

namespace channel {
	array_assignment_node::array_assignment_node(u64 line_number, node* array_base, const std::vector<node*>& index_nodes, node* expression_node)
		: node(line_number), m_array_base_node(array_base), m_array_element_index_nodes(index_nodes), m_expression_node(expression_node) {}

	bool array_assignment_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(array_assignment_node);
		return visitor.visit_array_assignment_node(*this, out_value);
	}

	std::string array_assignment_node::get_node_name() const {
		return "array_assignment_node";
	}

	node* array_assignment_node::get_array_base_node() const	{
		return m_array_base_node;
	}

	const std::vector<node*>& array_assignment_node::get_array_element_index_nodes() const {
		return m_array_element_index_nodes;
	}

	node* array_assignment_node::get_expression_node() const {
		return m_expression_node;
	}
}