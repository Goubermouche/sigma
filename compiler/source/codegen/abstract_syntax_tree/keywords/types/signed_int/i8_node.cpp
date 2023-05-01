#include "i8_node.h"

namespace channel {
	i8_node::i8_node(u64 line_number, i64 value)
		:node(line_number), m_value(value) {}

	bool i8_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_i8_node);
		return visitor.visit_keyword_i8_node(*this, out_value);
	}

	void i8_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix,
			"integral literal", 
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'i8' '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value)
			<< color::white
			<< "'\n";
	}

	i64 i8_node::get_value() const {
		return m_value;
	}
}