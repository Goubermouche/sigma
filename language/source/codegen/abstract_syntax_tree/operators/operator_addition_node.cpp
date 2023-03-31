#include "operator_addition_node.h"

namespace channel {
	operator_addition_node::operator_addition_node(node* left, node* right)
		: operator_binary(left, right) {}

	void operator_addition_node::accept(visitor& visitor) {
		visitor.visit_operator_addition_node(*this);
	}
}