#include "operator_unary.h"

namespace channel {
	operator_unary::operator_unary(u64 line_number, node* expression_node)
		: node(line_number), m_expression_node(expression_node) {}

	operator_unary::~operator_unary() {
		delete m_expression_node;
	}
}