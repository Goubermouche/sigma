#pragma once
#include "../node.h"
#include "../../type.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class function_node : public node {
	public:
		function_node(u64 line_number, const type& function_return_type, const std::string& function_identifier, const std::vector<node*>& function_statements, const std::vector<std::pair<std::string, type>>& function_arguments);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const type& get_function_return_type() const;
		const std::string& get_function_identifier() const;
		const std::vector<node*>& get_function_statements() const;
		const std::vector<std::pair<std::string, type>>& get_function_arguments() const;
	private:
		type m_function_return_type;
		std::string m_function_identifier;
		std::vector<node*> m_function_statements;
		std::vector<std::pair<std::string, type>> m_function_arguments;
	};
}