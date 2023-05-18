#include "operator_addition_assignment_node.h"

namespace channel {
	operator_addition_assignment_node::operator_addition_assignment_node(
		const token_position& position,
		const node_ptr& left_expression_node,
		const node_ptr& right_expression_node
	) : operator_binary_base(
		position,
		left_expression_node,
		right_expression_node
	) {}

	acceptation_result operator_addition_assignment_node::accept(code_generator& visitor, const code_generation_context& context) {
		return visitor.visit_operator_addition_assignment_node(*this, context);
	}

	void operator_addition_assignment_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_node_name(depth, prefix, "binary operator", is_last);
		console::out
			<< '\''
			<< AST_NODE_OPERATOR_COLOR
			<< "+="
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		get_left_expression_node()->print(depth + 1, new_prefix, false);
		get_right_expression_node()->print(depth + 1, new_prefix, true);
	}
}