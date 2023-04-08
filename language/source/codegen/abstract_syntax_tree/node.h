#pragma once
#include "value.h"
#include <llvm/IR/Value.h>
#include "../../utility/macros.h"

namespace channel {
	class visitor;

	/**
	 * \brief base AST node.
	 */
	class node {
	public:
		node(u64 line_index);
		virtual ~node() = default;

		/**
		 * \brief Accepts the given \a visitor. 
		 * \param visitor Visitor to accept
		 */
		virtual value* accept(visitor& visitor) = 0;
		virtual std::string get_node_name() const = 0;
		u64 get_declaration_line_index() const;
		// void set_declaration_line_index(u64 line_index);
	private:
		u64 m_line_index = 0;
	};
}

#include "../visitor.h"