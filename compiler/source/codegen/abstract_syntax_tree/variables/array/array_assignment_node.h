#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, assignment of a value.
	 */
	class array_assignment_node : public node {
	public:
		array_assignment_node(u64 line_number, const std::string& name, node* array_index_node,  node* expression);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		const std::string& get_name() const;
		node* get_array_index_node() const;
		node* get_expression_node() const;
	private:
		std::string m_name;
		node* m_array_index_node;
		node* m_expression;
	};
}