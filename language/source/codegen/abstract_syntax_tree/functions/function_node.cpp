#include "function_node.h"

namespace channel {
	function_node::function_node(u64 line_number, type return_type, const std::string& name, std::vector<node*> statements, std::vector<std::pair<std::string, type>> arguments)
		: node(line_number), m_return_type(return_type), m_name(name), m_statements(std::move(statements)), m_arguments(std::move(arguments)) {}

	bool function_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(function_node);
		return visitor.visit_function_node(*this, out_value);
	}

	std::string function_node::get_node_name() const {
		return "function_node";
	}

	type function_node::get_return_type() const {
		return m_return_type;
	}

	const std::string& function_node::get_name() const {
		return m_name;
	}

	const std::vector<node*>& function_node::get_statements() const	{
		return m_statements;
	}

	const std::vector<std::pair<std::string, type>>& function_node::get_arguments() const {
		return m_arguments;
	}
}
