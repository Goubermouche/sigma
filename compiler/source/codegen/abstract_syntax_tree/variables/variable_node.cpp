#include "variable_node.h"

namespace channel {
	variable_node::variable_node(
		const token_position& position,
		const std::string& variable_identifier
	) : node(position),
	m_variable_identifier(variable_identifier) {}

	bool variable_node::accept(visitor& visitor, value_ptr& out_value, codegen_context context) {
		return visitor.visit_variable_node(*this, out_value, context);
	}

	void variable_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
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
