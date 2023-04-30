#include "i32_node.h"

namespace channel {
	i32_node::i32_node(u64 line_number, i64 value)
		: node(line_number), m_value(value) {}

	bool i32_node::accept(visitor& visitor, value_ptr& out_value)	{
		LOG_NODE_NAME(keyword_i32_node);
		return visitor.visit_keyword_i32_node(*this, out_value);
	}

	void i32_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"IntegerLiteral", "'i32' '" + std::to_string(m_value) + "'\n", is_last);
	}

	i64 i32_node::get_value() const	{
		return m_value;
	}
}