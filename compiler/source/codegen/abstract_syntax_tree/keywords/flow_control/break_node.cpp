#include "break_node.h"

namespace channel {
	break_node::break_node(const token_position& position)
		: node(position) {}

	bool break_node::accept(visitor& visitor, value_ptr& out_value, codegen_context context) {
		return visitor.visit_break_node(*this, out_value, context);
	}

	void break_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "break statement", is_last);
		console::out << "\n";
	}
}