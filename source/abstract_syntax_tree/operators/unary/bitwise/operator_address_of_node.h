#pragma once
#include "abstract_syntax_tree/operators/unary/operator_unary_base.h"

namespace sigma {
	/**
	 * \brief AST node, represents the bitwise not unary operator.
	 */
	class operator_address_of_node : public operator_unary_base {
	public:
		operator_address_of_node(
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