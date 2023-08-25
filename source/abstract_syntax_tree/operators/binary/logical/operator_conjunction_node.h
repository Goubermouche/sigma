#pragma once
#include "abstract_syntax_tree/operators/binary/operator_binary_base.h"

namespace sigma {
	/**
	 * \brief AST node, represents the conjunction binary operator.
	 */
	class operator_conjunction_node : public operator_binary_base {
	public:
		operator_conjunction_node(
			const utility::file_range& range,
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		utility::outcome::result<value_ptr> accept(
			abstract_syntax_tree_visitor_template& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix, 
			bool is_last
		) override;
	};
}
