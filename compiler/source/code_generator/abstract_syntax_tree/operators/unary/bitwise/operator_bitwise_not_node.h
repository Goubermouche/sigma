#pragma once
#include "code_generator/abstract_syntax_tree/operators/unary/operator_unary_base.h"

namespace channel {
	/**
	 * \brief AST node, represents the bitwise not unary operator.
	 */
	class operator_bitwise_not_node : public operator_unary_base {
	public:
		operator_bitwise_not_node(
			const token_location& location,
			const node_ptr& expression_node
		);

		expected_value accept(
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
