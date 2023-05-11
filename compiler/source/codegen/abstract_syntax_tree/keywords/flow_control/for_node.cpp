#include "for_node.h"

namespace channel {
	for_node::for_node(
		const token_position& position,
		const node_ptr& loop_initialization_node,
		const node_ptr& loop_condition_node,
		const std::vector<node_ptr>& post_iteration_nodes,
		const std::vector<node_ptr>& statement_nodes
	) : node(position),
	m_loop_initialization_node(loop_initialization_node),
	m_loop_condition_node(loop_condition_node),
	m_post_iteration_nodes(post_iteration_nodes),
	m_loop_body_nodes(statement_nodes) {}

	acceptation_result for_node::accept(visitor& visitor, const codegen_context& context) {
		return visitor.visit_for_node(*this, context);
	}

	void for_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "for statement", is_last);
		console::out << "\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		const bool has_post_iteration_nodes = !m_post_iteration_nodes.empty();
		const bool has_loop_body_nodes = !m_loop_body_nodes.empty();

		// print inner statements
		m_loop_initialization_node->print(depth + 1, new_prefix, false);
		m_loop_condition_node->print(depth + 1, new_prefix, !(has_post_iteration_nodes || has_loop_body_nodes));

		for (u64 i = 0; i < m_post_iteration_nodes.size(); ++i) {
			m_post_iteration_nodes[i]->print(depth + 1, new_prefix, has_loop_body_nodes ? false : i == m_post_iteration_nodes.size() - 1);
		}

		for (u64 i = 0; i < m_loop_body_nodes.size(); ++i) {
			m_loop_body_nodes[i]->print(depth + 1, new_prefix, i == m_loop_body_nodes.size() - 1);
		}
	}

	const node_ptr& for_node::get_loop_initialization_node() const {
		return m_loop_initialization_node;
	}

	const node_ptr& for_node::get_loop_condition_node() const {
		return m_loop_condition_node;
	}

	const std::vector<node_ptr>& for_node::get_post_iteration_nodes() const	{
		return m_post_iteration_nodes;
	}

	const std::vector<node_ptr>& for_node::get_loop_body_nodes() const {
		return m_loop_body_nodes;
	}
}