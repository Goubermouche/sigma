#pragma once
#include "abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a function.
	 */
	class if_else_node : public node {
	public:
		if_else_node(
			const utility::file_range& range,
			const std::vector<node_ptr>& condition_nodes,
			const std::vector<std::vector<node_ptr>>& branch_nodes
		);

		utility::outcome::result<value_ptr> accept(
			abstract_syntax_tree_visitor_template& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth, 
			const std::wstring& prefix,
			bool is_last
		) override;

		const std::vector<node_ptr>& get_condition_nodes();
		const std::vector<std::vector<node_ptr>>& get_branch_nodes();
	private:
		std::vector<node_ptr> m_condition_nodes;
		std::vector<std::vector<node_ptr>> m_branch_nodes;
	};
}