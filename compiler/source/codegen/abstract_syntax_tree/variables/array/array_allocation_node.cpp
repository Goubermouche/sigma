#include "array_allocation_node.h"

namespace channel {
	array_allocation_node::array_allocation_node(u64 line_number, const type& array_element_type, node* array_element_count_node)
		: node(line_number), m_array_element_type(array_element_type), m_array_element_count(array_element_count_node) {}

	bool array_allocation_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(allocation_node);
		return visitor.visit_allocation_node(*this, out_value);
	}

	void array_allocation_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"ArrayAllocation", "'" + m_array_element_type.to_string() +"'\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		m_array_element_count->print(depth + 1, new_prefix, true);
	}

	const type& array_allocation_node::get_array_element_type() const {
		return m_array_element_type;
	}

	node* array_allocation_node::get_array_element_count_node() const {
		return m_array_element_count;
	}
}