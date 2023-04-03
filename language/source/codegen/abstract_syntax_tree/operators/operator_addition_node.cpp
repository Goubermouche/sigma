#include "operator_addition_node.h"

namespace channel {
	operator_addition_node::operator_addition_node(node* left, node* right)
		: operator_binary(left, right) {}

	llvm::Value* operator_addition_node::accept(visitor& visitor) {
		LOG_NODE_NAME(operator_addition_node);
		return visitor.visit_operator_addition_node(*this);
	}

	std::string operator_addition_node::get_node_name() const {
		return "operator_addition_node";
	}
}