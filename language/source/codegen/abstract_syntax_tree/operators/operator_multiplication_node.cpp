#include "operator_multiplication_node.h"

namespace channel {
	operator_multiplication_node::operator_multiplication_node(node* left, node* right)
		: operator_binary(left, right) {}

	llvm::Value* operator_multiplication_node::accept(visitor& visitor) {
		LOG_NODE_NAME(operator_multiplication_node);
		return visitor.visit_operator_multiplication_node(*this);
	}

	std::string operator_multiplication_node::get_node_name() const {
		return "operator_multiplication_node";
	}
}