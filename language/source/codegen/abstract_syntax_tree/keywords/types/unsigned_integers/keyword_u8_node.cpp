#include "keyword_u8_node.h"

namespace channel {
	keyword_u8_node::keyword_u8_node(u8 value)
		: m_value(value) {}

	value* keyword_u8_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_u8_node);
		return visitor.visit_keyword_u8_node(*this);
	}

	std::string keyword_u8_node::get_node_name() const {
		return "keyword_u8_node";
	}

	u8 keyword_u8_node::get_value() const {
		return m_value;
	}
}