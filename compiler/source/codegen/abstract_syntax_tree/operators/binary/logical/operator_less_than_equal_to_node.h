#pragma once
#include "../operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary division operator.
	 */
	class operator_less_than_equal_to_node : public operator_binary {
	public:
		operator_less_than_equal_to_node(u64 line_number, node* left_expression_node, node* right_expression_node);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
