#include "keyword_u64_node.h"

namespace channel {
	keyword_u64_node::keyword_u64_node(u64 line_index, u64 value)
		:node(line_index), m_value(value) {}

	value* keyword_u64_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_u64_node);
		return visitor.visit_keyword_u64_node(*this);
	}

	std::string keyword_u64_node::get_node_name() const {
		return "keyword_u64_node";
	}

	u64 keyword_u64_node::get_value() const {
		return m_value;
	}
}