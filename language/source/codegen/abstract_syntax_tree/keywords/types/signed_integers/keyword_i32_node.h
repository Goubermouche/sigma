#pragma once
#include "../integer_base_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i32 type keyword.
	 */
	class keyword_i32_node : public integer_base_node {
	public:
		keyword_i32_node(i32 value);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		bool is_signed() const override;
		i32 get_value() const;
	private:
		i32 m_value;
	};
}
