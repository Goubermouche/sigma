#include "while_node.h"

namespace channel {
	while_node::while_node(u64 line_number, node* loop_condition_node, const std::vector<node*>& statement_nodes)
		: node(line_number), m_loop_condition_node(loop_condition_node), m_statement_nodes(statement_nodes) {}

	bool while_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(while_node);
		return visitor.visit_while_node(*this, out_value);
	}

	std::string while_node::get_node_name() const {
		return "while_node";
	}

	node* while_node::get_loop_condition_node() const {
		return m_loop_condition_node;
	}

	const std::vector<node*>& while_node::get_statement_nodes() const {
		return m_statement_nodes;
	}
}