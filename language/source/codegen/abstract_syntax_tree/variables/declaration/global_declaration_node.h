#pragma once
#include "../../node.h"
#include "declaration_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the global declaration of a variable.
	 */
	class global_declaration_node : public declaration_node {
	public:
		global_declaration_node(u64 line_number, type declaration_type, const std::string& name, node* expression = nullptr);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;
	};
}