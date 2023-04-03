#include "function_node.h"

namespace channel {
	function_node::function_node(const std::string& return_type, const std::string& name, std::vector<node*> statements)
		: m_return_type(return_type), m_name(name), m_statements(std::move(statements)) {}

	llvm::Value* function_node::accept(visitor& visitor) {
		std::cout << "accepting function_node\n";
		return visitor.visit_function_node(*this);
	}

	std::string function_node::get_node_name() const {
		return "function_node";
	}

	const std::string& function_node::get_return_type() const {
		return m_return_type;
	}

	const std::string& function_node::get_name() const {
		return m_name;
	}

	const std::vector<node*>& function_node::get_statements() const	{
		return m_statements;
	}
}
