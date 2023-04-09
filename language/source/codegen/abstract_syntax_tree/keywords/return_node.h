#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class return_node : public node {
	public:
		return_node(u64 line_number, node* expression);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		node* get_expression() const;
	private:
		node* m_expression;
	};
}


