#include "char_node.h"
#include "string_helper.h"

namespace sigma {
	char_node::char_node(
		const file_range& range,
		char value
	) : node(range),
	m_value(value)	{}

	outcome::result<value_ptr> char_node::accept(
		code_generator_template& visitor,
		const code_generation_context& context
	)	{
		return visitor.visit_keyword_char_node(*this, context);
	}

	void char_node::print(
		u64 depth,
		const std::wstring& prefix, 
		bool is_last
	) {
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
			<< detail::escape_string(std::string(1, m_value))
			<< color::white
			<< "'\n";
	}

	char char_node::get_value() const {
		return m_value;
	}
}