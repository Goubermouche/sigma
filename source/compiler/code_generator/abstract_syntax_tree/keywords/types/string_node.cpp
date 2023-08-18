#include "string_node.h"
#include <utility/string_helper.h>

namespace sigma {
	string_node::string_node(
		const file_range& range, 
		const std::string& value
	) : node(range),
	m_value(value) {}

	outcome::result<value_ptr> string_node::accept(
		code_generator_template& visitor, 
		const code_generation_context& context
	) {
		return visitor.visit_keyword_string_node(*this, context);
	}

	void string_node::print(
		u64 depth, 
		const std::wstring& prefix,
		bool is_last
	) {
		print_node_name(
			depth, 
			prefix, 
			"string literal", 
			is_last
		);

		console::out
			<< '\''
			<< AST_NODE_TYPE_COLOR
			<< "char"
			<< color::white
			<< '['
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value.size())
			<< color::white 
			<< "]' '"
			<< AST_NODE_TEXT_LITERAL_COLOR
			<< detail::escape_string(m_value)
			<< color::white
			<< "'\n";
	}

	const std::string& string_node::get_value() const {
		return m_value;
	}
}