#include "operator_division_node.h"

namespace channel {
	operator_division_node::operator_division_node(u64 line_number, node* left_expression_node, node* right_expression_node)
		: operator_binary(line_number, left_expression_node, right_expression_node) {}

	bool operator_division_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_division_node);
		return visitor.visit_operator_division_node(*this, out_value);
	}

	void operator_division_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"BinaryOperator", "'/'\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		get_left_expression_node()->print(depth + 1, new_prefix, false);
		get_right_expression_node()->print(depth + 1, new_prefix, true);
	}
}