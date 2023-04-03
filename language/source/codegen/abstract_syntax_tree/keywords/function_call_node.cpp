#include "function_call_node.h"

namespace channel {
	function_call_node::function_call_node(const std::string& name, const std::vector<node*>& arguments)
		: m_name(name), m_arguments(arguments) {}

	llvm::Value* function_call_node::accept(visitor& visitor) {
		std::cout << "accepting function_call_node\n";
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
