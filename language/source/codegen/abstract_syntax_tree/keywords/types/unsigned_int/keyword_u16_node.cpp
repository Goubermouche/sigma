#include "keyword_u16_node.h"

namespace channel {
	keyword_u16_node::keyword_u16_node(u64 line_index, u64 value)
		:node(line_index), m_value(value) {}

	bool keyword_u16_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(keyword_u16_node);
		return visitor.visit_keyword_u16_node(*this, out_value);
	}

	std::string keyword_u16_node::get_node_name() const {
		return "keyword_u16_node";
	}

	u64 keyword_u16_node::get_value() const {
		return m_value;
	}
}