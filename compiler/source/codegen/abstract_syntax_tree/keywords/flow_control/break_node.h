#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class break_node : public node {
	public:
		break_node(u64 line_number);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}