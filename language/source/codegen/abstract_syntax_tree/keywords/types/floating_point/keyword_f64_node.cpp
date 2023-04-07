#include "keyword_f64_node.h"

namespace channel {
	keyword_f64_node::keyword_f64_node(f64 value)
		: m_value(value) {}

	value* keyword_f64_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_f64_node);
		return visitor.visit_keyword_f64_node(*this);
	}

	std::string keyword_f64_node::get_node_name() const {
		return "keyword_f64_node";
	}

	f64 keyword_f64_node::get_value() const {
		return m_value;
	}
}