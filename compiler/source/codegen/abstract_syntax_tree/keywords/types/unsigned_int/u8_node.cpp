#include "u8_node.h"

namespace channel {
	u8_node::u8_node(u64 line_number, u64 value)
		: node(line_number), m_value(value) {}

	bool u8_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_u8_node);
		return visitor.visit_keyword_u8_node(*this, out_value);
	}

	void u8_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth,
			prefix,
			"integral literal", 
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'u8' '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value)
			<< color::white
			<< "'\n";
	}

	u64 u8_node::get_value() const {
		return m_value;
	}
}