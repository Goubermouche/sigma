#include "declaration_node.h"

namespace channel {
	declaration_node::declaration_node(type declaration_type, const std::string& name, node* expression)
		: m_declaration_type(declaration_type), m_name(name), m_expression(expression)
	{}

	const std::string& declaration_node::get_name() const {
		return m_name;
	}

	node* declaration_node::get_expression() const	{
		return m_expression;
	}

	type declaration_node::get_declaration_type() const {
		return m_declaration_type;
	}
}