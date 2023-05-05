#pragma once
#include "../operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the less than or equal to binary operator.
	 */
	class operator_less_than_equal_to_node : public operator_binary {
	public:
		operator_less_than_equal_to_node(
			const token_position& position,
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
