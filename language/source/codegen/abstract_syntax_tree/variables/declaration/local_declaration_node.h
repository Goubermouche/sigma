#pragma once
#include "../../node.h"
#include "declaration_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the local declaration of a variable.
	 */
	class local_declaration_node : public declaration_node {
	public:
		local_declaration_node(u64 line_index, type declaration_type, const std::string& name, node* expression = nullptr);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;
	};
}