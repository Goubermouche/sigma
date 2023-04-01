#include "operator_multiplication_node.h"

namespace channel {
	operator_multiplication_node::operator_multiplication_node(node* left, node* right)
		: operator_binary(left, right) {}

	llvm::Value* operator_multiplication_node::accept(visitor& visitor) {
		return visitor.visit_operator_multiplication_node(*this);
	}
}