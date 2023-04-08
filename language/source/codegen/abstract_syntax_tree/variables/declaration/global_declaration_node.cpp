#include "global_declaration_node.h"

namespace channel {
	global_declaration_node::global_declaration_node(u64 line_index, type declaration_type, const std::string& name, node* expression)
		: declaration_node(line_index, declaration_type, name, expression) {}

	bool global_declaration_node::accept(visitor& visitor, value*& out_value)	{
		LOG_NODE_NAME(global_declaration_node);
		return visitor.visit_global_declaration_node(*this, out_value);
	}

	std::string global_declaration_node::get_node_name() const {
		return "get_node_name";
	}
}