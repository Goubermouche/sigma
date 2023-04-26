#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class if_else_node : public node {
	public:
		if_else_node(u64 line_number, const std::vector<node*>& condition_nodes, const std::vector<std::vector<node*>>& branch_nodes);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		const std::vector<node*>& get_condition_nodes();
		const std::vector<std::vector<node*>>& get_branch_nodes();
	private:
		std::vector<node*> m_condition_nodes;
		std::vector<std::vector<node*>> m_branch_nodes;
	};
}