#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i8 type keyword.
	 */
	class function_call_node : public node {
	public:
		function_call_node(const std::string& name, const std::vector<node*>& arguments);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		const std::string& get_name() const;
		const std::vector<node*>& get_arguments() const;
	private:
		std::string m_name;
		std::vector<node*> m_arguments;
	};
}
