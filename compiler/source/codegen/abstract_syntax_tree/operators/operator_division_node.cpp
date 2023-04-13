#include "operator_division_node.h"

namespace channel {
	operator_division_node::operator_division_node(u64 line_number, node* left, node* right)
		: operator_binary(line_number, left, right) {}

	bool operator_division_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_division_node);
		return visitor.visit_operator_division_node(*this, out_value);
	}

	std::string operator_division_node::get_node_name() const {
		return "operator_division_node";
	}
}