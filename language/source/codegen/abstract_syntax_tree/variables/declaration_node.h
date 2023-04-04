#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the declaration of a variable. 
	 */
	class declaration_node : public node {
	public:
		declaration_node(const std::string& name, bool is_global, node* expression = nullptr);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		const std::string& get_name() const;
		bool is_global() const;
		node* get_expression() const;
	private:
		std::string m_name;
		bool m_is_global;
		node* m_expression;
	};
}