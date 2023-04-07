#pragma once
#include "../integer_base_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i8 type keyword.
	 */
	class keyword_i8_node : public integer_base_node {
	public:
		keyword_i8_node(i8 value);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		bool is_signed() const override;
		i8 get_value() const;
	private:
		i8 m_value;
	};
}