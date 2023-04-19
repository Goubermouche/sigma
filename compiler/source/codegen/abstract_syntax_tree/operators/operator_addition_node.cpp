#include "operator_addition_node.h"

namespace channel {
	operator_addition_node::operator_addition_node(u64 line_number, node* left_expression_node, node* right_expression_node)
		: operator_binary(line_number, left_expression_node, right_expression_node) {}

	bool operator_addition_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_addition_node);
		return visitor.visit_operator_addition_node(*this, out_value);
	}

	std::string operator_addition_node::get_node_name() const {
		return "operator_addition_node";
	}
}