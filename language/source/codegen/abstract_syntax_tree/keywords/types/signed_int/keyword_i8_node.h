#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i8 type keyword.
	 */
	class keyword_i8_node : public node {
	public:
		keyword_i8_node(u64 line_index, i64 value);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		i64 get_value() const;
	private:
		i64 m_value;
	};
}