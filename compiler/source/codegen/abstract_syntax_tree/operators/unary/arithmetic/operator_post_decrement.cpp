#include "operator_post_decrement.h"

namespace channel {
	operator_post_decrement::operator_post_decrement(
		const token_position& position,
		const node_ptr& expression_node
	) : operator_unary(position, expression_node) {}

	bool operator_post_decrement::accept(visitor& visitor, value_ptr& out_value, codegen_context context) {
		return visitor.visit_operator_post_decrement_node(*this, out_value, context);
	}

	void operator_post_decrement::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "unary operator", is_last);
		console::out
			<< '\''
			<< AST_NODE_OPERATOR_COLOR
			<< "postfix"
			<< color::white
			<< "' '"
			<< AST_NODE_OPERATOR_COLOR
			<< "--"
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		get_expression_node()->print(depth + 1, new_prefix, true);
	}
}
