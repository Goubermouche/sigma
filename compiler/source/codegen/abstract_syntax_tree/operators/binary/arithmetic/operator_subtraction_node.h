#pragma once
#include "../operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary subtraction operator.
	 */
	class operator_subtraction_node : public operator_binary {
	public:
		operator_subtraction_node(u64 line_number, node_ptr left, node_ptr right);
		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
