#include "allocation_node.h"

namespace channel {
	allocation_node::allocation_node(u64 line_number, type element_type, node* array_size)
		: node(line_number), m_element_type(element_type), m_array_size(array_size) {}

	bool allocation_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(allocation_node);
		return visitor.visit_allocation_node(*this, out_value);
	}

	std::string allocation_node::get_node_name() const {
		return "allocation_node";
	}

	type allocation_node::get_element_type() const {
		return m_element_type;
	}

	node* allocation_node::get_array_size_node() const {
		return m_array_size;
	}
}