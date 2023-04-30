#include "if_else_node.h"

namespace channel {
	if_else_node::if_else_node(u64 line_number, const std::vector<node_ptr>& condition_nodes, const std::vector<std::vector<node_ptr>>& branch_nodes)
		: node(line_number), m_condition_nodes(condition_nodes), m_branch_nodes(branch_nodes) {}

	bool if_else_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(if_else_node);
		return visitor.visit_if_else_node(*this, out_value);
	}

	void if_else_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"IfStmt", "\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		const bool has_trailing_else = m_condition_nodes.back() == nullptr;

		// print inner statements
		for (u64 i = 0; i < m_branch_nodes.size(); ++i) {
			const bool has_body_statements = !m_branch_nodes[i].empty();
			bool last;

			if (m_condition_nodes[i]) {
				if (i == m_condition_nodes.size() - 2) {
					last = has_body_statements ? false : !(has_trailing_else && !m_branch_nodes[i + 1].empty());
				}
				else {
					last = !has_body_statements && (i == m_condition_nodes.size() - 1);
				}

				m_condition_nodes[i]->print(depth + 1, new_prefix, last);
			}

			last = false;

			// print body nodes
			for (u64 j = 0; j < m_branch_nodes[i].size(); ++j) {

				if (i == m_condition_nodes.size() - 2 && has_trailing_else && m_branch_nodes[i + 1].empty()) {
					last = (j == m_branch_nodes[i].size() - 1) || m_branch_nodes[i].empty();
				}
				else if (i == m_condition_nodes.size() - 1 && j == m_branch_nodes[i].size() - 1) {
					last = true;
				}

				m_branch_nodes[i][j]->print(depth + 1, new_prefix, last);
			}
		}
	}

	const std::vector<node_ptr>& if_else_node::get_condition_nodes() {
		return m_condition_nodes;
	}

	const std::vector<std::vector<node_ptr>>& if_else_node::get_branch_nodes()	{
		return m_branch_nodes;
	}
}