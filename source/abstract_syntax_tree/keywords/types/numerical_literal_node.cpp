#include "numerical_literal_node.h"

namespace sigma {
	numerical_literal_node::numerical_literal_node(
		const utility::file_range& range,
		const std::string& value,
		type preferred_type
	) : node(range),
	m_value(value),
	m_preferred_type(preferred_type) {}

	utility::outcome::result<value_ptr> numerical_literal_node::accept(
		abstract_syntax_tree_visitor_template& visitor, 
		const code_generation_context& context
	) {
		return visitor.visit_numerical_literal_node(*this, context);
	}

	void numerical_literal_node::print(
		u64 depth,
		const std::wstring& prefix,
		bool is_last
	) {
		print_node_name(
			depth,
			prefix,
			"numerical literal",
			is_last
		);

		utility::console::out
			<< "'"
			<< AST_NODE_TYPE_COLOR
			<< m_preferred_type.to_string()
			<< utility::color::white
			<< "' '"
			<< AST_NODE_NUMERICAL_LITERAL_COLOR
			<< m_value
			<< utility::color::white
			<< "'\n";
	}

	const std::string& numerical_literal_node::get_value() const {
		return m_value;
	}

	const type& numerical_literal_node::get_preferred_type() const {
		return m_preferred_type;
	}
}