#include "bool_node.h"

namespace sigma {
	bool_node::bool_node(
		const utility::file_range& range, bool
		value
	) : node(range),
	m_value(value) {}

	utility::outcome::result<value_ptr> bool_node::accept(
		abstract_syntax_tree_visitor_template& visitor, 
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

		utility::console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'bool' '"
			<< AST_NODE_BOOLEAN_LITERAL_COLOR
			<< value_string
			<< utility::color::white
			<< "'\n";
	}

	bool bool_node::get_value() const {
		return m_value;
	}
}