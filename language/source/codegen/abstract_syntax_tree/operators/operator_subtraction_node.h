#pragma once
#include "operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary subtraction operator.
	 */
	class operator_subtraction_node : public operator_binary {
	public:
		operator_subtraction_node(node* left, node* right);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;
	};
}
