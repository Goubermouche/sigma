#pragma once
#include "codegen/abstract_syntax_tree/operators/unary/operator_unary.h"

namespace channel {
	/**
	 * \brief AST node, represents the post decrement binary operator.
	 */
	class operator_post_decrement : public operator_unary {
	public:
		operator_post_decrement(
			const token_position& position,
			const node_ptr& expression_node
		);

		bool accept(visitor& visitor, value_ptr& out_value, codegen_context context) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}