#include "operator_division_node.h"

namespace channel {
	operator_division_node::operator_division_node(u64 line_index, node* left, node* right)
		: operator_binary(line_index, left, right) {}

	value* operator_division_node::accept(visitor& visitor) {
		LOG_NODE_NAME(operator_division_node);
		return visitor.visit_operator_division_node(*this);
	}

	std::string operator_division_node::get_node_name() const {
		return "operator_division_node";
	}
}