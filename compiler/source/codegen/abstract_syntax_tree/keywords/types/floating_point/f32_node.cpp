#include "f32_node.h"

namespace channel {
	f32_node::f32_node(u64 line_number, f32 value)
		: node(line_number), m_value(value) {}

	bool f32_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_f32_node);
		return visitor.visit_keyword_f32_node(*this, out_value);
	}

	void f32_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"FloatingLiteral", "'f32' '" + std::to_string(m_value) + "'\n", is_last);
	}

	f32 f32_node::get_value() const	{
		return m_value;
	}
}