#include "local_declaration_node.h"

namespace channel {
	local_declaration_node::local_declaration_node(u64 line_index, type declaration_type, const std::string& name, node* expression)
		: declaration_node(line_index, declaration_type, name, expression) {}

	value* local_declaration_node::accept(visitor& visitor) {
		LOG_NODE_NAME(local_declaration_node);
		return visitor.visit_local_declaration_node(*this);
	}

	std::string local_declaration_node::get_node_name() const {
		return "local_declaration_node";
	}
}
