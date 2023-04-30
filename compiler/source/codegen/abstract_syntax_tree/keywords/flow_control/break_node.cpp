#include "break_node.h"

namespace channel {
	break_node::break_node(u64 line_number)
		: node(line_number) {}

	bool break_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(break_node);
		return visitor.visit_break_node(*this, out_value);
	}

	void break_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"BreakStmt", "\n", is_last);
	}
}