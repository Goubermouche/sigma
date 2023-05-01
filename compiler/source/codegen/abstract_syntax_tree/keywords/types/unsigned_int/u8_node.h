#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a u8 literal.
	 */
	class u8_node : public node {
	public:
		u8_node(u64 line_number, u64 value);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		u64 get_value() const;
	private:
		u64 m_value;
	};
}