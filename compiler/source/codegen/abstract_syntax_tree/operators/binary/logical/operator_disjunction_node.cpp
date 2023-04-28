#include "operator_disjunction_node.h"

namespace channel {
	operator_disjunction_node::operator_disjunction_node(u64 line_number, node* left_expression_node, node* right_expression_node)
		: operator_binary(line_number, left_expression_node, right_expression_node) {}

	bool operator_disjunction_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_logical_disjunction_node);
		return visitor.visit_operator_logical_disjunction_node(*this, out_value);
	}

	std::string operator_disjunction_node::get_node_name() const {
		return "operator_logical_disjunction_node";
	}
}