#pragma once
#include "../operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the greater than binary operator.
	 */
	class operator_greater_than_node : public operator_binary {
	public:
		operator_greater_than_node(
			u64 line_number, 
			const node_ptr& left_expression_node,
			const node_ptr& right_expression_node
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
