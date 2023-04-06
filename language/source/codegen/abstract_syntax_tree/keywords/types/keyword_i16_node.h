#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i16 type keyword.
	 */
	class keyword_i16_node : public node {
	public:
		keyword_i16_node(i16 value);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		i16 get_value() const;
	private:
		i16 m_value;
	};
}
