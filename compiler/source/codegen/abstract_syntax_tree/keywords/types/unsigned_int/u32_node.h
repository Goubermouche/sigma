#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a u32 literal.
	 */
	class u32_node : public node {
	public:
		u32_node(
			const token_position& position,
			u64 value
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		u64 get_value() const;
	private:
		u64 m_value;
	};
}
