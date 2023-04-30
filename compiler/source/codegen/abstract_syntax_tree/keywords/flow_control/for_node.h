#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class for_node : public node {
	public:
		for_node(
			u64 line_number,
			node_ptr loop_initialization_node,
			node_ptr loop_condition_node,
			const std::vector<node_ptr>& post_iteration_nodes,
			const std::vector<node_ptr>& statement_nodes);
		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		node_ptr get_loop_initialization_node() const;
		node_ptr get_loop_condition_node() const;
		const std::vector<node_ptr>& get_post_iteration_nodes() const;
		const std::vector<node_ptr>& get_loop_body_nodes() const;
	private:
		node_ptr m_loop_initialization_node;
		node_ptr m_loop_condition_node;
		std::vector<node_ptr> m_post_iteration_nodes;
		std::vector<node_ptr> m_loop_body_nodes;
	};
}