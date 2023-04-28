#include "for_node.h"

namespace channel {
	for_node::for_node(u64 line_number, node* loop_index_node, node* loop_condition_node, node* loop_index_increment_node, const std::vector<node*>& statement_nodes)
		: node(line_number), m_loop_index_node(loop_index_node), m_loop_condition_node(loop_condition_node), m_loop_index_increment_node(loop_index_increment_node), m_statement_nodes(statement_nodes) {}

	bool for_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(for_node);
		return visitor.visit_for_node(*this, out_value);
	}

	std::string for_node::get_node_name() const {
		return "for_node";
	}

	node* for_node::get_loop_index_node() const {
		return m_loop_index_node;
	}

	node* for_node::get_loop_condition_node() const {
		return m_loop_condition_node;
	}

	node* for_node::get_loop_index_increment_node() const {
		return m_loop_index_increment_node;
	}

	const std::vector<node*>& for_node::get_statement_nodes() const {
		return m_statement_nodes;
	}
}