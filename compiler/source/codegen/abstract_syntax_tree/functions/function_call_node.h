#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a call to a given function. 
	 */
	class function_call_node : public node {
	public:
		function_call_node(
			const token_position& position,
			const std::string& function_identifier,
			const std::vector<node_ptr>& function_arguments
		);

		bool accept(visitor& visitor, value_ptr& out_value, codegen_context context) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const std::string& get_function_identifier() const;
		const std::vector<node_ptr>& get_function_arguments() const;
	private:
		std::string m_function_name;
		std::vector<node_ptr> m_function_arguments;
	};
}
