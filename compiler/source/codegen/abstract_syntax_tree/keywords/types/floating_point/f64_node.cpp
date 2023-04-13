#include "f64_node.h"

namespace channel {
	f64_node::f64_node(u64 line_number, f64 value)
		:node(line_number), m_value(value) {}

	bool f64_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(keyword_f64_node);
		return visitor.visit_keyword_f64_node(*this, out_value);
	}

	std::string f64_node::get_node_name() const {
		return "keyword_f64_node";
	}

	f64 f64_node::get_value() const {
		return m_value;
	}
}