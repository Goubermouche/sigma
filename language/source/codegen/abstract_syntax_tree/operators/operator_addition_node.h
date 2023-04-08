#pragma once
#include "operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary addition operator.
	 */
	class operator_addition_node : public operator_binary {
	public:
		operator_addition_node(u64 line_index, node* left, node* right);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;
	};
}
