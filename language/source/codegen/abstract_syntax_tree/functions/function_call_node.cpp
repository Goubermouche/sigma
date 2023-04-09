#include "function_call_node.h"

namespace channel {
	function_call_node::function_call_node(u64 line_number, const std::string& name, const std::vector<node*>& arguments)
		: node(line_number), m_name(name), m_arguments(arguments) {}

	bool function_call_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(function_call_node);
		return visitor.visit_function_call_node(*this, out_value);
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