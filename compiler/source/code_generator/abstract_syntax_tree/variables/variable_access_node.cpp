#include "variable_access_node.h"

namespace channel {
	variable_access_node::variable_access_node(
		const token_position& position,
		const std::string& variable_identifier
	) : node(position),
	m_variable_identifier(variable_identifier) {}

	acceptation_result variable_access_node::accept(code_generator& visitor, const codegen_context& context) {
		return visitor.visit_variable_access_node(*this, context);
	}

	void variable_access_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_node_name(
			depth, 
			prefix, 
			"variable access", 
			is_last
		);

		console::out
			<< "'"
			<< AST_NODE_VARIABLE_COLOR
			<< m_variable_identifier 
			<< color::white
			<< "'\n";
	}

	const std::string& variable_access_node::get_variable_identifier() const {
		return m_variable_identifier;
	}
}