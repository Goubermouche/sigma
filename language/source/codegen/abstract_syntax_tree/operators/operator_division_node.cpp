#include "operator_division_node.h"

namespace channel {
	operator_division_node::operator_division_node(node* left, node* right)
		: operator_binary(left, right) {}

	llvm::Value* operator_division_node::accept(visitor& visitor) {
		std::cout << "accepting operator_division_node\n";
		return visitor.visit_operator_division_node(*this);
	}

	std::string operator_division_node::get_node_name() const {
		return "operator_division_node";
	}
}