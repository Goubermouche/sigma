#pragma once
#include <llvm/IR/Value.h>
#include "../../utility/macros.h"

namespace channel {
	class visitor;

	/**
	 * \brief base AST node.
	 */
	class node {
	public:
		virtual ~node() = default;

		/**
		 * \brief Accepts the given \a visitor. 
		 * \param visitor Visitor to accept
		 */
		virtual llvm::Value* accept(visitor& visitor) = 0;
	};
}

#include "../visitor.h"