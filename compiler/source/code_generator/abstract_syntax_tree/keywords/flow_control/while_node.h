#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a while statement.
	 */
	class while_node : public node {
	public:
		while_node(
			const token_position& position,
			const node_ptr& loop_condition_node,
			const std::vector<node_ptr>& statement_nodes
		);

		acceptation_result accept(
			code_generator& visitor,
			const codegen_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix,
			bool is_last
		) override;

		const node_ptr& get_loop_condition_node() const;
		const std::vector<node_ptr>& get_loop_body_nodes() const;
	private:
		node_ptr m_loop_condition_node;
		std::vector<node_ptr> m_loop_body_nodes;
	};
}