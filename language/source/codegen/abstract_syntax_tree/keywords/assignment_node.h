#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, assignment of a value.
	 */
	class assignment_node : public node {
	public:
		assignment_node(const std::string& name, node* expression);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		const std::string& get_name() const;
		node* get_expression() const;
	private:
		std::string m_name;
		node* m_expression;
	};
}