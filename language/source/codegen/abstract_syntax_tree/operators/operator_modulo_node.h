#pragma once
#include "operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary division operator.
	 */
	class operator_modulo_node : public operator_binary {
	public:
		operator_modulo_node(node* left, node* right);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;
	};
}
