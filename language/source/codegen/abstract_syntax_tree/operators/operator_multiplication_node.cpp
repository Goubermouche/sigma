#include "operator_multiplication_node.h"

namespace channel {
	operator_multiplication_node::operator_multiplication_node(u64 line_index, node* left, node* right)
		: operator_binary(line_index, left, right) {}

	value* operator_multiplication_node::accept(visitor& visitor) {
		LOG_NODE_NAME(operator_multiplication_node);
		return visitor.visit_operator_multiplication_node(*this);
	}

	std::string operator_multiplication_node::get_node_name() const {
		return "operator_multiplication_node";
	}
}