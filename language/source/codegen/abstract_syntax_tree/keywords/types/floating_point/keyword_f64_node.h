#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the f32 type keyword.
	 */
	class keyword_f64_node : public node {
	public:
		keyword_f64_node(f64 value);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		f64 get_value() const;
	private:
		f64 m_value;
	};
}