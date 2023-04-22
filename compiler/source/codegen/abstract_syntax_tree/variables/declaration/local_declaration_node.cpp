#include "local_declaration_node.h"

namespace channel {
	local_declaration_node::local_declaration_node(u64 line_number, const type& declaration_type, const std::string& declaration_identifier, node* expression_node)
		: declaration_node(line_number, declaration_type, declaration_identifier, expression_node) {}

	bool local_declaration_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(local_declaration_node);
		return visitor.visit_local_declaration_node(*this, out_value);
	}

	std::string local_declaration_node::get_node_name() const {
		return "local_declaration_node";
	}
}
