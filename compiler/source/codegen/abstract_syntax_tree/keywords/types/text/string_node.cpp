#include "string_node.h"	

namespace channel {
	string_node::string_node(
		const token_position& position, 
		const std::string& value
	) : node(position),
	m_value(value) {}

	bool string_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(string_node);
		return visitor.visit_keyword_string_node(*this, out_value);
	}

	void string_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix, 
			"string literal", 
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< '\''
			<< "char"
			<< color::white
			<< '['
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value.size())
			<< color::white 
			<< "]' '"
			<< AST_NODE_TEXT_LITERAL_COLOR
			<< escape_string(m_value)
			<< color::white
			<< "'\n";
	}

	const std::string& string_node::get_value() const {
		return m_value;
	}
}