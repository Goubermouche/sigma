#include "operator_binary.h"

namespace channel {
	operator_binary::operator_binary(u64 line_number, node* left_expression_node, node* right_expression_node)
		: node(line_number), m_left_expression_node(left_expression_node), m_right_expression_node(right_expression_node) {}

	operator_binary::~operator_binary() {
		delete m_left_expression_node;
		delete m_right_expression_node;
	}

	node* operator_binary::get_left_expression_node() const {
		return m_left_expression_node;
	}
	node* operator_binary::get_right_expression_node() const {
		return m_right_expression_node;
	}
}