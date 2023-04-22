#include "bool_node.h"

namespace channel {
	bool_node::bool_node(u64 line_number, bool value)
		:node(line_number), m_value(value) {}

	bool bool_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(bool_node);
		return visitor.visit_keyword_bool_node(*this, out_value);
	}

	std::string bool_node::get_node_name() const {
		return "bool_node";
	}

	bool bool_node::get_value() const {
		return m_value;
	}
}