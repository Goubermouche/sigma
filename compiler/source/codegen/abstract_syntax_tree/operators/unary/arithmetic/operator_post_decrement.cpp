#include "operator_post_decrement.h"

namespace channel {
	operator_post_decrement::operator_post_decrement(u64 line_number, node* expression_node)
		: operator_unary(line_number, expression_node) {}

	bool operator_post_decrement::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_post_decrement);
		return visitor.visit_operator_post_decrement_node(*this, out_value);
	}

	void operator_post_decrement::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"UnaryOperator", "'postfix' '--'\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		get_expression_node()->print(depth + 1, new_prefix, true);
	}
}
