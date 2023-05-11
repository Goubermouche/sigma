#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class if_else_node : public node {
	public:
		if_else_node(
			const token_position& position,
			const std::vector<node_ptr>& condition_nodes,
			const std::vector<std::vector<node_ptr>>& branch_nodes
		);

		bool accept(visitor& visitor, value_ptr& out_value, codegen_context context) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const std::vector<node_ptr>& get_condition_nodes();
		const std::vector<std::vector<node_ptr>>& get_branch_nodes();
	private:
		std::vector<node_ptr> m_condition_nodes;
		std::vector<std::vector<node_ptr>> m_branch_nodes;
	};
}