#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, assignment of a value.
	 */
	class assignment_node : public node {
	public:
		assignment_node(u64 line_number, const std::string& name, node* expression);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		const std::string& get_name() const;
		node* get_expression() const;
	private:
		std::string m_name;
		node* m_expression;
	};
}