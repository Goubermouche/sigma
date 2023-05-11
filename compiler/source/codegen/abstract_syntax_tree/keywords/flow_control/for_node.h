#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a for statement.
	 */
	class for_node : public node {
	public:
		for_node(
			const token_position& position,
			const node_ptr& loop_initialization_node,
			const node_ptr& loop_condition_node,
			const std::vector<node_ptr>& post_iteration_nodes,
			const std::vector<node_ptr>& statement_nodes
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const node_ptr& get_loop_initialization_node() const;
		const node_ptr& get_loop_condition_node() const;
		const std::vector<node_ptr>& get_post_iteration_nodes() const;
		const std::vector<node_ptr>& get_loop_body_nodes() const;
	private:
		node_ptr m_loop_initialization_node;
		node_ptr m_loop_condition_node;
		std::vector<node_ptr> m_post_iteration_nodes;
		std::vector<node_ptr> m_loop_body_nodes;
	};
}