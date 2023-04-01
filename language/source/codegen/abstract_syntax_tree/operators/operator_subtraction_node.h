#pragma once
#include "operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary addition operator.
	 */
	class operator_subtraction_node : public operator_binary {
	public:
		operator_subtraction_node(node* left, node* right);
		llvm::Value* accept(visitor& visitor) override;
	};
}
