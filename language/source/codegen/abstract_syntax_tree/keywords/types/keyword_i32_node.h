#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i32 type keyword.
	 */
	class keyword_i32_node : public node {
		void accept(visitor& visitor) override;
		i32 value;
	};
}
