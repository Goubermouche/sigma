#include "global_declaration_node.h"

namespace channel {
	global_declaration_node::global_declaration_node(const std::string& name, node* expression)
		: declaration_node(name, expression) {}

	llvm::Value* global_declaration_node::accept(visitor& visitor)	{
		LOG_NODE_NAME(global_declaration_node);
		return visitor.visit_global_declaration_node(*this);
	}

	std::string global_declaration_node::get_node_name() const {
		return "get_node_name";
	}
}