#include "operator_subtraction_node.h"

namespace channel {
	operator_subtraction_node::operator_subtraction_node(u64 line_number, node_ptr left, node_ptr right)
		: operator_binary(line_number, left, right) {}

	bool operator_subtraction_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(operator_subtraction_node);
		return visitor.visit_operator_subtraction_node(*this, out_value);
	}

	void operator_subtraction_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"BinaryOperator", "'-'\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		get_left_expression_node()->print(depth + 1, new_prefix, false);
		get_right_expression_node()->print(depth + 1, new_prefix, true);
	}
}