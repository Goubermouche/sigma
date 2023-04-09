#include "keyword_i8_node.h"

namespace channel {
	keyword_i8_node::keyword_i8_node(u64 line_number, i64 value)
		:node(line_number), m_value(value) {}

	bool keyword_i8_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(keyword_i8_node);
		return visitor.visit_keyword_i8_node(*this, out_value);
	}

	std::string keyword_i8_node::get_node_name() const {
		return "keyword_i8_node";
	}

	i64 keyword_i8_node::get_value() const {
		return m_value;
	}
}