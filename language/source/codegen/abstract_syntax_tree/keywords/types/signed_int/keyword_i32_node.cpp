#include "keyword_i32_node.h"

namespace channel {
	keyword_i32_node::keyword_i32_node(u64 line_index, i64 value)
		: node(line_index), m_value(value) {}

	bool keyword_i32_node::accept(visitor& visitor, value*& out_value)	{
		LOG_NODE_NAME(keyword_i32_node);
		return visitor.visit_keyword_i32_node(*this, out_value);
	}

	std::string keyword_i32_node::get_node_name() const {
		return "keyword_i32_node";
	}

	i64 keyword_i32_node::get_value() const	{
		return m_value;
	}
}