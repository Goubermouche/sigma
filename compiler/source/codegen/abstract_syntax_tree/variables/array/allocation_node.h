#pragma once
#include "../../node.h"

namespace channel {
	class allocation_node : public node {
	public:
		allocation_node(u64 line_number, type element_type, node* array_size);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		type get_element_type() const;
		node* get_element_count_node() const;
	private:
		type m_element_type;
		node* m_array_size;
	};
}
