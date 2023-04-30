#pragma once
#include "../../node.h"

namespace channel {
	class array_allocation_node : public node {
	public:
		array_allocation_node(u64 line_number, const type& array_element_type, node_ptr array_element_count_node);
		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const type& get_array_element_type() const;
		node_ptr get_array_element_count_node() const;
	private:
		type m_array_element_type;
		node_ptr m_array_element_count;
	};
}
