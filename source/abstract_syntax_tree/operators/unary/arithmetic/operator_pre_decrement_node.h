#pragma once
#include "abstract_syntax_tree/operators/unary/operator_unary_base.h"

namespace sigma {
	/**
	 * \brief AST node, represents the pre decrement unary operator.
	 */
	class operator_pre_decrement_node : public operator_unary_base {
	public:
		operator_pre_decrement_node(
			const utility::file_range& range,
			const node_ptr& expression_node
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