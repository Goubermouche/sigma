#pragma once
#include "code_generator/abstract_syntax_tree/operators/unary/operator_unary.h"

namespace channel {
	/**
	 * \brief AST node, represents the pre decrement binary operator.
	 */
	class operator_pre_decrement : public operator_unary {
	public:
		operator_pre_decrement(
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