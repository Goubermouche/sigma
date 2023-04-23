#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, assignment of a value.
	 */
	class assignment_node : public node {
	public:
		assignment_node(u64 line_number, node* variable, node* expression_node);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		node* get_variable_node() const;
		node* get_expression_node() const;
	private:
		node* m_variable_node;
		node* m_expression_node;
	};
}