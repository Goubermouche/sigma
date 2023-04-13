#include "u8_node.h"

namespace channel {
	u8_node::u8_node(u64 line_number, u64 value)
		: node(line_number), m_value(value) {}

	bool u8_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(keyword_u8_node);
		return visitor.visit_keyword_u8_node(*this, out_value);
	}

	std::string u8_node::get_node_name() const {
		return "keyword_u8_node";
	}

	u64 u8_node::get_value() const {
		return m_value;
	}
}