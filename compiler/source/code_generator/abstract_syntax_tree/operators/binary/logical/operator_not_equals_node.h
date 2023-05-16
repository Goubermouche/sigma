#pragma once
#include "code_generator/abstract_syntax_tree/operators/binary/operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the negated equality binary operator.
	 */
	class operator_not_equals_node : public operator_binary {
	public:
		operator_not_equals_node(
			const token_position& position,
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		acceptation_result accept(
			code_generator& visitor,
			const codegen_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix, 
			bool is_last
		) override;
	};
}
