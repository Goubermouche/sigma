#pragma once
#include "../node.h"
#include "../../type.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class function_node : public node {
	public:
		function_node(u64 line_number, type type, const std::string& name, std::vector<node*> statements);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		type get_return_type() const;
		const std::string& get_name() const;
		const std::vector<node*>& get_statements() const;
	private:
		type m_return_type;
		std::string m_name;
		std::vector<node*> m_statements;
	};
}