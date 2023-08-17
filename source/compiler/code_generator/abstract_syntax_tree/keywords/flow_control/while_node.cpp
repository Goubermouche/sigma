#include "while_node.h"

namespace sigma {
	while_node::while_node(
		const file_range& range,
		const node_ptr& loop_condition_node,
		const std::vector<node_ptr>& statement_nodes
	) : node(range),
	m_loop_condition_node(loop_condition_node),
	m_loop_body_nodes(statement_nodes) {}

	outcome::result<value_ptr> while_node::accept(
		code_generator_template& visitor,
		const code_generation_context& context
	) {
		return visitor.visit_while_node(*this, context);
	}

	void while_node::print(
		u64 depth, 
		const std::wstring& prefix,
		bool is_last
	) {
		print_node_name(depth, prefix, "while statement", is_last);
		console::out << "\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		const bool has_body_statements = !m_loop_body_nodes.empty();

		// print inner statements
		m_loop_condition_node->print(depth + 1, new_prefix, !has_body_statements);

		for (u64 i = 0; i < m_loop_body_nodes.size(); ++i) {
			m_loop_body_nodes[i]->print(depth + 1, new_prefix, i == m_loop_body_nodes.size() - 1);
		}
	}

	const node_ptr& while_node::get_loop_condition_node() const {
		return m_loop_condition_node;
	}

	const std::vector<node_ptr>& while_node::get_loop_body_nodes() const {
		return m_loop_body_nodes;
	}
}