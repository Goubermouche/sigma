#include "i64_node.h"

namespace channel {
	i64_node::i64_node(u64 line_number, i64 value)
		:node(line_number), m_value(value) {}

	bool i64_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_i64_node);
		return visitor.visit_keyword_i64_node(*this, out_value);
	}

	void i64_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix,
			"integral literal",
			"'i64' '" + std::to_string(m_value) + "'\n", 
			is_last
		);
	}

	i64 i64_node::get_value() const {
		return m_value;
	}
}