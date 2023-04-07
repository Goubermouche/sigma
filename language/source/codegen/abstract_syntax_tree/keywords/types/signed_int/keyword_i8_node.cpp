#include "keyword_i8_node.h"

namespace channel {
	keyword_i8_node::keyword_i8_node(i64 value)
		: m_value(value) {}

	value* keyword_i8_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_i8_node);
		return visitor.visit_keyword_i8_node(*this);
	}

	std::string keyword_i8_node::get_node_name() const {
		return "keyword_i8_node";
	}

	i64 keyword_i8_node::get_value() const {
		return m_value;
	}
}