#include "operator_pre_increment.h"

namespace channel {
	operator_pre_increment::operator_pre_increment(u64 line_number, node* expression_node)
		: operator_unary(line_number, expression_node) {}

	bool operator_pre_increment::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_pre_increment);
		return visitor.visit_operator_pre_increment_node(*this, out_value);
	}

	void operator_pre_increment::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"UnaryOperator", "'prefix' '++'\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		get_expression_node()->print(depth + 1, new_prefix, true);
	}
}