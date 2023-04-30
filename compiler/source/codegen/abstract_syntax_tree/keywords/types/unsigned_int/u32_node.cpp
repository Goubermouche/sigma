#include "u32_node.h"

namespace channel {
	u32_node::u32_node(u64 line_number, u64 value)
		: node(line_number), m_value(value) {}

	bool u32_node::accept(visitor& visitor, value_ptr& out_value)	{
		LOG_NODE_NAME(keyword_u32_node);
		return visitor.visit_keyword_u32_node(*this, out_value);
	}

	void u32_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"IntegerLiteral", "'u32' '" + std::to_string(m_value) + "'\n", is_last);
	}

	u64 u32_node::get_value() const	{
		return m_value;
	}
}