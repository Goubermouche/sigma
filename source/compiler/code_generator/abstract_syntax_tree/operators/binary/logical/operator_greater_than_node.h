#pragma once
#include "code_generator/abstract_syntax_tree/operators/binary/operator_binary_base.h"

namespace sigma {
	/**
	 * \brief AST node, represents the greater than binary operator.
	 */
	class operator_greater_than_node : public operator_binary_base {
	public:
		operator_greater_than_node(
			const file_range& range,
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		outcome::result<value_ptr> accept(
			code_generator_template& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth, 
			const std::wstring& prefix,
			bool is_last
		) override;
	};
}