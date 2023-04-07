#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i64 type keyword.
	 */
	class keyword_i64_node : public node {
	public:
		keyword_i64_node(u64 value);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;
		
		u64 get_value() const;
	private:
		u64 m_value;
	};
}
