#include "i16_node.h"

namespace channel {
	i16_node::i16_node(u64 line_number, i64 value)
		:node(line_number), m_value(value) {}

	bool i16_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(keyword_i16_node);
		return visitor.visit_keyword_i16_node(*this, out_value);
	}

	void i16_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"IntegerLiteral", "'i16' '" + std::to_string(m_value) + "'\n", is_last);
	}

	i64 i16_node::get_value() const {
		return m_value;
	}
}