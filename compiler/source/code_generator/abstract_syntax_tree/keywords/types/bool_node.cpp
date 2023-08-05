#include "bool_node.h"

namespace sigma {
	bool_node::bool_node(
		const file_range& range, bool 
		value
	) : node(range),
	m_value(value) {}

	outcome::result<value_ptr> bool_node::accept(
		code_generator_template& visitor, 
		const code_generation_context& context
	) {
		return visitor.visit_keyword_bool_node(*this, context);
	}

	void bool_node::print(
		u64 depth,
		const std::wstring& prefix,
		bool is_last
	) {
		const std::string value_string = m_value ? "true" : "false";

		print_node_name(
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