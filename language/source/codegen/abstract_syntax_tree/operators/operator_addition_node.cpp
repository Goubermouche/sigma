#include "operator_addition_node.h"

namespace channel {
	operator_addition_node::operator_addition_node(node* left, node* right)
		: operator_binary(left, right) {}

	llvm::Value* operator_addition_node::accept(visitor& visitor) {
		return visitor.visit_operator_addition_node(*this);
	}
}