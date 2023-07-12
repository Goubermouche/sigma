#include "operator_disjunction_node.h"

namespace sigma {
	operator_disjunction_node::operator_disjunction_node(
		const file_position& location,
		const node_ptr& left_expression_node,
		const node_ptr& right_expression_node
	) : operator_binary_base(
		location,
		left_expression_node,
		right_expression_node
	) {}

	outcome::result<value_ptr> operator_disjunction_node::accept(
		code_generator& visitor, 
		const code_generation_context& context
	) {
		return visitor.visit_operator_logical_disjunction_node(*this, context);
	}

	void operator_disjunction_node::print(
		u64 depth, 
		const std::wstring& prefix, 
		bool is_last
	) {
		print_node_name(depth, prefix, "binary operator", is_last);
		console::out
			<< '\''
			<< AST_NODE_OPERATOR_COLOR
			<< "||"
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		get_left_expression_node()->print(depth + 1, new_prefix, false);
		get_right_expression_node()->print(depth + 1, new_prefix, true);
	}
}