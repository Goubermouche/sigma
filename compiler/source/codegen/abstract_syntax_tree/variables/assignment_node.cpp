#include "assignment_node.h"

namespace channel {
	assignment_node::assignment_node(u64 line_number, node* variable, node* expression_node)
		: node(line_number), m_variable_node(variable), m_expression_node(expression_node) {}

	bool assignment_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(assignment_node);
		return visitor.visit_assignment_node(*this, out_value);
	}

	void assignment_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"VariableAssignment", "\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		m_variable_node->print(depth + 1, new_prefix, false);
		m_expression_node->print(depth + 1, new_prefix, true);
	}

	node* assignment_node::get_variable_node() const {
		return m_variable_node;
	}

	node* assignment_node::get_expression_node() const {
		return m_expression_node;
	}
}