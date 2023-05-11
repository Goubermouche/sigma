#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a dynamic array allocation.
	 */
	class array_allocation_node : public node {
	public:
		array_allocation_node(
			const token_position& position,
			const type& array_element_type,
			const node_ptr& array_element_count_node
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const type& get_array_element_type() const;
		const node_ptr& get_array_element_count_node() const;
	private:
		type m_array_element_type;
		node_ptr m_array_element_count;
	};
}
