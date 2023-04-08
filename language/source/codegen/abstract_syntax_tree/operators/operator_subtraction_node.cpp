#include "operator_subtraction_node.h"

namespace channel {
	operator_subtraction_node::operator_subtraction_node(u64 line_index, node* left, node* right)
		: operator_binary(line_index, left, right) {}

	value* operator_subtraction_node::accept(visitor& visitor) {
		LOG_NODE_NAME(operator_subtraction_node);
		return visitor.visit_operator_subtraction_node(*this);
	}

	std::string operator_subtraction_node::get_node_name() const {
		return "operator_subtraction_node";
	}
}