#pragma once
#include "../../node.h"
#include "../../../type.h"

namespace channel {
	/**
	 * \brief AST node, represents the declaration of a variable.
	 */
	class declaration_node : public node {
	public:
		declaration_node(u64 line_number, const type& declaration_type, const std::string& declaration_identifier, node_ptr expression_node = nullptr);

		const std::string& get_declaration_identifier() const;
		node_ptr get_expression_node() const;
		const type& get_declaration_type() const;
	private:
		type m_declaration_type;
		std::string m_declaration_identifier;
		node_ptr m_expression_node;
	};
}