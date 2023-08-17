#pragma once
#include "code_generator/abstract_syntax_tree/node.h"
#include "llvm_wrappers/type.h"

namespace sigma {
	/**
	 * \brief AST node, represents a function declaration.
	 */
	class function_node : public node {
	public:
		function_node(
			const file_range& range,
			const type& function_return_type,
			bool is_var_arg,
			const std::string& function_identifier,
			const std::vector<std::pair<std::string, type>>& function_arguments,
			const std::vector<node_ptr>& function_statements
		);

		outcome::result<value_ptr> accept(
			code_generator_template& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth, 
			const std::wstring& prefix,
			bool is_last
		) override;

		const type& get_function_return_type() const;
		const std::string& get_function_identifier() const;
		bool is_var_arg() const;
		const std::vector<node_ptr>& get_function_statements() const;
		const std::vector<std::pair<std::string, type>>& get_function_arguments() const;
	private:
		type m_function_return_type;
		std::string m_function_identifier;
		bool m_is_var_arg;
		std::vector<std::pair<std::string, type>> m_function_arguments;
		std::vector<node_ptr> m_function_statements;
	};
}