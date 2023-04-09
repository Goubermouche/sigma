#include "u64_node.h"

namespace channel {
	u64_node::u64_node(u64 line_number, u64 value)
		:node(line_number), m_value(value) {}

	bool u64_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(keyword_u64_node);
		return visitor.visit_keyword_u64_node(*this, out_value);
	}

	std::string u64_node::get_node_name() const {
		return "keyword_u64_node";
	}

	u64 u64_node::get_value() const {
		return m_value;
	}
}