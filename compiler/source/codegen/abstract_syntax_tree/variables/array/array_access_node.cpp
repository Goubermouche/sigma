#include "array_access_node.h"

namespace channel {
	array_access_node::array_access_node(u64 line_number, node* array_base, const std::vector<node*>& array_element_index_nodes)
		: node(line_number), m_array_base(array_base), m_array_element_index_nodes(array_element_index_nodes) {}

	bool array_access_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(array_access_node);
		return visitor.visit_array_access_node(*this, out_value);
	}

	std::string array_access_node::get_node_name() const {
		return "array_access_node";
	}

	node* array_access_node::get_array_base_node() const {
		return m_array_base;
	}

	const std::vector<node*>& array_access_node::get_array_element_index_nodes() const {
		return m_array_element_index_nodes;
	}
}