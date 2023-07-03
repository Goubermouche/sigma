#pragma once
#include "code_generator/abstract_syntax_tree/operators/unary/operator_unary_base.h"

namespace sigma {
	/**
	 * \brief AST node, represents the post decrement unary operator.
	 */
	class operator_post_decrement_node : public operator_unary_base {
	public:
		operator_post_decrement_node(
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