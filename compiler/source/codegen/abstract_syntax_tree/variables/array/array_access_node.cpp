#include "array_access_node.h"

namespace channel {
	array_access_node::array_access_node(u64 line_number, node* array_base, const std::vector<node*>& array_element_index_nodes)
		: node(line_number), m_array_base(array_base), m_array_element_index_nodes(array_element_index_nodes) {}

	bool array_access_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(array_access_node);
		return visitor.visit_array_access_node(*this, out_value);
	}

	void array_access_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"ArrayAccess", "\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statements
		m_array_base->print(depth + 1, new_prefix, false);

		for (u64 i = 0; i < m_array_element_index_nodes.size(); ++i) {
			m_array_element_index_nodes[i]->print(depth + 1, new_prefix, i == m_array_element_index_nodes.size() - 1);
		}
	}

	node* array_access_node::get_array_base_node() const {
		return m_array_base;
	}

	const std::vector<node*>& array_access_node::get_array_element_index_nodes() const {
		return m_array_element_index_nodes;
	}
}