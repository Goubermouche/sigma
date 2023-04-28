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
			node* loop_initialization_node,
			node* loop_condition_node,
			const std::vector<node*>& post_iteration_nodes,
			const std::vector<node*>& statement_nodes);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		node* get_loop_initialization_node() const;
		node* get_loop_condition_node() const;
		const std::vector<node*>& get_post_iteration_nodes() const;
		const std::vector<node*>& get_statement_nodes() const;
	private:
		node* m_loop_initialization_node;
		node* m_loop_condition_node;
		std::vector<node*> m_post_iteration_nodes;
		std::vector<node*> m_statement_nodes;
	};
}