#pragma once
#include "../../node.h"
#include "../../../type.h"

namespace channel {
	/**
	 * \brief AST node, represents the declaration of a variable.
	 */
	class declaration_node : public node {
	public:
		declaration_node(u64 line_number, type declaration_type, const std::string& name, node* expression = nullptr);

		const std::string& get_name() const;
		node* get_expression() const;
		type get_declaration_type() const;
	private:
		type m_declaration_type;
		std::string m_name;
		node* m_expression;
	};
}