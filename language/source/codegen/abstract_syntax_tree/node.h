#pragma once
#include "../../utility/macros.h"
#include "../visitor.h"

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
		virtual void accept(visitor& visitor) = 0;
	};
}