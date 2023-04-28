#pragma once
#include "../../node.h"

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
			node* loop_index_node,
			node* loop_condition_node,
			node* loop_index_increment_node,
			const std::vector<node*>& statement_nodes);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		node* get_loop_index_node() const;
		node* get_loop_condition_node() const;
		node* get_loop_index_increment_node() const;
		const std::vector<node*>& get_statement_nodes() const;
	private:
		node* m_loop_index_node;
		node* m_loop_condition_node;
		node* m_loop_index_increment_node;
		std::vector<node*> m_statement_nodes;
	};
}