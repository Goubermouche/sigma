#pragma once
#include "../operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the disjunction binary operator.
	 */
	class operator_disjunction_node : public operator_binary {
	public:
		operator_disjunction_node(
			const token_position& position,
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
