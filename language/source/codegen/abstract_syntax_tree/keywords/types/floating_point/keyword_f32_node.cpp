#include "keyword_f32_node.h"

namespace channel {
	keyword_f32_node::keyword_f32_node(f32 value)
		: m_value(value) {}

	value* keyword_f32_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_f32_node);
		return visitor.visit_keyword_f32_node(*this);
	}

	std::string keyword_f32_node::get_node_name() const	{
		return "keyword_f32_node";
	}

	f32 keyword_f32_node::get_value() const	{
		return m_value;
	}
}