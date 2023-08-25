#include "variable_access_node.h"

namespace sigma {
	variable_access_node::variable_access_node(
		const utility::file_range& range,
		const std::string& identifier
	) : node(range),
	m_identifier(identifier) {}

	utility::outcome::result<value_ptr> variable_access_node::accept(
		abstract_syntax_tree_visitor_template& visitor,
		const code_generation_context& context
	) {
		return visitor.visit_variable_access_node(*this, context);
	}

	void variable_access_node::print(
		u64 depth, 
		const std::wstring& prefix, 
		bool is_last
	) {
		print_node_name(
			depth, 
			prefix, 
			"variable access", 
			is_last
		);

		utility::console::out
			<< "'"
			<< AST_NODE_VARIABLE_COLOR
			<< m_identifier 
			<< utility::color::white
			<< "'\n";
	}

	const std::string& variable_access_node::get_identifier() const {
		return m_identifier;
	}
}