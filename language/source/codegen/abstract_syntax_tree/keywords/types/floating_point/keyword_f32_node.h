#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the f32 type keyword.
	 */
	class keyword_f32_node : public node {
	public:
		keyword_f32_node(u64 line_index, f32 value);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		f32 get_value() const;
	private:
		f32 m_value;
	};
}