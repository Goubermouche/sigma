#include "char_node.h"

namespace channel {
	char_node::char_node(
		const token_position& position,
		char value
	) : node(position),
	m_value(value)	{}

	acceptation_result char_node::accept(visitor& visitor, const codegen_context& context)	{
		return visitor.visit_keyword_char_node(*this, context);
	}

	void char_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_node_name(
			depth,
			prefix, 
			"character literal", 
			is_last
		);

		console::out
			<< '\''
			<< AST_NODE_TYPE_COLOR
			<< "char"
			<< color::white 
			<< "' '"
			<< AST_NODE_TEXT_LITERAL_COLOR
			<< escape_string(std::string(1, m_value))
			<< color::white
			<< "'\n";
	}

	char char_node::get_value() const {
		return m_value;
	}
}