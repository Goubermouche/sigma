#include "operator_binary.h"

namespace channel {
	operator_binary::operator_binary(u64 line_number, node* left_expression_node, node* right_expression_node)
		: node(line_number), left_expression_node(left_expression_node), right_expression_node(right_expression_node) {}

	operator_binary::~operator_binary() {
		delete left_expression_node;
		delete right_expression_node;
	}
}