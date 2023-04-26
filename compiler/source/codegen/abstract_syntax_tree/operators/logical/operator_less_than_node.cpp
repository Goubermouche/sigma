#include "operator_less_than_node.h"

namespace channel {
	operator_less_than_node::operator_less_than_node(u64 line_number, node* left_expression_node, node* right_expression_node)
		: operator_binary(line_number, left_expression_node, right_expression_node) {}

	bool operator_less_than_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_logical_disjunction_node);
		return visitor.visit_operator_less_than_node(*this, out_value);
	}

	std::string operator_less_than_node::get_node_name() const {
		return "operator_less_than_node";
	}
}