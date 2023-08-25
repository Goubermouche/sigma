#include "variable_node.h"

namespace sigma {
	variable_node::variable_node(
		const utility::file_range& range,
		const std::string& identifier
	) : node(range),
	m_identifier(identifier) {}

	utility::outcome::result<value_ptr> variable_node::accept(
		abstract_syntax_tree_visitor_template& visitor, 
		const code_generation_context& context
	) {
		return visitor.visit_variable_node(*this, context);
	}

	void variable_node::print(
		u64 depth, 
		const std::wstring& prefix, 
		bool is_last
	) {
		print_node_name(
			depth,
			prefix, 
			"variable", 
			is_last
		);

		utility::console::out << "'" + m_identifier + "'\n";
	}

	const std::string& variable_node::get_identifier() const {
		return m_identifier;
	}
}
