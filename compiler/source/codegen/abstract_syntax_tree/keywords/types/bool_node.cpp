#include "bool_node.h"

namespace channel {
	bool_node::bool_node(u64 line_number, bool value)
		:node(line_number), m_value(value) {}

	bool bool_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(bool_node);
		return visitor.visit_keyword_bool_node(*this, out_value);
	}

	void bool_node::print(int depth, const std::wstring& prefix, bool is_last) {
		const std::string value_string = m_value ? "true" : "false";

		print_value(
			depth,
			prefix, 
			"boolean literal",
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'bool' '"
			<< AST_NODE_BOOLEAN_LITERAL_COLOR
			<< value_string
			<< color::white
			<< "'\n";
	}

	bool bool_node::get_value() const {
		return m_value;
	}
}