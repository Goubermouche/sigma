#include "operator_bitwise_not_node.h"

namespace sigma {
	operator_bitwise_not_node::operator_bitwise_not_node(
		const file_position& position,
		const node_ptr& expression_node
	) : operator_unary_base(position, expression_node) {}

	outcome::result<value_ptr> operator_bitwise_not_node::accept(
		code_generator_template& visitor,
		const code_generation_context& context
	) {
		return visitor.visit_operator_bitwise_not_node(*this, context);
	}

	void operator_bitwise_not_node::print(
		u64 depth, 
		const std::wstring& prefix, 
		bool is_last
	) {
		print_node_name(depth, prefix, "unary operator", is_last);
		console::out
			<< '\''
			<< AST_NODE_OPERATOR_COLOR
			<< "prefix"
			<< color::white
			<< "' '"
			<< AST_NODE_OPERATOR_COLOR
			<< "~"
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		get_expression_node()->print(depth + 1, new_prefix, true);
	}
}