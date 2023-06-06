#include "variable_node.h"

namespace channel {
	variable_node::variable_node(
		const token_location& location,
		const std::string& variable_identifier
	) : node(location),
	m_variable_identifier(variable_identifier) {}

	expected_value variable_node::accept(code_generator& visitor, const code_generation_context& context) {
		return visitor.visit_variable_node(*this, context);
	}

	void variable_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_node_name(
			depth,
			prefix, 
			"variable", 
			is_last
		);

		console::out << "'" + m_variable_identifier + "'\n";
	}

	const std::string& variable_node::get_variable_identifier() const {
		return m_variable_identifier;
	}
}
