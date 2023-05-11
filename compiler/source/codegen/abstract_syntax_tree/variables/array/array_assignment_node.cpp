#include "array_assignment_node.h"

namespace channel {
	array_assignment_node::array_assignment_node(
		const token_position& position,
		const node_ptr& array_base,
		const std::vector<node_ptr>& index_nodes,
		const node_ptr& expression_node
	) : node(position),
	m_array_base_node(array_base),
	m_array_element_index_nodes(index_nodes),
	m_expression_node(expression_node) {}

	acceptation_result array_assignment_node::accept(visitor& visitor, const codegen_context& context) {
		return visitor.visit_array_assignment_node(*this, context);
	}

	void array_assignment_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_node_name(depth, prefix, "array assignment", is_last);
		console::out << "\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		m_array_base_node->print(depth + 1, new_prefix, false);

		for (u64 i = 0; i < m_array_element_index_nodes.size(); ++i) {
			m_array_element_index_nodes[i]->print(depth + 1, new_prefix, false);
		}

		m_expression_node->print(depth + 1, new_prefix, true);
	}

	const node_ptr& array_assignment_node::get_array_base_node() const	{
		return m_array_base_node;
	}

	const std::vector<node_ptr>& array_assignment_node::get_array_element_index_nodes() const {
		return m_array_element_index_nodes;
	}

	const node_ptr& array_assignment_node::get_expression_node() const {
		return m_expression_node;
	}
}