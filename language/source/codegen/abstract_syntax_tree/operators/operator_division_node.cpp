#include "operator_division_node.h"

namespace channel {
	operator_division_node::operator_division_node(node* left, node* right)
		: operator_binary(left, right) {}

	llvm::Value* operator_division_node::accept(visitor& visitor) {
		return visitor.visit_operator_division_node(*this);
	}
}