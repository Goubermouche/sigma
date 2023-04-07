#include "keyword_i64_node.h"

namespace channel {
	keyword_i64_node::keyword_i64_node(i64 value)
		: m_value(value) {}

	value* keyword_i64_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_i64_node);
		return visitor.visit_keyword_i64_node(*this);
	}

	std::string keyword_i64_node::get_node_name() const {
		return "keyword_i64_node";
	}

	bool keyword_i64_node::is_signed() const {
		return true;
	}

	i64 keyword_i64_node::get_value() const {
		return m_value;
	}
}