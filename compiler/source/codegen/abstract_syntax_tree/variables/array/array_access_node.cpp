#include "array_access_node.h"

namespace channel {
	array_access_node::array_access_node(u64 line_number, const std::string& array_identifier, node* array_index_node)
		: node(line_number), m_array_identifier(array_identifier), m_array_element_index_node(array_index_node){
	}

	bool array_access_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(array_access_node);
		return visitor.visit_array_access_node(*this, out_value);
	}

	std::string array_access_node::get_node_name() const {
		return "array_access_node";
	}

	const std::string& array_access_node::get_array_identifier() const {
		return m_array_identifier;
	}

	node* array_access_node::get_array_element_index_node() const {
		return m_array_element_index_node;
	}
}