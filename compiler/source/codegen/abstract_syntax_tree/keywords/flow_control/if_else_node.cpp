#include "if_else_node.h"

namespace channel {
	if_else_node::if_else_node(u64 line_number, const std::vector<node*>& condition_nodes, const std::vector<std::vector<node*>>& branch_nodes)
		: node(line_number), m_condition_nodes(condition_nodes), m_branch_nodes(branch_nodes) {}

	bool if_else_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(if_else_node);
		return visitor.visit_if_else_node(*this, out_value);
	}

	std::string if_else_node::get_node_name() const {
		return "if_else_node";
	}

	const std::vector<node*>& if_else_node::get_condition_nodes() {
		return m_condition_nodes;
	}

	const std::vector<std::vector<node*>>& if_else_node::get_branch_nodes()	{
		return m_branch_nodes;
	}
}