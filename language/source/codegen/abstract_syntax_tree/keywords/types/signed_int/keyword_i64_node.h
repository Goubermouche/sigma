#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i64 type keyword.
	 */
	class keyword_i64_node : public node {
	public:
		keyword_i64_node(u64 line_number, i64 value);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;
		
		i64 get_value() const;
	private:
		i64 m_value;
	};
}