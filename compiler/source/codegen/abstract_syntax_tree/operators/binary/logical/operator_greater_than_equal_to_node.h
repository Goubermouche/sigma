#pragma once
#include "codegen/abstract_syntax_tree/operators/binary/operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the greater than or equal binary operator.
	 */
	class operator_greater_than_equal_to_node : public operator_binary {
	public:
		operator_greater_than_equal_to_node(
			const token_position& position,
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		bool accept(visitor& visitor, value_ptr& out_value, codegen_context context) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
