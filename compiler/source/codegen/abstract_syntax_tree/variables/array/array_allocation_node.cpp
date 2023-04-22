#include "array_allocation_node.h"

namespace channel {
	array_allocation_node::array_allocation_node(u64 line_number, const type& array_element_type, node* array_element_count_node)
		: node(line_number), m_array_element_type(array_element_type), m_array_element_count(array_element_count_node) {}

	bool array_allocation_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(allocation_node);
		return visitor.visit_allocation_node(*this, out_value);
	}

	std::string array_allocation_node::get_node_name() const {
		return "array_allocation_node";
	}

	const type& array_allocation_node::get_array_element_type() const {
		return m_array_element_type;
	}

	node* array_allocation_node::get_array_element_count_node() const {
		return m_array_element_count;
	}
}