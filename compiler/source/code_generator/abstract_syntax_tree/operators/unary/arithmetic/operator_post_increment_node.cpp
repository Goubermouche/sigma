#include "operator_post_increment_node.h"

namespace sigma {
	operator_post_increment_node::operator_post_increment_node(
		const file_position& location,
		const node_ptr& expression_node
	) : operator_unary_base(location, expression_node) {}

	expected_value operator_post_increment_node::accept(code_generator& visitor, const code_generation_context& context) {
		return visitor.visit_operator_post_increment_node(*this, context);
	}

	void operator_post_increment_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_node_name(depth, prefix, "unary operator", is_last);
		console::out
			<< '\''
			<< AST_NODE_OPERATOR_COLOR
			<< "postfix"
			<< color::white
			<< "' '"
			<< AST_NODE_OPERATOR_COLOR
			<< "++"
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		get_expression_node()->print(depth + 1, new_prefix, true);
	}
}
