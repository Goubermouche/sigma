#include "u64_node.h"

namespace channel {
	u64_node::u64_node(
		const token_position& position,
		u64 value
	) : node(position),
	m_value(value) {}

	bool u64_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_u64_node);
		return visitor.visit_keyword_u64_node(*this, out_value);
	}

	void u64_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix, 
			"integral literal", 
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'u64' '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value)
			<< color::white
			<< "'\n";
	}

	u64 u64_node::get_value() const {
		return m_value;
	}
}