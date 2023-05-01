#include "u64_node.h"

namespace channel {
	u64_node::u64_node(u64 line_number, u64 value)
		:node(line_number), m_value(value) {}

	bool u64_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_u64_node);
		return visitor.visit_keyword_u64_node(*this, out_value);
	}

	void u64_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix, 
			"integral literal", 
			"'u64' '" + std::to_string(m_value) + "'\n", 
			is_last
		);
	}

	u64 u64_node::get_value() const {
		return m_value;
	}
}