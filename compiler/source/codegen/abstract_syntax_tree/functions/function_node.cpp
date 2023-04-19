#include "function_node.h"

namespace channel {
	function_node::function_node(u64 line_number, type function_return_type, const std::string& function_identifier, const std::vector<node*>& function_statements, const std::vector<std::pair<std::string, type>>& function_arguments)
		: node(line_number), m_function_return_type(function_return_type), m_function_identifier(function_identifier), m_function_statements(function_statements), m_function_arguments(function_arguments) {}

	bool function_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(function_node);
		return visitor.visit_function_node(*this, out_value);
	}

	std::string function_node::get_node_name() const {
		return "function_node";
	}

	type function_node::get_function_return_type() const {
		return m_function_return_type;
	}

	const std::string& function_node::get_function_identifier() const {
		return m_function_identifier;
	}

	const std::vector<node*>& function_node::get_function_statements() const	{
		return m_function_statements;
	}

	const std::vector<std::pair<std::string, type>>& function_node::get_function_arguments() const {
		return m_function_arguments;
	}
}
