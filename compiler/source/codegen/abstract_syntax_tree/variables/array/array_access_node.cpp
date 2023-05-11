#include "array_access_node.h"

namespace channel {
	array_access_node::array_access_node(
		const token_position& position,
		const node_ptr& array_base,
		const std::vector<node_ptr>& array_element_index_nodes
	) : node(position),
	m_array_base(array_base),
	m_array_element_index_nodes(array_element_index_nodes) {}

	acceptation_result array_access_node::accept(visitor& visitor, const codegen_context& context) {
		return visitor.visit_array_access_node(*this, context);
	}

	void array_access_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "array access", is_last);
		console::out << "\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		m_array_base->print(depth + 1, new_prefix, false);

		for (u64 i = 0; i < m_array_element_index_nodes.size(); ++i) {
			m_array_element_index_nodes[i]->print(depth + 1, new_prefix, i == m_array_element_index_nodes.size() - 1);
		}
	}

	const node_ptr& array_access_node::get_array_base_node() const {
		return m_array_base;
	}

	const std::vector<node_ptr>& array_access_node::get_array_element_index_nodes() const {
		return m_array_element_index_nodes;
	}
}