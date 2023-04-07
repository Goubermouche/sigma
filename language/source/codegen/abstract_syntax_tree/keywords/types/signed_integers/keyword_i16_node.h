#pragma once
#include "../integer_base_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i16 type keyword.
	 */
	class keyword_i16_node : public integer_base_node {
	public:
		keyword_i16_node(i16 value);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		bool is_signed() const override;
		i16 get_value() const;
	private:
		i16 m_value;
	};
}
