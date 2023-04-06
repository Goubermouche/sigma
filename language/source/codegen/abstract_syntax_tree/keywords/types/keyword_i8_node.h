#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i8 type keyword.
	 */
	class keyword_i8_node : public node {
	public:
		keyword_i8_node(i8 value);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		i8 get_value() const;
	private:
		i8 m_value;
	};
}
