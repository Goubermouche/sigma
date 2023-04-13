#include "u32_node.h"

namespace channel {
	u32_node::u32_node(u64 line_number, u64 value)
		: node(line_number), m_value(value) {}

	bool u32_node::accept(visitor& visitor, value*& out_value)	{
		LOG_NODE_NAME(keyword_u32_node);
		return visitor.visit_keyword_u32_node(*this, out_value);
	}

	std::string u32_node::get_node_name() const {
		return "keyword_u32_node";
	}

	u64 u32_node::get_value() const	{
		return m_value;
	}
}