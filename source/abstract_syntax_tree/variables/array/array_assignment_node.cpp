#include "array_assignment_node.h"

namespace sigma {
	array_assignment_node::array_assignment_node(
		const utility::file_range& range,
		const node_ptr& array_base,
		const std::vector<node_ptr>& index_nodes,
		const node_ptr& expression_node
	) : node(range),
	m_array_base_node(array_base),
	m_array_element_index_nodes(index_nodes),
	m_expression_node(expression_node) {}

	utility::outcome::result<value_ptr> array_assignment_node::accept(
		abstract_syntax_tree_visitor_template& visitor,
		const code_generation_context& context
	) {
		return visitor.visit_array_assignment_node(*this, context);
	}

	void array_assignment_node::print(
		u64 depth, 
		const std::wstring& prefix,
		bool is_last
	) {
		print_node_name(depth, prefix, "array assignment", is_last);
		utility::console::out << "\n";

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