#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i32 type keyword.
	 */
	class keyword_i32_node : public node {
	public:
		keyword_i32_node(u64 value);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		u64 get_value() const;
	private:
		u64 m_value;
	};
}
