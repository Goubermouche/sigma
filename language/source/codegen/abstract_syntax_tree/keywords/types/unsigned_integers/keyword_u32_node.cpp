#include "keyword_u32_node.h"

namespace channel {
	keyword_u32_node::keyword_u32_node(u64 value)
		: m_value(value) {}

	value* keyword_u32_node::accept(visitor& visitor)	{
		LOG_NODE_NAME(keyword_u32_node);
		return visitor.visit_keyword_u32_node(*this);
	}

	std::string keyword_u32_node::get_node_name() const {
		return "keyword_u32_node";
	}

	u64 keyword_u32_node::get_value() const	{
		return m_value;
	}
}