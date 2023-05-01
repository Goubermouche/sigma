#include "u16_node.h"

namespace channel {
	u16_node::u16_node(u64 line_number, u64 value)
		:node(line_number), m_value(value) {}

	bool u16_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_u16_node);
		return visitor.visit_keyword_u16_node(*this, out_value);
	}

	void u16_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix,
			"integral literal",
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'u16' '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value)
			<< color::white
			<< "'\n";
	}

	u64 u16_node::get_value() const {
		return m_value;
	}
}