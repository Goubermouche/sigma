#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the u32 type keyword.
	 */
	class keyword_u32_node : public node {
	public:
		keyword_u32_node(u32 value);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		u32 get_value() const;
	private:
		u32 m_value;
	};
}
