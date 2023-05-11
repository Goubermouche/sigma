#include "assignment_node.h"

namespace channel {
	assignment_node::assignment_node(
		const token_position& position,
		const node_ptr& variable,
		const node_ptr& expression_node
	) : node(position),
	m_variable_node(variable),
	m_expression_node(expression_node) {}

	bool assignment_node::accept(visitor& visitor, value_ptr& out_value, codegen_context context) {
		return visitor.visit_assignment_node(*this, out_value, context);
	}

	void assignment_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "variable assignment", is_last);
		console::out << "\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		m_variable_node->print(depth + 1, new_prefix, false);
		m_expression_node->print(depth + 1, new_prefix, true);
	}

	const node_ptr& assignment_node::get_variable_node() const {
		return m_variable_node;
	}

	const node_ptr& assignment_node::get_expression_node() const {
		return m_expression_node;
	}
}