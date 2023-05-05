#include "i32_node.h"

namespace channel {
	i32_node::i32_node(
		const token_position& position,
		i64 value
	) : node(position),
	m_value(value) {}

	bool i32_node::accept(visitor& visitor, value_ptr& out_value)	{
		LOG_NODE_NAME(keyword_i32_node);
		return visitor.visit_keyword_i32_node(*this, out_value);
	}

	void i32_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix, 
			"integral literal", 
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'i32' '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value)
			<< color::white
			<< "'\n";
	}

	i64 i32_node::get_value() const	{
		return m_value;
	}
}