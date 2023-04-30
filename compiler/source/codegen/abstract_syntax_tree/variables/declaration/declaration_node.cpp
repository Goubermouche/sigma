#include "declaration_node.h"

namespace channel {
	declaration_node::declaration_node(u64 line_number, const type& declaration_type, const std::string& declaration_identifier, node_ptr expression_node)
		: node(line_number), m_declaration_type(declaration_type), m_declaration_identifier(declaration_identifier), m_expression_node(expression_node)
	{}

	const std::string& declaration_node::get_declaration_identifier() const {
		return m_declaration_identifier;
	}

	node_ptr declaration_node::get_expression_node() const	{
		return m_expression_node;
	}

	const type& declaration_node::get_declaration_type() const {
		return m_declaration_type;
	}
}