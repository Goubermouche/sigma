#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i8 type keyword.
	 */
	class variable_node : public node {
	public:
		variable_node(const std::string& name);
		llvm::Value* accept(visitor& visitor) override;

		const std::string& get_name() const;
	private:
		std::string m_name;
	};
}
