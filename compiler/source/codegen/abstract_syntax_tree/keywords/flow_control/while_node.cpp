#include "while_node.h"

namespace channel {
	while_node::while_node(u64 line_number, node_ptr loop_condition_node, const std::vector<node_ptr>& statement_nodes)
		: node(line_number), m_loop_condition_node(loop_condition_node), m_loop_body_nodes(statement_nodes) {}

	bool while_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(while_node);
		return visitor.visit_while_node(*this, out_value);
	}

	void while_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"WhileStmt", "\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		const bool has_body_statements = !m_loop_body_nodes.empty();

		// print inner statements
		m_loop_condition_node->print(depth + 1, new_prefix, !has_body_statements);

		for (u64 i = 0; i < m_loop_body_nodes.size(); ++i) {
			m_loop_body_nodes[i]->print(depth + 1, new_prefix, i == m_loop_body_nodes.size() - 1);
		}
	}

	node_ptr while_node::get_loop_condition_node() const {
		return m_loop_condition_node;
	}

	const std::vector<node_ptr>& while_node::get_loop_body_nodes() const {
		return m_loop_body_nodes;
	}
}