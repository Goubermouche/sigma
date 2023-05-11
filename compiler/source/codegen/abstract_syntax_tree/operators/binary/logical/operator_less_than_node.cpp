#include "operator_less_than_node.h"

namespace channel {
	operator_less_than_node::operator_less_than_node(
		const token_position& position,
		const node_ptr& left_expression_node,
		const node_ptr& right_expression_node
	) : operator_binary(
		position,
		left_expression_node,
		right_expression_node
	) {}

	acceptation_result operator_less_than_node::accept(visitor& visitor, const codegen_context& context) {
		return visitor.visit_operator_less_than_node(*this, context);
	}

	void operator_less_than_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "binary operator", is_last);
		console::out
			<< '\''
			<< AST_NODE_OPERATOR_COLOR
			<< '<'
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		get_left_expression_node()->print(depth + 1, new_prefix, false);
		get_right_expression_node()->print(depth + 1, new_prefix, true);
	}
}