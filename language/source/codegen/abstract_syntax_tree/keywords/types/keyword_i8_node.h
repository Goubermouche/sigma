#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i8 type keyword.
	 */
	class keyword_i8_node : public node {
	public:
		llvm::Value* accept(visitor& visitor) override;

		i8 value;
	};
}
