#include "function_call_node.h"

namespace channel {
	function_call_node::function_call_node(u64 line_index, const std::string& name, const std::vector<node*>& arguments)
		: node(line_index), m_name(name), m_arguments(arguments) {}

	value* function_call_node::accept(visitor& visitor) {
		LOG_NODE_NAME(function_call_node);
		return visitor.visit_function_call_node(*this);
	}

	std::string function_call_node::get_node_name() const {
		return "function_call_node";
	}

	const std::string& function_call_node::get_name() const	{
		return m_name;
	}
	const std::vector<node*>& function_call_node::get_arguments() const	{
		return m_arguments;
	}
}
