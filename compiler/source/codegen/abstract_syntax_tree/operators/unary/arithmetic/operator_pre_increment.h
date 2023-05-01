#pragma once
#include "../operator_unary.h"

namespace channel {
	/**
	 * \brief AST node, represents the pre decrement binary operator.
	 */
	class operator_pre_increment : public operator_unary {
	public:
		operator_pre_increment(
			u64 line_number, 
			const node_ptr& expression_node
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
