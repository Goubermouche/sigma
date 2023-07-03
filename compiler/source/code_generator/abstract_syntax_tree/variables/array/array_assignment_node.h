#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, assignment to an array.
	 */
	class array_assignment_node : public node {
	public:
		array_assignment_node(
			const token_location& location,
			const node_ptr& array_base,
			const std::vector<node_ptr>& index_nodes,
			const node_ptr& expression_node
		);

		expected_value accept(
			code_generator& visitor,
			const code_generation_context& context
		) override;

		void print(u64 depth, const std::wstring& prefix, bool is_last) override;

		const node_ptr& get_array_base_node() const;
		const std::vector<node_ptr>& get_array_element_index_nodes() const;
		const node_ptr& get_expression_node() const;
	private:
		node_ptr m_array_base_node;
		std::vector<node_ptr> m_array_element_index_nodes;
		node_ptr m_expression_node;
	};
}