#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class return_node : public node {
	public:
		return_node(node* expression);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		node* get_expression() const;
	private:
		node* m_expression;
	};
}


