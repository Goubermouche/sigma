#include "operator_addition_node.h"

namespace channel {
	operator_addition_node::operator_addition_node(
		const token_position& position,
		const node_ptr& left_expression_node,
		const node_ptr& right_expression_node
	) : operator_binary(
		position,
		left_expression_node,
		right_expression_node
	) {}

	bool operator_addition_node::accept(visitor& visitor, value_ptr& out_value, codegen_context context) {
		return visitor.visit_operator_addition_node(*this, out_value, context);
	}

	void operator_addition_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "binary operator", is_last);
		console::out
			<< '\''
			<< AST_NODE_OPERATOR_COLOR
			<< '+'
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		get_left_expression_node()->print(depth + 1, new_prefix, false);
		get_right_expression_node()->print(depth + 1, new_prefix, true);
	}
}