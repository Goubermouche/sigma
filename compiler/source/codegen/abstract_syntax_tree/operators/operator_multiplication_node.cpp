#include "operator_multiplication_node.h"

namespace channel {
	operator_multiplication_node::operator_multiplication_node(u64 line_number, node* left, node* right)
		: operator_binary(line_number, left, right) {}

	bool operator_multiplication_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_multiplication_node);
		return visitor.visit_operator_multiplication_node(*this, out_value);
	}

	std::string operator_multiplication_node::get_node_name() const {
		return "operator_multiplication_node";
	}
}