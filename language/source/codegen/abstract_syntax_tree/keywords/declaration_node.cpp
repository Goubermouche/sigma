#include "declaration_node.h"

namespace channel {
	declaration_node::declaration_node(const std::string& name, bool is_global, node* expression)
		: m_name(name), m_is_global(is_global), m_expression(expression) {}

	llvm::Value* declaration_node::accept(visitor& visitor) {
		return visitor.visit_declaration_node(*this);
	}

	std::string declaration_node::get_node_name() const {
		return "declaration_node";
	}

	const std::string& declaration_node::get_name() const {
		return m_name;
	}

	bool declaration_node::is_global() const {
		return m_is_global;
	}

	node* declaration_node::get_expression() const {
		return m_expression;
	}
}