#include "declaration_node.h"

namespace sigma {
	declaration_node::declaration_node(
		const file_position& location,
		const type& declaration_type, 
		const std::string& declaration_identifier,
		const node_ptr& expression_node
	) : node(location),
	m_declaration_type(declaration_type),
	m_declaration_identifier(declaration_identifier),
	m_expression_node(expression_node) {}

	const std::string& declaration_node::get_declaration_identifier() const {
		return m_declaration_identifier;
	}

	const node_ptr& declaration_node::get_expression_node() const	{
		return m_expression_node;
	}

	const type& declaration_node::get_declaration_type() const {
		return m_declaration_type;
	}
}