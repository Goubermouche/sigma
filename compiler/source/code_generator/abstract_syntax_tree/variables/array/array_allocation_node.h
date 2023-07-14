#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a dynamic array allocation.
	 */
	class array_allocation_node : public node {
	public:
		array_allocation_node(
			const file_position& location,
			const type& array_element_type,
			const node_ptr& array_element_count_node
		);

		outcome::result<value_ptr> accept(
			code_generator_template& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth, 
			const std::wstring& prefix,
			bool is_last
		) override;

		const type& get_array_element_type() const;
		const node_ptr& get_array_element_count_node() const;
	private:
		type m_array_element_type;
		node_ptr m_array_element_count;
	};
}
