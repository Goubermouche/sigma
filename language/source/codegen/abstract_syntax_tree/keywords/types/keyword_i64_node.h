#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i64 type keyword.
	 */
	class keyword_i64_node : public node {
	public:
		llvm::Value* accept(visitor& visitor) override;

		i64 value;
	};
}
