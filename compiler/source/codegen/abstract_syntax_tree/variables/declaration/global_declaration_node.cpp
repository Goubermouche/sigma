#include "global_declaration_node.h"

namespace channel {
	global_declaration_node::global_declaration_node(u64 line_number, type declaration_type, const std::string& declaration_identifier, node* expression_node)
		: declaration_node(line_number, declaration_type, declaration_identifier, expression_node) {}

	bool global_declaration_node::accept(visitor& visitor, value*& out_value)	{
		LOG_NODE_NAME(global_declaration_node);
		return visitor.visit_global_declaration_node(*this, out_value);
	}

	std::string global_declaration_node::get_node_name() const {
		return "get_node_name";
	}
}