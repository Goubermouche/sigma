#include "operator_subtraction_node.h"

namespace channel {
	operator_subtraction_node::operator_subtraction_node(u64 line_index, node* left, node* right)
		: operator_binary(line_index, left, right) {}

	bool operator_subtraction_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_subtraction_node);
		return visitor.visit_operator_subtraction_node(*this, out_value);
	}

	std::string operator_subtraction_node::get_node_name() const {
		return "operator_subtraction_node";
	}
}