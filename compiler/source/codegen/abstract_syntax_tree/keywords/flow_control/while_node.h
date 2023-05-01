#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a while statement.
	 */
	class while_node : public node {
	public:
		while_node(
			u64 line_number,
			const node_ptr& loop_condition_node,
			const std::vector<node_ptr>& statement_nodes
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const node_ptr& get_loop_condition_node() const;
		const std::vector<node_ptr>& get_loop_body_nodes() const;
	private:
		node_ptr m_loop_condition_node;
		std::vector<node_ptr> m_loop_body_nodes;
	};
}