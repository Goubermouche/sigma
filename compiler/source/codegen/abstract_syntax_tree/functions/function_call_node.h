#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a call to a given function. 
	 */
	class function_call_node : public node {
	public:
		function_call_node(u64 line_number, const std::string& function_identifier, const std::vector<node*>& function_arguments);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const std::string& get_function_identifier() const;
		const std::vector<node*>& get_function_arguments() const;
	private:
		std::string m_function_name;
		std::vector<node*> m_function_arguments;
	};
}
