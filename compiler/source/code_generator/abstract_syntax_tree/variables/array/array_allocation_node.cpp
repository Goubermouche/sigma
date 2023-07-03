#include "array_allocation_node.h"

namespace sigma {
	array_allocation_node::array_allocation_node(
		const token_location& location,
		const type& array_element_type, 
		const node_ptr& array_element_count_node
	) : node(location),
	m_array_element_type(array_element_type),
	m_array_element_count(array_element_count_node) {}

	expected_value array_allocation_node::accept(code_generator& visitor, const code_generation_context& context) {
		return visitor.visit_allocation_node(*this, context);
	}

	void array_allocation_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_node_name(
			depth, 
			prefix, 
			"array allocation", 
			is_last
		);

		console::out << "'" + m_array_element_type.to_string() + "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		m_array_element_count->print(depth + 1, new_prefix, true);
	}

	const type& array_allocation_node::get_array_element_type() const {
		return m_array_element_type;
	}

	const node_ptr& array_allocation_node::get_array_element_count_node() const {
		return m_array_element_count;
	}
}