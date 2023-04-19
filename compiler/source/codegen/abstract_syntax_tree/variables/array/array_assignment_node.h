#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, assignment of a value.
	 */
	class array_assignment_node : public node {
	public:
		array_assignment_node(u64 line_number, const std::string& array_identifier, node* array_element_index_node, node* expression_node);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		const std::string& get_array_identifier() const;
		node* get_array_element_index_node() const;
		node* get_expression_node() const;
	private:
		std::string m_array_identifier;
		node* m_array_element_index_node;
		node* m_expression_node;
	};
}