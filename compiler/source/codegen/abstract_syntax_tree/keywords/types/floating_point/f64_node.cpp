#include "f64_node.h"

namespace channel {
	f64_node::f64_node(u64 line_number, f64 value)
		: node(line_number), m_value(value) {}

	bool f64_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(keyword_f64_node);
		return visitor.visit_keyword_f64_node(*this, out_value);
	}

	void f64_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth,
			prefix, 
			"floating literal",
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'f64' '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< std::to_string(m_value)
			<< color::white
			<< "'\n";
	}

	f64 f64_node::get_value() const {
		return m_value;
	}
}