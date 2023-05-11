#pragma once
#include "codegen/abstract_syntax_tree/operators/binary/operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the conjunction binary operator.
	 */
	class operator_conjunction_node : public operator_binary {
	public:
		operator_conjunction_node(
			const token_position& position,
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
