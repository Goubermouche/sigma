#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class function_node : public node {
	public:
		function_node(const std::string& return_type, const std::string& name, std::vector<node*> statements);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		const std::string& get_return_type() const;
		const std::string& get_name() const;
		const std::vector<node*>& get_statements() const;
	private:
		std::string m_return_type;
		std::string m_name;
		std::vector<node*> m_statements;
	};
}


