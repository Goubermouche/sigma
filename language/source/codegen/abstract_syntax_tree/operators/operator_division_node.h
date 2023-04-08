#pragma once
#include "operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary division operator.
	 */
	class operator_division_node : public operator_binary {
	public:
		operator_division_node(u64 line_index, node* left, node* right);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;
	};
}
