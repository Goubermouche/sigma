#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the u8 type keyword.
	 */
	class keyword_u8_node : public node {
	public:
		keyword_u8_node(u8 value);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		u8 get_value() const;
	private:
		u8 m_value;
	};
}