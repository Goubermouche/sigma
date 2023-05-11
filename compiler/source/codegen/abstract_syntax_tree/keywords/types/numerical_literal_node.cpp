#include "numerical_literal_node.h"

namespace channel {
	numerical_literal_node::numerical_literal_node(
		const token_position& position,
		const std::string& value,
		type preferred_type
	) : node(position),
	m_value(value),
	m_preferred_type(preferred_type) {}

	bool numerical_literal_node::accept(visitor& visitor, value_ptr& out_value, codegen_context context) {
		return visitor.visit_numerical_literal_node(*this, out_value, context);
	}

	void numerical_literal_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth,
			prefix,
			"numerical literal",
			is_last
		);

		console::out
			<< AST_NODE_VARIABLE_COLOR
			<< "'number' '"
			<< m_preferred_type.to_string()
			<< "'  '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< m_value
			<< color::white
			<< "'\n";
	}

	const std::string& numerical_literal_node::get_value() const {
		return m_value;
	}

	const type& numerical_literal_node::get_preferred_type() const {
		return m_preferred_type;
	}
}