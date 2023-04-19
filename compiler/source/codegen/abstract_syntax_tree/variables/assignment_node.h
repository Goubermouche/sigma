#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, assignment of a value.
	 */
	class assignment_node : public node {
	public:
		assignment_node(u64 line_number, const std::string& variable_identifier, node* expression_node);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		const std::string& get_variable_identifier() const;
		node* get_expression_node() const;
	private:
		std::string m_variable_identifier;
		node* m_expression_node;
	};
}