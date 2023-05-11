#include "string_node.h"	

namespace channel {
	string_node::string_node(
		const token_position& position, 
		const std::string& value
	) : node(position),
	m_value(value) {}

	acceptation_result string_node::accept(visitor& visitor, const codegen_context& context) {
		return visitor.visit_keyword_string_node(*this, context);
	}

	void string_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_node_name(
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