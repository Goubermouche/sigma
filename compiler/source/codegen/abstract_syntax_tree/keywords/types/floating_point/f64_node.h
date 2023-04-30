#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the f32 type keyword.
	 */
	class f64_node : public node {
	public:
		f64_node(u64 line_number, f64 value);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		f64 get_value() const;
	private:
		f64 m_value;
	};
}