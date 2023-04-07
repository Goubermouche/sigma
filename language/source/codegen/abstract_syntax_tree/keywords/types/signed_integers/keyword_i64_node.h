#pragma once
#include "../integer_base_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i64 type keyword.
	 */
	class keyword_i64_node : public integer_base_node {
	public:
		keyword_i64_node(i64 value);
		llvm::Value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		bool is_signed() const override;
		i64 get_value() const;
	private:
		i64 m_value;
	};
}
