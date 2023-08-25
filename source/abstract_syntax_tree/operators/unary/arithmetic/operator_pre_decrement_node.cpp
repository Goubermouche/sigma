#include "operator_pre_decrement_node.h"

namespace sigma {
	operator_pre_decrement_node::operator_pre_decrement_node(
		const utility::file_range& range,
		const node_ptr& expression_node
	) : operator_unary_base(range, expression_node) {}

	utility::outcome::result<value_ptr> operator_pre_decrement_node::accept(
		abstract_syntax_tree_visitor_template& visitor, 
		const code_generation_context& context
	) {
		return visitor.visit_operator_pre_decrement_node(*this, context);
	}

	void operator_pre_decrement_node::print(
		u64 depth, 
		const std::wstring& prefix,
		bool is_last
	) {
		print_node_name(depth, prefix, "unary operator", is_last);
		utility::console::out
			<< '\''
			<< AST_NODE_OPERATOR_COLOR
			<< "prefix"
			<< utility::color::white
			<< "' '"
			<< AST_NODE_OPERATOR_COLOR
			<< "--"
			<< utility::color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		get_expression_node()->print(depth + 1, new_prefix, true);
	}
}