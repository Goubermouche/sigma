#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a call to a given function. 
	 */
	class function_call_node : public node {
	public:
		function_call_node(u64 line_number, const std::string& name, const std::vector<node*>& arguments);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		const std::string& get_name() const;
		const std::vector<node*>& get_arguments() const;
	private:
		std::string m_name;
		std::vector<node*> m_arguments;
	};
}
