#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the f32 type keyword.
	 */
	class f32_node : public node {
	public:
		f32_node(u64 line_number, f32 value);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		f32 get_value() const;
	private:
		f32 m_value;
	};
}