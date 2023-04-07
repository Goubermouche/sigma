#include "keyword_i32_node.h"

namespace channel {
	keyword_i32_node::keyword_i32_node(i64 value)
		: m_value(value) {}

	value* keyword_i32_node::accept(visitor& visitor)	{
		LOG_NODE_NAME(keyword_i32_node);
		return visitor.visit_keyword_i32_node(*this);
	}

	std::string keyword_i32_node::get_node_name() const {
		return "keyword_i32_node";
	}

	i64 keyword_i32_node::get_value() const	{
		return m_value;
	}
}