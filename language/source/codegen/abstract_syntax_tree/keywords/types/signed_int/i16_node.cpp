#include "i16_node.h"

namespace channel {
	i16_node::i16_node(u64 line_number, i64 value)
		:node(line_number), m_value(value) {}

	bool i16_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(keyword_i16_node);
		return visitor.visit_keyword_i16_node(*this, out_value);
	}

	std::string i16_node::get_node_name() const {
		return "keyword_i16_node";
	}

	i64 i16_node::get_value() const {
		return m_value;
	}
}