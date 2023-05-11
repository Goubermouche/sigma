#pragma once
#include "codegen/abstract_syntax_tree/operators/unary/operator_unary.h"

namespace channel {
	/**
	 * \brief AST node, represents the post increment binary operator.
	 */
	class operator_post_increment : public operator_unary {
	public:
		operator_post_increment(
			const token_position& position,
			const node_ptr& expression_node
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix,
			bool is_last
		) override;
	};
}
