#include "f32_node.h"

namespace channel {
	f32_node::f32_node(
		const token_position& position,
		f32 value
	) : node(position),
	m_value(value) {}

	bool f32_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_f32_node);
		return visitor.visit_keyword_f32_node(*this, out_value);
	}

	void f32_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix,
			"floating literal",
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'f32' '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value)
			<< color::white
			<< "'\n";
	}

	f32 f32_node::get_value() const	{
		return m_value;
	}
}