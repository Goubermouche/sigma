#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i16 type keyword.
	 */
	class keyword_i16_node : public node {
	public:
		llvm::Value* accept(visitor& visitor) override;

		i16 value;
	};
}
