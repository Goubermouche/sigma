#include "operator_conjunction_node.h"

namespace channel {
	operator_conjunction_node::operator_conjunction_node(u64 line_number, node* left_expression_node, node* right_expression_node)
		: operator_binary(line_number, left_expression_node, right_expression_node) {}

	bool operator_conjunction_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_logical_conjunction_node);
		return visitor.visit_operator_logical_conjunction_node(*this, out_value);
	}

	std::string operator_conjunction_node::get_node_name() const {
		return "operator_logical_conjunction_node";
	}
}