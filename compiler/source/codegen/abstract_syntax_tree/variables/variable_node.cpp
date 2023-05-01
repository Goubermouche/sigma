#include "variable_node.h"

namespace channel {
	variable_node::variable_node(
		u64 line_number, 
		const std::string& variable_identifier
	) : node(line_number),
	m_variable_identifier(variable_identifier) {}

	bool variable_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(variable_node);
		return visitor.visit_variable_node(*this, out_value);
	}

	void variable_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth,
			prefix, 
			"variable", 
			"'" + m_variable_identifier +"'\n",
			is_last
		);
	}

	const std::string& variable_node::get_variable_identifier() const {
		return m_variable_identifier;
	}
}
