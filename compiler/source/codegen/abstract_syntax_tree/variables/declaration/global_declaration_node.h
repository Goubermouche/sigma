#pragma once
#include "../../node.h"
#include "declaration_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the global declaration of a variable.
	 */
	class global_declaration_node : public declaration_node {
	public:
		global_declaration_node(u64 line_number, const type& declaration_type, const std::string& declaration_identifier, node* expression_node = nullptr);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}