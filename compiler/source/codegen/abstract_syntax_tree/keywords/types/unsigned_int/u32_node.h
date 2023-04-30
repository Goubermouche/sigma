#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the u32 type keyword.
	 */
	class u32_node : public node {
	public:
		u32_node(u64 line_number, u64 value);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		u64 get_value() const;
	private:
		u64 m_value;
	};
}
