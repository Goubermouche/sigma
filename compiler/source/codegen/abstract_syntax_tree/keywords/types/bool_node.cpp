#include "bool_node.h"

namespace channel {
	bool_node::bool_node(
		const token_position& position, bool 
		value
	) : node(position),
	m_value(value) {}

	acceptation_result bool_node::accept(visitor& visitor, const codegen_context& context) {
		return visitor.visit_keyword_bool_node(*this, context);
	}

	void bool_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
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