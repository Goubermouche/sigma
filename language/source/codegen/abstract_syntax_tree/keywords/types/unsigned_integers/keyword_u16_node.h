#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the u16 type keyword.
	 */
	class keyword_u16_node : public node {
	public:
		keyword_u16_node(u16 value);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		u16 get_value() const;
	private:
		u16 m_value;
	};
}
