#pragma once
#include "code_generator/abstract_syntax_tree/operators/unary/operator_unary_base.h"

namespace channel {
	/**
	 * \brief AST node, represents the post increment binary operator.
	 */
	class operator_post_increment_node : public operator_unary_base {
	public:
		operator_post_increment_node(
			const token_position& position,
			const node_ptr& expression_node
		);

		acceptation_result accept(
			code_generator& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix,
			bool is_last
		) override;
	};
}
