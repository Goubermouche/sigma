#pragma once
#include "operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary division operator.
	 */
	class operator_division_node : public operator_binary {
	public:
		operator_division_node(u64 line_number, node* left_expression_node, node* right_expression_node);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;
	};
}
